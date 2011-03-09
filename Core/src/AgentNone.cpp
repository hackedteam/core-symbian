/*
 ============================================================================
 Name		: AgentNone.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentNone implementation
 ============================================================================
 */

#include "AgentNone.h"

CAgentNone::CAgentNone(TAgentType aId) :
	CAbstractAgent(aId)
	{
	// No implementation required
	}

CAgentNone::~CAgentNone()
	{
	}

CAgentNone* CAgentNone::NewLC(TAgentType aId, const TDesC8& params)
	{
	CAgentNone* self = new (ELeave) CAgentNone(aId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentNone* CAgentNone::NewL(TAgentType aId, const TDesC8& params)
	{
	CAgentNone* self = CAgentNone::NewLC(aId, params);
	CleanupStack::Pop(); // self;
	return self;
	}

void CAgentNone::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	}

void CAgentNone::StartAgentCmdL()
	{
	}

void CAgentNone::StopAgentCmdL()
	{
	}

