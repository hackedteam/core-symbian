/*
 ============================================================================
 Name		: AbstractAction.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractAction declaration
 ============================================================================
 */

#ifndef ABSTRACTACTION_H
#define ABSTRACTACTION_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <HT\AbstractQueueEndPoint.h>

// CLASS DECLARATION

/**
 *  CAbstractAction
 * 
 */
class CAbstractAction : public CAbstractQueueEndPoint
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	IMPORT_C ~CAbstractAction();

protected:
	/**
	 * Subclasses MUST implement this method and eventually call the method MarkCommandAsDispatchedL();
	 */
	IMPORT_C virtual void DispatchStartCommandL()=0;
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	IMPORT_C CAbstractAction(TActionType aType);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	IMPORT_C void BaseConstructL(const TDesC8& params);

private:
	virtual void DispatchCommandL(TCmdStruct aCommand);
	};

#endif // ABSTRACTACTION_H
