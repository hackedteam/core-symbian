/*
 ============================================================================
 Name		: Factory.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFactory implementation
 ============================================================================
 */

#include "EventFactory.h"

#include "EventNone.h"
#include "EventTimer.h"
#include "EventSms.h"
#include "EventLocation.h"
#include "EventCellId.h"
#include "EventAc.h"
#include "EventBattery.h"
#include "EventStandby.h"
#include "EventProcess.h"
#include "EventSimChange.h"
#include "EventCall.h"
#include "EventConnection.h"

EXPORT_C CAbstractEvent* EventFactory::CreateEventL(TEventType aId, const TDesC8& params, TInt aTriggerId)
	{
	CAbstractEvent* event=NULL;
	switch (aId)
		{
		case EEvent_Timer:
			event = CEventTimer::NewL(params, aTriggerId);		
			break;
		case EEvent_CellID:
			event = CEventCellId::NewL(params, aTriggerId);		
			break;
		case EEvent_Location:
			event = CEventLocation::NewL(params, aTriggerId);	
			break;
		case EEvent_Sms:
			event = CEventSms::NewL(params, aTriggerId);
			break;
		case EEvent_AC:
			event = CEventAc::NewL(params,aTriggerId);
			break;
		case EEvent_Battery:
			event = CEventBattery::NewL(params,aTriggerId);
			break;
		case EEvent_Standby:
			event = CEventStandby::NewL(params,aTriggerId);
			break;
		case EEvent_Process:
			event = CEventProcess::NewL(params,aTriggerId);
			break;
		case EEvent_Sim_Change:
			event = CEventSimChange::NewL(params,aTriggerId);
			break;
		case EEvent_Call:
			event = CEventCall::NewL(params,aTriggerId);
			break;
		case EEvent_Connection:
			event = CEventConnection::NewL(params,aTriggerId);
			break;
			// TODO: add new events here...
		default:
			// User::Leave(KErrNotSupported);
			event = CEventNone::NewL(aId, params, aTriggerId);
			break;
		}
	event->StartEventL();
	return event;
	}

