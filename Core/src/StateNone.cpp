/*
 ============================================================================
 Name		: StateNone.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CChallenge implementation
 ============================================================================
 */

#include "StateNone.h"


CStateNone::CStateNone(MStateObserver& aObserver) : CAbstractState(EState_None, aObserver)
	{
	// No implementation required
	}

CStateNone::~CStateNone()
	{
	}

CStateNone* CStateNone::NewLC(MStateObserver& aObserver)
	{
	CStateNone* self = new (ELeave) CStateNone(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateNone* CStateNone::NewL(MStateObserver& aObserver)
	{
	CStateNone* self = CStateNone::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateNone::ConstructL()
	{

	}

void CStateNone::ActivateL(const TDesC8& aData)
	{
	}

void CStateNone::ProcessDataL(const TDesC8& aData) 
	{
	}

