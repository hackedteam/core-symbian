#include "RecvSmsSocket.h"
#include <smsuaddr.h>	// KSmsAddrFamily
#include <gsmubuf.h>	// CSmsBuffer
#include <smsustrm.h>	// RSmsSocketReadStream
#include <s32file.h>
#include <smutset.h>

//Constructor
CSmsReceiverSocket::CSmsReceiverSocket(MSmsRecvHandler& aHandler, RFs& fs, RSocketServ& aSocketServer) :
	CActive(EPriorityStandard), iHandler(aHandler), iFs(fs), iSocketServer(aSocketServer)
	{
	CActiveScheduler::Add(this);
	}

CSmsReceiverSocket* CSmsReceiverSocket::NewL(MSmsRecvHandler& aHandler, RFs& fs, RSocketServ& aSocketServer)
	{
	// two phase construction
	CSmsReceiverSocket* self = new (ELeave) CSmsReceiverSocket(aHandler, fs, aSocketServer);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void CSmsReceiverSocket::ConstructL()
	{
	__FLOG_OPEN("HT", "RecvSms.txt");
	__FLOG(_L("**********"));
	}

// here u open a Socket, bind it an address and start listening
void CSmsReceiverSocket::StartReceivingL(const TDesC& matchTag)
	{
	if (iStato != EInit)
		return;
#ifndef __WINS__
	User::LeaveIfError( iRecvSocket.Open(iSocketServer, KSMSAddrFamily, KSockDatagram, KSMSDatagramProtocol) );
	iSmsAddr.SetSmsAddrFamily(ESmsAddrMatchText);
	TBuf8<25> match8;
	match8.Copy(matchTag);
	iSmsAddr.SetTextMatch(match8);
	__FLOG(_L("Bind Pass:"));
	__FLOG( matchTag );
	TInt ris = iRecvSocket.Bind(iSmsAddr);
	__FLOG_1(_L("Bind Ris:%d"), ris);
	User::LeaveIfError( ris );
	Receive();
#endif
	}

void CSmsReceiverSocket::DoCancel()
	{
	__FLOG(_L("Cancel"));
	if (iStato == EReceiving)
		iRecvSocket.CancelAll();
	}

void CSmsReceiverSocket::Receive()
	{
	__FLOG(_L("Receive"));
	iStato = EReceiving;
	iOctlRead() = KSockSelectRead;
	iRecvSocket.Ioctl(KIOctlSelect, iStatus, &iOctlRead, KSOLSocket);
	SetActive();
	}

void CSmsReceiverSocket::Confirm()
	{
	__FLOG(_L("Confirm"));
	iStato = EReceived;
	iRecvSocket.Ioctl(KIoctlReadMessageSucceeded, iStatus, NULL, KSolSmsProv);
	SetActive();
	}

void CSmsReceiverSocket::RunL()
	{
	__FLOG_1(_L("RunL.Status %d"), iStatus.Int());
	switch (iStato)
		{
		case EReceived:
			{
			// Schedule a new request
			Receive();

			__FLOG(_L("RunL.Received"));

			// Removes the TextMatch...
			// iMsg->Buffer().DeleteL(0, iSmsAddr.TextMatch().Length());
			TInt len = iMsg->Buffer().Length();
			__FLOG_1(_L("len:%d"), len);
			RBuf ris;
			ris.CleanupClosePushL();
			ris.CreateL(len + 2); // Tanto per essere sicuri che con len = 0 non si schianti...
			iMsg->Buffer().Extract(ris, 0, len);
			iHandler.IncomingSmsL(this, iMsg->ToFromAddress(), ris);
			CleanupStack::PopAndDestroy(); // ris

			break;
			}

		case EReceiving:
			{
			__FLOG(_L("RunL.Receiving"));
			if (iMsg)
				{
				delete iMsg;
				iMsg = NULL;
				}
			iMsg = CSmsMessage::NewL(iFs, CSmsPDU::ESmsDeliver, CSmsBuffer::NewL());

			RSmsSocketReadStream readStream(iRecvSocket);
			readStream >> *(iMsg);
			readStream.Close();
			__FLOG(_L("Closed Stream"));

			// Schedula la conferma
			Confirm();
			break;
			}
		default:
			break;
		}
	}

TInt CSmsReceiverSocket::RunError(TInt aError)
	{
	__FLOG_1(_L("ERROR:%d"), aError);
	return aError;
	}

CSmsReceiverSocket::~CSmsReceiverSocket()
	{
	__FLOG(_L("Destructor"));
	Cancel();
	delete iMsg;
	iRecvSocket.Close();
	__FLOG(_L("EndDestructor"));
	__FLOG_CLOSE;
	}

