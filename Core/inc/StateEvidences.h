/*
 * StateEvidences.h
 *
 *  Created on: 16/feb/2011
 *      Author: Giovanna
 */

#ifndef STATEEVIDENCES_H_
#define STATEEVIDENCES_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

#include "AbstractState.h"

// CLASS DECLARATION


/**
 *  CStateEvidences
 * 
 */
class CStateEvidences : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateEvidences();

	/**
	 * Two-phased constructor.
	 */
	static CStateEvidences* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateEvidences* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateEvidences(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

private:
	TBuf8<16>	iSignKey;
	enum TEvidencesState 
		{
		EInitState,
		ESendLogData
		};
	
	TEvidencesState iState;
	RFs iFs;
	RPointerArray<HBufC> iFileList;
	
	HBufC8*		iRequestData;
	HBufC8*		iResponseData;
	};

#endif /* STATEEVIDENCES_H_ */
