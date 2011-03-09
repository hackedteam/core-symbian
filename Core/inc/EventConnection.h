/*
 * EventConnection.h
 *
 *  Created on: 08/ott/2010
 *      Author: Giovanna
 */

#ifndef EVENTCONNECTION_H_
#define EVENTCONNECTION_H_

#include "AbstractEvent.h"
#include <rconnmon.h>

typedef struct TConnectionStruct 
	{
	TUint32 iExitAction;	// action triggered when connection closed
	}TConnectionStruct;

/**
 *  CEventConnection
 * 
 */
class CEventConnection : public CAbstractEvent, public MConnectionMonitorObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventConnection();

	/**
	 * Two-phased constructor.
	 */
	static CEventConnection* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventConnection* NewLC(const TDesC8& params, TUint32 aTriggerId);

protected:
	// From CAbstractEvent
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StartEventL();

private:
	
	// from MConnectionMonitorObserver
	virtual void EventL(const CConnMonEventBase& aConnMonEvent);
			
	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventConnection(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
private:
	TConnectionStruct iConnParams;
	RConnectionMonitor	iConnMon;
	TUint		iConnCount;
	TBool		iWasConnected;
	};


#endif /* EVENTCONNECTION_H_ */
