/*
 ============================================================================
 Name		: EventTimer.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventTimer declaration
 ============================================================================
 */

#ifndef EVENTTIMER_H
#define EVENTTIMER_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractEvent.h"
#include <HT\TimeOutTimer.h>
#include <HT\Logging.h>

// CLASS DECLARATION


enum TTimerType
	{
	Type_Single = 0,
	Type_Repeat,
	Type_Date
	};

/**
 *  CEventTimer
 * 
 */
class CEventTimer : public CAbstractEvent, public MTimeOutNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventTimer();

	/**
	 * Two-phased constructor.
	 */
	static CEventTimer* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventTimer* NewLC(const TDesC8& params, TUint32 aTriggerId);

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
	CEventTimer(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);
    
    /**
     * Translate Windows Filetime into Symbian time.
     * @return A TInt64 microseconds value, suitable for Symbian TTime.
     */
    //TInt64 SetSymbianTime(TUint64 aFiletime);
    
private:
	CTimeOutTimer* iTimer;
	TTimerType iTimerType;
	TTime iTimeAt;
	TTimeIntervalSeconds iSecondsInterv;
	__FLOG_DECLARATION_MEMBER
	};

#endif // EVENTTIMER_H
