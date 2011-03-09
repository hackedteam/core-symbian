/*
 * RESTUtils.cpp
 *
 *  Created on: 13/feb/2011
 *      Author: Giovanna
 */

#include "RESTUtils.h"


HBufC8* CRestUtils::GetRestHeaderL(const TDesC8& aHost, const TDesC8& aCookie)
	{
	CBufBase* buffer = CBufFlat::NewL(100);
	CleanupStack::PushL(buffer);
	
	//insert method
	buffer->InsertL(buffer->Size(),KPost().Ptr(),KPost().Size());
	//insert Host
	buffer->InsertL(buffer->Size(),KHost().Ptr(),KHost().Size());
	buffer->InsertL(buffer->Size(),aHost.Ptr(),aHost.Size());
	buffer->InsertL(buffer->Size(),KNewLine().Ptr(),KNewLine().Size());
	//insert Cookie
	if(aCookie.Size()!=0)
		{
		buffer->InsertL(buffer->Size(),KCookie().Ptr(),KCookie().Size());
		buffer->InsertL(buffer->Size(),aCookie.Ptr(),aCookie.Size());
		buffer->InsertL(buffer->Size(),KNewLine().Ptr(),KNewLine().Size());
		}
	//insert ContentType
	buffer->InsertL(buffer->Size(),KContentType().Ptr(),KContentType().Size());
			
	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	return result;
	}

TInt CRestUtils::GetContentLength(const TDesC8& aRestHeader)
	{
	RBuf8 header;
	header.CreateL(aRestHeader);
	header.CleanupClosePushL();
		
	TInt pos;
	TPtr8 headerPtr(0,0);
	headerPtr.Set((TUint8 *)header.Ptr(),header.Size(),header.Size());
		
	while((pos=headerPtr.Find(KNewLine)) != KErrNotFound)
		{
		TPtrC8 line(headerPtr.Mid(0,pos));
		if(line.Find(KContentLength) != KErrNotFound)
			{
			//this is the Content-Length line
			TInt size=line.Size();
			TBuf8<16> lengthBuf(line.Right(size-KContentLength().Size()));
			lengthBuf.TrimAll();
			TLex8 lex(lengthBuf);
			TInt value=0;
			lex.Val(value);
			CleanupStack::PopAndDestroy(&header);
			return value;
			}
		headerPtr = headerPtr.Right( headerPtr.Size() - (pos+KNewLine().Size()));
		}
	CleanupStack::PopAndDestroy(&header);
	return 0;
	}


HBufC8* CRestUtils::GetCookieL(const TDesC8& aRestHeader)
	{
	RBuf8 header;
	header.CreateL(aRestHeader);
	header.CleanupClosePushL();
	
	TInt pos;
	
	TPtr8 headerPtr(0,0);
	headerPtr.Set((TUint8 *)header.Ptr(),header.Size(),header.Size());
	
	while((pos=headerPtr.Find(KNewLine)) != KErrNotFound)
		{
		TPtrC8 line(headerPtr.Mid(0,pos));
		if(line.Find(KSetCookie) != KErrNotFound)
			{
			//this is the Set-Cookie line
			TInt size=line.Size();
			TBuf8<32> cookieBuf(line.Right(size-KSetCookie().Size()));
			HBufC8* cookie = HBufC8::NewL(cookieBuf.Size());
			cookie->Des().Copy(cookieBuf);
			CleanupStack::PopAndDestroy(&header);
			return cookie;
			}
		headerPtr = headerPtr.Right( headerPtr.Size() - (pos + KNewLine().Size()));
		}
	
	CleanupStack::PopAndDestroy(&header);
	HBufC8* cookie = HBufC8::NewL(0);
	return cookie;
	}

HBufC8* CRestUtils::GetBodyL(const TDesC8& aResponse)
	{
	TInt pos;
	if((pos=aResponse.Find(KDoubleNewLine))==KErrNotFound)
		{
		HBufC8* body = HBufC8::NewL(0);
		return body;
		}
	else
		{
		HBufC8* body = HBufC8::NewL(aResponse.Size()-(pos+KDoubleNewLine().Size()));
		body->Des().Copy(aResponse.Right(aResponse.Size()-(pos+KDoubleNewLine().Size())));
		return body;
		}
	}

TInt CRestUtils::GetHeaderLength(const TDesC8& aResponse)
	{
	TInt pos;
	if((pos=aResponse.Find(KDoubleNewLine)) == KErrNotFound)
		{
		return 0;
		}
	else
		{
		return (pos+KDoubleNewLine().Size());
		}
	}
