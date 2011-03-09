/*
 Livello intermedio smsstack via RSocket (smsprot.prt)
 */

#ifndef __RECVSMSSOCKET_H__
#define __RECVSMSSOCKET_H__

#include <e32base.h>	// CActive
#include <es_sock.h>	// RSocket
#include <f32file.h>	// RFs
#include <gsmumsg.h>	// CSmsMessage
#include <smsuaddr.h>	// TSmsAddr
#include <HT\logging.h>

class MSmsRecvHandler
	{
public:
	virtual void IncomingSmsL(const TAny* src, const TDesC& aFromNumber, const TDesC& aData)=0;
	};

class CSmsReceiverSocket : public CActive
	{
	enum TReceiveStatus
		{
		EInit, EReceiving, EReceived
		};

public:
	static CSmsReceiverSocket* NewL(MSmsRecvHandler& aHandler, RFs& fs, RSocketServ& aSocketServer);
	CSmsReceiverSocket(MSmsRecvHandler& aHandler, RFs& fs, RSocketServ& aSocketServer);
	virtual ~CSmsReceiverSocket();

	// Invia una richiesta asincrona
	void StartReceivingL(const TDesC& matchTag);

protected:
	void ConstructL();

private:
	// from CActive
	TInt RunError(TInt aError);
	void RunL();
	void DoCancel();
	void Confirm();
	void Receive();

private:
	MSmsRecvHandler& iHandler;
	RFs& iFs;
	RSocketServ& iSocketServer;
	TSmsAddr iSmsAddr;
	TReceiveStatus iStato;
	RSocket iRecvSocket; // own
	TPckgBuf<TUint> iOctlRead; // own
	CSmsMessage* iMsg; // own
	__FLOG_DECLARATION_MEMBER
	};

#endif

