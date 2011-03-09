/*
 ============================================================================
 Name		: EventTimer.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventTimer implementation
 ============================================================================
 */

#include "EventTimer.h"
#include <HT\TimeUtils.h>

typedef struct TTimerStruct
	{
	TUint32 iType;
	TUint32	iLoDelay;
	TUint32	iHiDelay;
	} TTimerStruct;

	
CEventTimer::CEventTimer(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Timer, aTriggerId)
	{
	// No implementation required
	}

CEventTimer::~CEventTimer()
	{
	__FLOG(_L("Destructor"));
	delete iTimer;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CEventTimer* CEventTimer::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventTimer* self = new (ELeave) CEventTimer(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventTimer* CEventTimer::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventTimer* self = CEventTimer::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}
 
void CEventTimer::ConstructL(const TDesC8& params)
	{
	//TUint32 addr = (TUint32) (this);
	__FLOG_OPEN_ID("HT", "EventTimer.txt");
	__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	iTimer = CTimeOutTimer::NewL(*this);
	//TTimerStruct* timerParams = (TTimerStruct*)iParams.Ptr();	// crash on N96
	TTimerStruct timerParams;
	Mem::Copy( &timerParams, iParams.Ptr(), sizeof(TTimerStruct));
	
	TUint64 timeMillis = timerParams.iHiDelay;
	timeMillis = timerParams.iHiDelay;
	timeMillis <<= 32;
	timeMillis += timerParams.iLoDelay;
	iSecondsInterv = (timeMillis / 1000);
	
	iTimerType = (TTimerType) timerParams.iType;

	if(iTimerType == Type_Date){
		//iTimeAt = SetSymbianTime(timeMillis);
		iTimeAt = TimeUtils::GetSymbianTime(timeMillis);
	} else {
		iTimeAt = (timeMillis*1000);
	}
	}

void CEventTimer::StartEventL()
	{
	__FLOG(_L("StartEventL"));
	__FLOG_2(_L("%d %d"), iTimerType, iSecondsInterv.Int());
	if (iTimerType != Type_Date)
		{
		iTimeAt.HomeTime();
		iTimeAt += iSecondsInterv;
		iTimer->At(iTimeAt);
		}
	else {
		// First we have to check if the date is expired
		TTime now;
		now.UniversalTime();
		if (iTimeAt <= now){
			TTimeIntervalSeconds secondsInterv = 3;
			iTimeAt.HomeTime();
			iTimeAt += secondsInterv;
			iTimer->At(iTimeAt);
		} else {
			iTimer->AtUTC( iTimeAt );
		}
	}
	// Code below is useful for debugging purposes... 
	// Uncomment to trigger the event after 1 second.
/*
#ifdef _DEBUG
	iTimerType = Type_Single;
	iTimeAt.HomeTime();
	iTimeAt += TTimeIntervalSeconds(1);
	iTimer->Cancel();
	iTimer->At( iTimeAt ); 
#endif
*/     
	}


void CEventTimer::TimerExpiredL(TAny* /*src*/)
	{
	__FLOG(_L("TimerExpiredL"));
	SendActionTriggerToCoreL();
	__FLOG(_L("EventSentL"));
	if (iTimerType == Type_Repeat)
		{
		__FLOG(_L("After"));
		iTimeAt.HomeTime();
		iTimeAt += iSecondsInterv;
		iTimer->At( iTimeAt );
		}
	}

/*
 * A filetime is a 64-bit value that represents the number of 100-nanosecond intervals 
 * that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
 * Please also note that in defining KInitialTime the month and day values are offset from zero.
 * 
 */
/*
TInt64 CEventTimer::SetSymbianTime(TUint64 aFiletime)
{

	_LIT(KFiletimeInitialTime,"16010000:000000");

	TTime initialFiletime;
	initialFiletime.Set(KFiletimeInitialTime);

	TInt64 interval;
	interval = initialFiletime.Int64();

	TInt64 date = aFiletime/10;

	return (interval + date);
}
*/
/*
 L'EventTimer, definito nel file di configurazione dal relativo EventId, triggera l'azione ad esso associata ad intervalli di tempo prestabiliti.

 Sono disponibili tre tipi di timer:

 1. Single: il timer triggera la relativa azione una volta, dopo l'intervallo di tempo stabilito.
 2. Loop: il timer triggera la relativa azione ogni volta che si raggiunge l'intervallo di tempo stabilito.
 3. Date: il timer triggera la relativa azione alla data stabilita. 

 Parametri

 L'evento riceve, tramite la propria EventStruct, una seconda struttura cosi' definita:

 typedef struct _TimerStruct {
 UINT uType;     // Tipo di timer
 UINT Lo_Delay;  // Low Delay Part in ms
 UINT Hi_Delay;  // High Delay Part in ms
 } TimerStruct, *pTimerStruct;

 Timer Type

 I tipi di timer sono cosi' definiti:

 * CONF_TIMER_SINGLE = 0: triggera dopo n millisecondi.
 * CONF_TIMER_REPEAT = 1: triggera ogni n millisecondi.
 * CONF_TIMER_DATE = 2: triggera una volta alla data x identificata come timestamp (ovvero: Lo_Delay e Hi_Delay della TimerStruct). 
 */

