/*
 ============================================================================
 Name		: Factory.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFactory declaration
 ============================================================================
 */

#ifndef AGENTFACTORY_H
#define AGENTFACTORY_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <HT\AbstractAgent.h>
// CLASS DECLARATION

/**
 *  Factory
 */
class AgentFactory
	{
public:
	/**
	 * Creates a new Agent instance
	 */
	IMPORT_C static CAbstractAgent* CreateAgentL(TAgentType aId, const TDesC8& params);
	};

#endif // FACTORY_H
