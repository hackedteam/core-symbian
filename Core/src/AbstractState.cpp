/*
 ============================================================================
 Name		: AbstractState.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractState implementation
 ============================================================================
 */

#include "AbstractState.h"
#include "Keys.h"

CAbstractState::CAbstractState(TState aStateId, MStateObserver& aObserver) : iState(aStateId), iObserver(aObserver)
	{
	// No implementation required
	}

CAbstractState::~CAbstractState()
	{
	}

TState CAbstractState::Type()
	{
	return iState;
	}
