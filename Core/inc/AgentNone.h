/*
 ============================================================================
 Name		: AgentNone.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentNone declaration
 ============================================================================
 */

#ifndef AGENTNONE_H
#define AGENTNONE_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractAgent.h"
// CLASS DECLARATION

/**
 *  CAgentNone
 * 
 */
class CAgentNone : public CAbstractAgent
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentNone();

	/**
	 * Two-phased constructor.
	 */
	static CAgentNone* NewL(TAgentType aId, const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentNone* NewLC(TAgentType aId, const TDesC8& params);

protected:
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentNone(TAgentType aId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	};

#endif // AGENTNone_H
