/*
 * StateAutentication.h
 *
 *  Created on: 13/feb/2011
 *      Author: Giovanna
 */

#ifndef STATEAUTENTICATION_H_
#define STATEAUTENTICATION_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <Etel3rdParty.h>

#include "AbstractState.h"

// CLASS DECLARATION
class CStateAuthentication : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateAuthentication();

	/**
	 * Two-phased constructor.
	 */
	static CStateAuthentication* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateAuthentication* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateAuthentication(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

private:
	TBuf8<16>  	iSignKey;
	TBuf8<16>	iConfKey;
	
	TBuf8<16>	iKd;
	TBuf8<16>	iNonce;
	
	HBufC8*		iRequestData;  //data for the request
	HBufC8*		iResponseData; //response data
	};

#endif /* STATEAUTENTICATION_H_ */
