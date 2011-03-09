/*
 ============================================================================
 Name		: ActionNone.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CActionNone declaration
 ============================================================================
 */

#ifndef ACTIONNONE_H
#define ACTIONNONE_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractAction.h"

// CLASS DECLARATION

/**
 *  CActionNone
 * 
 */
class CActionNone : public CAbstractAction
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CActionNone();

	/**
	 * Two-phased constructor.
	 */
	static CActionNone* NewL(TActionType aId, const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CActionNone* NewLC(TActionType aId, const TDesC8& params);

protected:
	// from CAbstractAction
	virtual void DispatchStartCommandL();

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CActionNone(TActionType aId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	};

#endif // ActionNONE_H
