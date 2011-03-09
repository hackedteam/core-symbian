#include "PubSubObserver.h"

CPubSubObserver::CPubSubObserver(MPubSubObserver& obs, TUid category, TUint key) :
	CActive(EPriorityStandard), iObserver(obs), iCategory(category), iKey(key)
	{
	// adds CPubSubObserver to the active scheduler
	CActiveScheduler::Add(this);
	}

CPubSubObserver::~CPubSubObserver()
	{
	// cancels any request still pending
	Cancel();

	// closing the handle to the property
	iProperty.Close();
	}

CPubSubObserver* CPubSubObserver::NewL(MPubSubObserver& obs, TUid category, TUint key)
	{
	// two phase construction
	CPubSubObserver* self = new (ELeave) CPubSubObserver(obs, category, key);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void CPubSubObserver::RunL()
	{
	// Issue a new Request...
	StartMonitorProperty();

	TInt val = 0;
	iProperty.Get(val);
	iObserver.PropertyChangedL(iCategory, iKey, val);
	}

void CPubSubObserver::DoCancel()
	{
	iProperty.Cancel();
	}

void CPubSubObserver::StartMonitorProperty()
	{
	if (!IsActive())
		{
		// now subscribe 
		iProperty.Subscribe(iStatus);
		SetActive();
		}
	}

void CPubSubObserver::ConstructL()
	{
	// attach to the property
	User::LeaveIfError(iProperty.Attach(iCategory, iKey, EOwnerThread));
	}
