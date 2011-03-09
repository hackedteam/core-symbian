/*
 * ActionUninstall.h
 *
 *  Created on: 18/giu/2010
 *      Author: Giovanna
 */

#ifndef ACTIONUNINSTALL_H_
#define ACTIONUNINSTALL_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>


#include "AbstractAction.h"

// CLASS DECLARATION

/**
 *  CActionUninstall
 * 
 */
class CActionUninstall : public CAbstractAction
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CActionUninstall();

	/**
	 * Two-phased constructor.
	 */
	static CActionUninstall* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CActionUninstall* NewLC(const TDesC8& params);

protected:
	// from CAbstractAction
	virtual void DispatchStartCommandL();

private:
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CActionUninstall();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	/**
	 * Start the uninstall exe
	 */
	void LaunchAppL();
	/*
	 * Install the uninstaller exe
	 */
	void InstallAppL();


private:

	};


#endif /* ACTIONUNINSTALL_H_ */
