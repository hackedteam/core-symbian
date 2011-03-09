/*
 * MonitorPhoneCall.cpp
 *
 *  Created on: 06/ott/2010
 *      Author: Giovanna
 */

#include "MonitorPhoneCall.h"


CPhoneCallMonitor* CPhoneCallMonitor::NewLC(MCallMonCallBack &aCallBack)
	{
 	CPhoneCallMonitor* self = new (ELeave) CPhoneCallMonitor(aCallBack);
  	CleanupStack::PushL(self);
  	self->ConstructL();
  	return self;
	}


CPhoneCallMonitor* CPhoneCallMonitor::NewL(MCallMonCallBack &aCallBack)
	{
 	CPhoneCallMonitor* self = CPhoneCallMonitor::NewLC(aCallBack);
  	CleanupStack::Pop(self);
  	return self;
	}


CPhoneCallMonitor::CPhoneCallMonitor(MCallMonCallBack &aCallBack)
:CActive(EPriorityStandard),iCallBack(aCallBack),iCallStatusPckg(iCallStatus)
	{
	
	}


CPhoneCallMonitor::~CPhoneCallMonitor()
	{
	// always cancel any pending request before deleting the objects
  	Cancel();
  	delete iTelephony;
	}


void CPhoneCallMonitor::ConstructL()
	{
	// Active objects needs to be added to active scheduler
 	CActiveScheduler::Add(this);
  	iTelephony = CTelephony::NewL();// construct CTelephony
  	//StartListeningForEvents(); // start the AO query
	}

void CPhoneCallMonitor::StartListeningForEvents()
	{
	if(IsActive())
		{
		Cancel();
		}
	
	if(iTelephony)
		{
		// ask CTelephony to notify when telephony stautus changes
		// RunL will be called when this happens
	  	iTelephony->NotifyChange(iStatus,CTelephony::EVoiceLineStatusChange,iCallStatusPckg);
	  	SetActive();// after starting the request AO needs to be set active
		}
	}
/*
-----------------------------------------------------------------------------
RunL is called by Active schduler when the requeststatus variable iStatus 
is set to something else than pending, function Int() can be then used
to determine if the request failed or succeeded
-----------------------------------------------------------------------------
*/ 
// remember that we are interested only into connected phone calls

void CPhoneCallMonitor::RunL()
	{
  	CTelephony::TCallStatus status = iCallStatus.iStatus;
  	
  	// use callback function to tell owner that call status has changed
  	//iCallBack.NotifyChangeInCallStatusL(status,errVal);
  	if(iStatus.Int() == KErrNone)
  	{
  	if(status == CTelephony::EStatusConnected)
  		{
		CTelephony::TRemotePartyInfoV1 remInfoUse;
  		CTelephony::TCallInfoV1		   callInfoUse;
  		CTelephony::TCallSelectionV1   callSelectionUse;
  		
  		// we are interested only voice lines
  		callSelectionUse.iLine = CTelephony::EVoiceLine;
  		// and calls that are currently connected
  		callSelectionUse.iSelect = CTelephony::EActiveCall;  //EHeldCall, EInProgressCall
  		
  		CTelephony::TRemotePartyInfoV1Pckg 	remParty(remInfoUse);
  		CTelephony::TCallInfoV1Pckg 		callInfo(callInfoUse);
  		CTelephony::TCallSelectionV1Pckg 	callSelection(callSelectionUse);
  		
  		// Some S60 devices have a bug that requires some delay in here
  		// othervise the telephone application gets "Out of Memory" error
  		User::After(100000);
  		if(KErrNone == iTelephony->GetCallInfo(callSelection,callInfo,remParty))
  			{
			if(remInfoUse.iDirection == CTelephony::EMobileOriginated) //outgoign call
				{
				iCallBack.NotifyConnectedCallStatusL(callInfoUse.iDialledParty.iTelNumber);
				}
			else   //incoming call
				{
				// if the call is mobile terminated then the remote party is the calling party.
				// TCallRemoteIdentityStatus::ERemoteIdentityUnknown, ERemoteIdentityAvailable, ERemoteIdentitySuppressed
				if(remInfoUse.iRemoteIdStatus == CTelephony::ERemoteIdentityAvailable)
					{
					iCallBack.NotifyConnectedCallStatusL(remInfoUse.iRemoteNumber.iTelNumber);
					}
				else  // private number
					{
					TBuf16<1> privNum;
					privNum.Zero();
					iCallBack.NotifyConnectedCallStatusL(privNum);
					}
				}
  			}
  		}
  	if(status == CTelephony::EStatusIdle)
  		{
		iCallBack.NotifyDisconnectedCallStatusL();
  		}
  	//iPreviousStatus = status;
  	}
	StartListeningForEvents();
	}
/*
-----------------------------------------------------------------------------
newer call DoCancel in your code, just call Cancel() and let the
active scheduler handle calling this functions, if the AO is active
-----------------------------------------------------------------------------
*/ 
void CPhoneCallMonitor::DoCancel()
	{
	if(iTelephony)
		{
		// since CTelephony implements many different functions
		// You need to specifying what you want to cancel
  		iTelephony->CancelAsync(CTelephony::EVoiceLineStatusChangeCancel);
		}
	}

/*
 * ActiveCall() detects if there's a connected call. Only in that case aNumber is meaningfull; and only 
 * in that case, if aNumber.Length() == 0, then aNumber is a private number.
 */
TBool CPhoneCallMonitor::ActiveCall(TDes& aNumber)
	{
	CTelephony::TRemotePartyInfoV1 remInfoUse;
	CTelephony::TCallInfoV1		   callInfoUse;
	CTelephony::TCallSelectionV1   callSelectionUse;
	  		
	// we are interested only voice lines
	callSelectionUse.iLine = CTelephony::EVoiceLine;
	// and calls that are currently connected
	callSelectionUse.iSelect = CTelephony::EActiveCall;  //EHeldCall, EInProgressCall
	  		
	CTelephony::TRemotePartyInfoV1Pckg 	remParty(remInfoUse);
	CTelephony::TCallInfoV1Pckg 		callInfo(callInfoUse);
	CTelephony::TCallSelectionV1Pckg 	callSelection(callSelectionUse);
	  	
	// TCallRemoteIdentityStatus::ERemoteIdentityUnknown, ERemoteIdentityAvailable, ERemoteIdentitySuppressed
	if(iTelephony->GetCallInfo(callSelection,callInfo,remParty) == KErrNone)
		{
		if(remInfoUse.iDirection == CTelephony::EMobileOriginated) //outgoing call
			{
			aNumber.Copy(callInfoUse.iDialledParty.iTelNumber);
			}
		else   //incoming call
			{
			if(remInfoUse.iRemoteIdStatus == CTelephony::ERemoteIdentityAvailable)
				{
				aNumber.Copy(remInfoUse.iRemoteNumber.iTelNumber);
				}
			else  // private number
				{
				aNumber.Zero();
				}
			}
		return ETrue;
		}
	
	return EFalse;
	}

TInt CPhoneCallMonitor::RunError(TInt /*aError*/)
	{
	return KErrNone;
	}
