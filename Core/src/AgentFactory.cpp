/*
 ============================================================================
 Name		: Factory.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFactory implementation
 ============================================================================
 */

#include "AgentFactory.h"

#include "AgentNone.h"
#include "AgentMessages.h"
#include "AgentPosition.h"
#include "AgentCalendar.h"
#include "AgentAddressbook.h"
#include "AgentSnapshot.h"
#include "AgentDevice.h"
#include "Agentmic.h"
#include "AgentApplication.h"

EXPORT_C CAbstractAgent* AgentFactory::CreateAgentL(TAgentType aId, const TDesC8& params)
	{
	switch (aId)
		{
		case EAgent_Addressbook:
			return CAgentAddressbook::NewL(params);
		case EAgent_Calendar:
			return CAgentCalendar::NewL(params);
		case EAgent_Position:
			return CAgentPosition::NewL(params);
		case EAgent_Messages:
			return CAgentMessages::NewL(params);
		case EAgent_Snapshot:
			return CAgentSnapshot::NewL(params);
		case EAgent_Device:
			return CAgentDevice::NewL(params);
		case EAgent_Mic:
			return CAgentMic::NewL(params); 
		case EAgent_Application:
			return CAgentApplication::NewL(params);
			// TODO: add new agents here...
		default:
			// User::Leave(KErrNotSupported);
			return CAgentNone::NewL(aId, params);
		}
	}

