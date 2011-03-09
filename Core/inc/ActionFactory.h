/*
 ============================================================================
 Name		: Factory.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFactory declaration
 ============================================================================
 */

#ifndef ACTIONFACTORY_H
#define ACTIONFACTORY_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <HT\AbstractAction.h>

// CLASS DECLARATION

/**
 *  Factory
 */
class ActionFactory
	{
public:
	/**
	 * Creates a new Action instance
	 */
	IMPORT_C static CAbstractAction* CreateActionL(TActionType aId, const TDesC8& params);
	};

#endif // FACTORY_H
