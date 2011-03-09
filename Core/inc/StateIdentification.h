/*
 * StateIdentification.h
 *
 *  Created on: 16/feb/2011
 *      Author: Giovanna
 */

#ifndef STATEIDENTIFICATION_H_
#define STATEIDENTIFICATION_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <Etel3rdParty.h>

#include "AbstractState.h"

// CLASS DECLARATION
class CStateIdentification : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateIdentification();

	/**
	 * Two-phased constructor.
	 */
	static CStateIdentification* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateIdentification* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateIdentification(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

private:
	TBuf8<16>	iSignKey;
	HBufC8*		iRequestData;
	HBufC8*		iResponseData; //response data
	};


#endif /* STATEIDENTIFICATION_H_ */
