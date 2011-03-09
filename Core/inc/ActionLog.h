/*
 * ActionLog.h
 *
 *  Created on: 27/set/2010
 *      Author: Giovanna
 */

#ifndef ACTIONLOG_H_
#define ACTIONLOG_H_

#include "AbstractAction.h"

/**
 *  CActionLog
 * 
 */
class CActionLog : public CAbstractAction
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CActionLog();

	/**
	 * Two-phased constructor.
	 */
	static CActionLog* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CActionLog* NewLC(const TDesC8& params);

protected:
	// from CAbstractAction
	virtual void DispatchStartCommandL();

private:
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CActionLog();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
private:
	HBufC8* iLogText;
	};

#endif /* ACTIONLOG_H_ */
