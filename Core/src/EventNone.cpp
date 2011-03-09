/*
 ============================================================================
 Name		: EventNone.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventNone implementation
 ============================================================================
 */

#include "EventNone.h"

CEventNone::CEventNone(TEventType aId, TUint32 aTriggerId) :
	CAbstractEvent(aId, aTriggerId)
	{
	// No implementation required
	}

CEventNone::~CEventNone()
	{
	}

CEventNone* CEventNone::NewLC(TEventType aId, const TDesC8& params, TUint32 aTriggerId)
	{
	CEventNone* self = new (ELeave) CEventNone(aId, aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventNone* CEventNone::NewL(TEventType aId, const TDesC8& params, TUint32 aTriggerId)
	{
	CEventNone* self = CEventNone::NewLC(aId, params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventNone::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	}

void CEventNone::StartEventL()
	{
	}
