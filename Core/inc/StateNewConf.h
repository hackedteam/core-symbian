/*
 * StateNewConf.h
 *
 *  Created on: 17/feb/2011
 *      Author: Giovanna
 */

#ifndef STATENEWCONF_H_
#define STATENEWCONF_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractState.h"

// CLASS DECLARATION


/**
 *  CStateNewConf
 * 
 */
class CStateNewConf : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateNewConf();

	/**
	 * Two-phased constructor.
	 */
	static CStateNewConf* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateNewConf* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);   
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateNewConf(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
private:
	
	TBuf8<16>	iSignKey;
	HBufC8*		iRequestData;
	HBufC8*		iResponseData; //response data
		
	};


#endif /* STATENEWCONF_H_ */
