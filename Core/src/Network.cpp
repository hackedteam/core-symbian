
#include "Network.h"

const TInt KTimeOut = 3*60*1000000;

// -----------------------------------------------------------------------------
// CNetwork* CNetwork::NewL(MNetworkObserver& aObserver)
// -----------------------------------------------------------------------------
//
CNetwork* CNetwork::NewL(RSocketServ& aSocketServ, RConnection& aConnection, MNetworkObserver& aObserver)
	{
	CNetwork* self = CNetwork::NewLC(aSocketServ, aConnection, aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

// -----------------------------------------------------------------------------
// CNetwork* CNetwork::NewLC(MNetworkObserver& aObserver)
// -----------------------------------------------------------------------------
//
CNetwork* CNetwork::NewLC(RSocketServ& aSocketServ, RConnection& aConnection, MNetworkObserver& aObserver)
	{
	CNetwork* self = new (ELeave) CNetwork(aSocketServ, aConnection, aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

// -----------------------------------------------------------------------------
// CNetwork::CNetwork()
// Constructor for performing 1st stage construction
// -----------------------------------------------------------------------------
//
CNetwork::CNetwork(RSocketServ& aSocketServ, RConnection& aConnection, MNetworkObserver& aObserver) :
	CActive(EPriorityStandard), iSocketServ(aSocketServ), iConnection(aConnection), iConnectionState(
			EConnectionStateDisconnected), iObserver(aObserver)
	{
	// No implementation required
	}

CNetwork::~CNetwork()
	{
	Cancel(); // Cancel any request, if outstanding 
	delete iTimer;
	iResolver.Close();
	iSocket.Close();
	delete iWriteBuffer;
	}

// -----------------------------------------------------------------------------
// void CNetwork::ConstructL()
// EPOC default constructor for performing 2nd stage construction
// -----------------------------------------------------------------------------
//
void CNetwork::ConstructL()
	{
	CActiveScheduler::Add(this);
	iTimer = CTimeOutTimer::NewL(*this);
	}


TBool CNetwork::SendInProgressL()
	{
	return (iConnectionState == EConnectionStateSending);
	}


// -----------------------------------------------------------------------------
// void CNetwork::RunL()
// Handle completion
// -----------------------------------------------------------------------------
//
void CNetwork::RunL()
	{
	// Cancel the timer
	iTimer->Cancel();
	
	TInt res = iStatus.Int();
	if (iStatus != KErrNone && iStatus != KErrEof)
		{
		HandleConnectionErrorL(res);
		return;
		}
	switch (iConnectionState)
		{
		case EConnectionStateDnsLookingUp:
			HandleDnsLookingUpCompleteL();
			break;
		case EConnectionStateConnecting:
			HandleConnectionCompleteL();
			break;
		case EConnectionStateReceiving:
			HandleDataReceivedL();
			break;
		case EConnectionStateSending:
			HandleSendingCompleteL();
			break;
		default:
			break;
		}
	}

// -----------------------------------------------------------------------------
// void CNetwork::DoCancel()
// Handle cancellation of outstanding request
// -----------------------------------------------------------------------------
//
void CNetwork::DoCancel()
	{
	iTimer->Cancel();
	
	switch (iConnectionState)
		{
		case EConnectionStateDnsLookingUp:
			iResolver.Cancel();
			break;
		case EConnectionStateConnecting:
			iSocket.CancelConnect();
			break;
		case EConnectionStateReceiving:
			iSocket.CancelRead();
			break;
		case EConnectionStateSending:
			iSocket.CancelWrite();
			break;
		default:
			break;
		}
	}

// -----------------------------------------------------------------------------
// void CNetwork::ConnectToServerL(const TDesC8& aUrl, TInt aPort)
// Connects to the service
// -----------------------------------------------------------------------------
//
void CNetwork::ConnectToServerL(const TDesC& aUrl, TInt aPort)
	{
	Cancel();
	iResolver.Close();
	iSocket.Close();

	iPort = aPort;

	TInetAddr netAddr;
	if (netAddr.Input(aUrl) == KErrNone)
		{
		ConnectL(netAddr.Address());
		return;
		}
	
	// Initiate DNS
	User::LeaveIfError(iResolver.Open(iSocketServ, KAfInet, KProtocolInetUdp, iConnection));
	iResolver.GetByName(aUrl, iNameEntry, iStatus);
	
	iConnectionState = EConnectionStateDnsLookingUp;
	SetActive();
	
	iTimer->After( KTimeOut );
	}

// -----------------------------------------------------------------------------
// void CNetwork::Disconnect()
// Disconnects the object from the network
// -----------------------------------------------------------------------------
//
void CNetwork::Disconnect()
	{
	Cancel();  // Cancel any request if outstanding

	if (iConnectionState >= EConnectionStateConnecting)
		{
		iSocket.Close();
		iResolver.Close();  // jo: close here, otherwise connection icon showing for too long 
		iConnectionState = EConnectionStateDisconnected;
		}
	}

// -----------------------------------------------------------------------------
// void CNetwork::SendL(const TDesC& aDataBuffer)
// Sends data through network
// -----------------------------------------------------------------------------
//
void CNetwork::SendL(const TDesC8& aDataBuffer)
	{
	if (iConnectionState == EConnectionStateDisconnected)
		{
		return;
		//User::Leave(KErrNotReady);
		}

	Cancel();

	delete iWriteBuffer;
	iWriteBuffer = NULL;
	iWriteBuffer = aDataBuffer.AllocL();
	iSocket.Write(*iWriteBuffer, iStatus);
	iConnectionState = EConnectionStateSending;
	SetActive();
	
	iTimer->After( KTimeOut );
	}

// -----------------------------------------------------------------------------
// CNetwork::ConnectL(TUint32 aAddr)
// Tries to connect object to the remote end
// -----------------------------------------------------------------------------
//
void CNetwork::ConnectL(TUint32 aAddr)
	{
	TInetAddr inetAddr;
	inetAddr.SetPort(iPort);
	inetAddr.SetAddress(aAddr);
	inetAddr.SetFamily(KAfInet);

	User::LeaveIfError(iSocket.Open(iSocketServ, KAfInet, KSockStream, KProtocolInetTcp, iConnection));

	iSocket.Connect(inetAddr, iStatus);
	iConnectionState = EConnectionStateConnecting;
	SetActive();
	
	iTimer->After( KTimeOut );
	}

// -----------------------------------------------------------------------------
// void CNetwork::DoReceiveL()
// Starts data receiving. Received data will be put in the object buffer
// -----------------------------------------------------------------------------
//
void CNetwork::DoReceiveL()
	{
	iReadBuffer.Zero();
	iSocket.RecvOneOrMore(iReadBuffer, 0, iStatus, iBytesRead);
	iConnectionState = EConnectionStateReceiving;
	SetActive();
	
	iTimer->After( KTimeOut );
	}

// -----------------------------------------------------------------------------
// void CNetwork::HandleDnsLookingUpCompleteL()
// Handle DNS looking up success completion
// -----------------------------------------------------------------------------
//
void CNetwork::HandleDnsLookingUpCompleteL()
	{
	iResolver.Close();

	TNameRecord nameRecord = iNameEntry();
	TInetAddr netAddr = TInetAddr::Cast(nameRecord.iAddr);
	// TBuf<15> ipAddr;
	// netAddr.Output(ipAddr);
	ConnectL( netAddr.Address() );
	}

// -----------------------------------------------------------------------------
// void CNetwork::HandleConnectionCompleteL()
// Handle connection to server success completion
// -----------------------------------------------------------------------------
//
void CNetwork::HandleConnectionCompleteL()
	{
	DoReceiveL();
	iObserver.NotifyConnectionCompleteL();
	}

/*
// -----------------------------------------------------------------------------
// void CNetwork::HandleDataReceivedL()
// Handle date received from server
// -----------------------------------------------------------------------------
//
void CNetwork::HandleDataReceivedL()
	{
	TInt error = iStatus.Int();
	HBufC8* recievedBuffer = iReadBuffer.AllocLC();
	DoReceiveL();
	iObserver.NotifyDataRecievedL(*recievedBuffer);
	CleanupStack::PopAndDestroy(recievedBuffer); 
	}
*/

// -----------------------------------------------------------------------------
// void CNetwork::HandleDataReceivedL()
// Handle date received from server
// -----------------------------------------------------------------------------
//
//Since the server closes the socket when response sending has finished, 
//we notify the observer sending an empty string
void CNetwork::HandleDataReceivedL()
	{
	TInt error = iStatus.Int();
	if(error==KErrNone)
		{
		HBufC8* receivedBuffer = iReadBuffer.AllocLC();
		DoReceiveL();
		iObserver.NotifyDataReceivedL(*receivedBuffer);
		CleanupStack::PopAndDestroy(receivedBuffer); 
		}
	else if(error == KErrEof)
		{
		iObserver.NotifyDataReceivedL(KNullDesC8);
		}
	}

// -----------------------------------------------------------------------------
// void CNetwork::HandleSendingCompleteL()
// Handle sending data success completion
// -----------------------------------------------------------------------------
//
void CNetwork::HandleSendingCompleteL()
	{
	TInt err = iStatus.Int();
	DoReceiveL();
	iObserver.NotifySendingCompleteL();
	}

// -----------------------------------------------------------------------------
// void CNetwork::HandleConnectionErrorL(TInt aError)
// Handle error happened during network operation
// -----------------------------------------------------------------------------
//
void CNetwork::HandleConnectionErrorL(TInt aError)
	{
	switch (iConnectionState)
		{
		case EConnectionStateDnsLookingUp:
			iResolver.Close();
			break;
		case EConnectionStateConnecting:
		case EConnectionStateReceiving:
		case EConnectionStateSending:
			iSocket.CancelAll();
			iSocket.Close();
			break;
		default:
			break;
		}

	iConnectionState = EConnectionStateDisconnected;
	iObserver.NotifyNetworkError(aError);
	}

/*
TBool CNetwork::KeyEventCaptured(TWsEvent aEvent){
	Disconnect();
	return ETrue;
}
*/

void CNetwork::TimerExpiredL(TAny* src)
	{
	Cancel();
	HandleConnectionErrorL(KErrTimedOut);
	}
