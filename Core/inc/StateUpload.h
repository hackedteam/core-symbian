/*
 * StateUpload.h
 *
 *  Created on: 27/feb/2011
 *      Author: Giovanna
 */

#ifndef STATEUPLOAD_H_
#define STATEUPLOAD_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <BADESCA.H>

#include "AbstractState.h"

// CLASS DECLARATION


/**
 *  CStateUpload
 * 
 */
class CStateUpload : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateUpload();

	/**
	 * Two-phased constructor.
	 */
	static CStateUpload* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateUpload* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateUpload(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	

private:
	
	TBuf8<16>		iSignKey;
	HBufC8*			iRequestData;
	HBufC8*			iResponseData;
	
	};


#endif /* STATEUPLOAD_H_ */
