/*
 * Monitor.cpp
 *
 *  Created on: 
 *      Author: 
 */

#include <APGWGNAM.H>
#include <COEDEF.h>
#include <w32std.h>

#include "Monitor.h"

CEventCapturer* CEventCapturer::NewL(MMonitorObserver& aObserver)
	{
	CEventCapturer* self = CEventCapturer::NewLC(aObserver);
	CleanupStack::Pop(self);
	return self;
	}
 
CEventCapturer* CEventCapturer::NewLC(MMonitorObserver& aObserver)
	{
	CEventCapturer* self = new (ELeave) CEventCapturer(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}
 

//CEventCapturer::CEventCapturer(MMonitorObserver& aObserver):CActive(EPriorityHigh),iObserver(aObserver)
/*CEventCapturer::CEventCapturer(MMonitorObserver& aObserver):CActive(EPriorityStandard),iObserver(aObserver)
{
	 
}*/
// http://wiki.forum.nokia.com/index.php/KIS001547_-_Capturing_Send/End_key_events_fails_on_S60_3rd_Edition,_FP2
CEventCapturer::CEventCapturer(MMonitorObserver& aObserver):CActive(EPriorityHigh),iObserver(aObserver)
{
 
}
CEventCapturer::~CEventCapturer()
{
	if(IsActive()){
		Cancel();
	}
	iWg.Close();
	iWsSession.Close();
}
 
void CEventCapturer::ConstructL()
{
	User::LeaveIfError(iWsSession.Connect());
	
	CActiveScheduler::Add(this);
		
	iWg=RWindowGroup(iWsSession);
	User::LeaveIfError(iWg.Construct((TUint32)&iWg, EFalse));
 
	// How SetOrdinalPosition works in this case:
	// We put ourselves at position 1, that means exactly behind the foremost application that has position 0
	// _but_ we capture keys with the highest priority
	// this way, we can forward the key to the foremost application at position 0
	iWg.SetOrdinalPosition(1, ECoeWinPriorityAlwaysAtFront+2);
	iWg.EnableReceiptOfFocus(ETrue); //Whether this window group can accept keyboard focus.
	
	CApaWindowGroupName* wn=CApaWindowGroupName::NewLC(iWsSession);
	wn->SetHidden(ETrue);
	wn->SetWindowGroupName(iWg);
	CleanupStack::PopAndDestroy();
	
	iContinue = ETrue;
}
 

void CEventCapturer::RunL()
{
	if (iStatus == KErrNone) 
	{
		
		TWsEvent e;
		iWsSession.GetEvent(e);
		TInt type = e.Type();
 
		switch (type)
		{
			case EEventKey:	
			case EEventCaseOpened:
			case EEventKeyUp:
			case EEventKeyDown:
				{
					TBool consumed = iObserver.KeyEventCaptured(e);
					iContinue = EFalse;
				}
				break;
			default:
				break;
			
		}; 
		
	}
	
	if (iContinue){
		Listen();
	}
}

void CEventCapturer::DoCancel()
{
	if(IsActive()){
	iWsSession.EventReadyCancel();
	}
}
 
 
void CEventCapturer::Listen()
{
	if(!IsActive()){
		iWsSession.EventReady(&iStatus);
		SetActive();
	}
}
