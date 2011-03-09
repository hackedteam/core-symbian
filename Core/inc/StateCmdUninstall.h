/*
 * StateCmdUninstall.h
 *
 *  Created on: Apr 22, 2010
 *      Author: pavarang
 */

#ifndef STATECMDUNINSTALL_H_
#define STATECMDUNINSTALL_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractState.h"

// CLASS DECLARATION


/**
 *  CStateCmdUninstall
 * 
 */
class CStateCmdUninstall : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateCmdUninstall();

	/**
	 * Two-phased constructor.
	 */
	static CStateCmdUninstall* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateCmdUninstall* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateCmdUninstall(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
	void LaunchAppL();
	void InstallAppL();

	};


#endif /* STATECMDUNINSTALL_H_ */
