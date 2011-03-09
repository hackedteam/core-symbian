/*
 * StateIdentification.h
 *
 *  Created on: 16/feb/2011
 *      Author: Giovanna
 */

#ifndef STATEBYE_H
#define STATEBYE_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractState.h"

// CLASS DECLARATION


/**
 *  CStateBye
 * 
 */
class CStateBye : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateBye();

	/**
	 * Two-phased constructor.
	 */
	static CStateBye* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateBye* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateBye(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

private:
	
	TBuf8<16>		iSignKey;
	HBufC8*			iRequestData;
	HBufC8*			iResponseData;
	};

#endif // StateBye_H
