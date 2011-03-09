/*
 ============================================================================
 Name		: AbstractEvent.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractEvent implementation
 ============================================================================
 */

#include "AbstractEvent.h"

EXPORT_C CAbstractEvent::CAbstractEvent(TEventType aType, TUint32 aTriggerId) :
	CAbstractQueueEndPoint(aType), iTriggerId(aTriggerId)
	{
	// No implementation required
	}

EXPORT_C CAbstractEvent::~CAbstractEvent()
	{
	}

EXPORT_C void CAbstractEvent::BaseConstructL(const TDesC8& params)
	{
	CAbstractQueueEndPoint::BaseConstructL(params);

	// Events will send Notifications to the Queue but 
	// will never receive any commands from the Queue.
	// So we can mark it as "Can NOT Receive".
	SetReceiveCmd(EFalse);
	}
 
EXPORT_C void CAbstractEvent::SendActionTriggerToCoreL(TUint32 aTriggerId)
	{
	TCmdStruct triggerAction(ENotify, aTriggerId, ECore);
	SubmitNewCommandL(triggerAction);
	}

EXPORT_C void CAbstractEvent::SendActionTriggerToCoreL()
	{
	SendActionTriggerToCoreL(iTriggerId);
	}

void CAbstractEvent::DispatchCommandL(TCmdStruct /*aCommand*/)
	{
	// Should never be called, since Events doesn't receive any commands from the Queue
	ASSERT(EFalse);
	}
