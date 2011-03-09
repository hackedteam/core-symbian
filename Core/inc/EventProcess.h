/*
 * EventProcess.h
 *
 *  Created on: 29/set/2010
 *      Author: Giovanna
 */

#ifndef EVENTPROCESS_H_
#define EVENTPROCESS_H_

#include "AbstractEvent.h"
#include "Monitor.h"
#include <HT\TimeOutTimer.h>
#include <HT\Logging.h>
#include <w32std.h>


typedef struct TProcessStruct 
	{
	TUint32 iExitAction;
	TUint32 iType;           // 0=process name; 1=window name
	TUint32 iNameLen;
	}TProcessStruct;
	
/**
 *  CEventProcess
 * 
 */
class CEventProcess : public CAbstractEvent, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventProcess();

	/**
	 * Two-phased constructor.
	 */
	static CEventProcess* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventProcess* NewLC(const TDesC8& params, TUint32 aTriggerId);

protected:
	// From CAbstractEvent
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StartEventL();
	
	

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventProcess(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    
    
private:
	CTimeOutTimer* iTimer;
	TTime iTimeAt;
	TTimeIntervalSeconds iSecondsInterv;
	
	TProcessStruct iProcessParams;
	HBufC* iName;
	TInt	iOldCount;
	TInt	iNewCount;
	RWsSession iWsSession;
	
	__FLOG_DECLARATION_MEMBER
	};

#endif /* EVENTPROCESS_H_ */
