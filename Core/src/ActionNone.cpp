/*
 ============================================================================
 Name		: ActionNone.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CActionNone implementation
 ============================================================================
 */

#include "ActionNone.h"

CActionNone::CActionNone(TActionType aId) :
	CAbstractAction(aId)
	{
	// No implementation required
	}

CActionNone::~CActionNone()
	{
	}

CActionNone* CActionNone::NewLC(TActionType aId, const TDesC8& params)
	{
	CActionNone* self = new (ELeave) CActionNone(aId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionNone* CActionNone::NewL(TActionType aId, const TDesC8& params)
	{
	CActionNone* self = CActionNone::NewLC(aId, params);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionNone::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	}

void CActionNone::DispatchStartCommandL()
	{
	MarkCommandAsDispatchedL();
	}

