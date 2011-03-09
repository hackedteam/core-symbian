
// Splitting long running tasks with active objects
// http://wiki.forum.nokia.com/index.php/Splitting_long_running_tasks_with_active_objects

#include "LongRunTask.h"

CLongTaskAO::CLongTaskAO(MLongTaskCallBack& callBack) : CActive(EPriorityIdle), iCallBack(callBack)
{
	 CActiveScheduler::Add( this );
}

CLongTaskAO* CLongTaskAO::NewL(MLongTaskCallBack& callBack)
{
	CLongTaskAO* self = new (ELeave) CLongTaskAO(callBack);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);	// self
	return self;
}

void CLongTaskAO::ConstructL()
{
}
 

CLongTaskAO::~CLongTaskAO()
{
	Cancel();
}


void CLongTaskAO::DoCancel()
{
	// Implementation not needed.
}


void CLongTaskAO::NextRound()
{
	Cancel();
	SetActive();
	TRequestStatus* status=&iStatus;
	User::RequestComplete(status, KErrNone);
}


void CLongTaskAO::RunL()
{
	// Place this AO at the end of the Scheduler... Check comment below (*)
	Deque();
	CActiveScheduler::Add( this );
	iCallBack.DoOneRoundL();
}

// (*)
// http://developer.symbian.org/wiki/index.php/Active_Objects_%28Fundamentals_of_Symbian_C%2B%2B%29
// Note that there is a problem with running two or more background tasks in the same thread – because the active scheduler looks for the first eligible active object in the order they were added to the scheduler (and starts at the top of the list each time). The first background task added to the scheduler will run to completion, starting any other active objects with the same or lower priority. 
// The solution to this is that background active objects should call Deque()  on themselves, and then CActiveScheduler::Add() to add themselves to the end of the scheduler’s list. If all background active objects do this then there is a co-operative round-robin effect. 
