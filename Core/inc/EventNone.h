/*
 ============================================================================
 Name		: EventNone.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventNone declaration
 ============================================================================
 */

#ifndef EVENTNONE_H
#define EVENTNONE_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractEvent.h"

// CLASS DECLARATION

/**
 *  CEventNone
 * 
 */
class CEventNone : public CAbstractEvent
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventNone();

	/**
	 * Two-phased constructor.
	 */
	static CEventNone* NewL(TEventType aId, const TDesC8& params, TUint32 aEventInstance);

	/**
	 * Two-phased constructor.
	 */
	static CEventNone* NewLC(TEventType aId, const TDesC8& params, TUint32 aEventInstance);

protected:
	// from CAbstractQueueEndPoint
	virtual void StartEventL();

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventNone(TEventType aId, TUint32 aEventInstance);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	};

#endif // EVENTNONE_H
