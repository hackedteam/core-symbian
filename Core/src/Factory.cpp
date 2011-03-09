/*
 ============================================================================
 Name		: Factory.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFactory implementation
 ============================================================================
 */

#include "Factory.h"

#include <HT\EventFactory.h>
#include <HT\AgentFactory.h>
#include <HT\ActionFactory.h>

CAbstractAction* Factory::CreateActionL(TActionType aId, const TDesC8& params)
{
	return ActionFactory::CreateActionL(aId, params);
}


CAbstractAgent* Factory::CreateAgentL(TAgentType aId, const TDesC8& params)
	{
	return AgentFactory::CreateAgentL(aId, params);
	}


CAbstractEvent* Factory::CreateEventL(TEventType aId, const TDesC8& params, TInt aTriggerId)
	{
	return EventFactory::CreateEventL(aId, params, aTriggerId);
	}

