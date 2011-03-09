/*
 ============================================================================
 Name		: AbstractAction.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractAction implementation
 ============================================================================
 */

#include "AbstractAction.h"

EXPORT_C CAbstractAction::CAbstractAction(TActionType aType) :
	CAbstractQueueEndPoint(aType)
	{
	// No implementation required
	}

EXPORT_C CAbstractAction::~CAbstractAction()
	{
	}

EXPORT_C void CAbstractAction::BaseConstructL(const TDesC8& params)
	{
	CAbstractQueueEndPoint::BaseConstructL(params);
	}


void CAbstractAction::DispatchCommandL(TCmdStruct aCommand)
	{
	// Actions will receive only the Start command
	switch (aCommand.iType)
		{
		case EStart:
			DispatchStartCommandL();
			// Agents will not receive any more commands after the "Start"
			SetReceiveCmd(EFalse);
			break;
		case EStop:
			ASSERT(false);
			break;
		default:
			ASSERT(false);
			break;
		}
	}
