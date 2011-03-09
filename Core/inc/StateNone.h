/*
 ============================================================================
 Name		: StateNone.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CStateNone declaration
 ============================================================================
 */

#ifndef STATENone_H
#define STATENone_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractState.h"

// CLASS DECLARATION


/**
 *  CStateNone
 * 
 */
class CStateNone : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateNone();

	/**
	 * Two-phased constructor.
	 */
	static CStateNone* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateNone* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateNone(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

	};

#endif // StateNone_H
