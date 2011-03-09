#include <e32base.h>
#include <e32def.h>
#include <e32des8.h>

#include <f32file.h>
#include <S32MEM.H>
#include "SharedQueue.pan"
#include "SharedQueueErr.h"
#include "SharedQueueSession.h"
#include "SharedQueueCliSrv.h"

// Forward Declaration
void PanicClient(const RMessagePtr2& aMessage, TSharedQueuePanic aPanic);

// Gets a reference to the Server
inline CSharedQueueSrv& CSharedQueueSession::Server()
	{
	return *static_cast<CSharedQueueSrv*> (const_cast<CServer2*> (CSession2::Server()));
	}

CSharedQueueSession::CSharedQueueSession()
	{
	}

///////////////////////

void CSharedQueueSession::CreateL()
//
// 2nd phase construct for sessions - called by the CServer framework
//
	{
	Server().AddSession();
	__FLOG_OPEN_ID("HT", "Session.txt");
	__FLOG(_L("-------------"));
	}

CSharedQueueSession::~CSharedQueueSession()
	{
	__FLOG(_L("Destructor"));
	Server().DropSession();
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

void CSharedQueueSession::IsEmptyL(const RMessage2& aMessage)
	{
	TBool empty = Server().IsEmpty();
	TPckgBuf<TBool> pckg(empty);
	aMessage.WriteL(0, pckg);
	aMessage.Complete(KErrNone);
	}

void CSharedQueueSession::LockTop(const RMessage2& aMessage)
	{
	TBool res = Server().LockTop();
	TPckgBuf<TBool> pckg(res);
	aMessage.WriteL(0, pckg);
	aMessage.Complete(KErrNone);
	}

void CSharedQueueSession::TopL(const RMessage2& aMessage)
	{
	TCmdStruct cmd = Server().TopL();
	TPckgBuf<TCmdStruct> pckg(cmd);
	aMessage.WriteL(0, pckg);
	aMessage.Complete(KErrNone);
	}

void CSharedQueueSession::TopParamL(const RMessage2& aMessage)
	{
	TInt cliBufferLen = aMessage.GetDesMaxLength(0);
	HBufC8* paramToSend = Server().TopParamL();
	if (cliBufferLen < paramToSend->Length())
		{
		aMessage.Complete(KErrUnderflow);
		return;
		}
	aMessage.WriteL(0, paramToSend->Des());
	aMessage.Complete(KErrNone);
	}

void CSharedQueueSession::DequeueL(const RMessage2& aMessage)
	{
	TCmdStruct cmd = Server().DequeueL();
	TPckgBuf<TCmdStruct> pckg(cmd);
	aMessage.WriteL(0, pckg);
	aMessage.Complete(KErrNone);
	}

void CSharedQueueSession::EnqueueL(const RMessage2& aMessage)
	{
	TPckgBuf<TCmdStruct> pckg;
	aMessage.ReadL(0, pckg);
	TCmdStruct cmd = pckg();

	// Read the params buffer
	TInt params_len = aMessage.GetDesLength(1);
	HBufC8* buff = HBufC8::NewLC(params_len);
	TPtr8 ptrBuf = buff->Des();
	aMessage.ReadL(1, ptrBuf);

	Server().EnqueueL(cmd, buff->Des());

	CleanupStack::PopAndDestroy(buff);
	aMessage.Complete(KErrNone);
	}

void CSharedQueueSession::DoEmptyL(const RMessage2& aMessage)     // added jo
	{
	Server().DoEmptyL();
	
	aMessage.Complete(KErrNone);
	}

void CSharedQueueSession::ServiceL(const RMessage2& aMessage)
//
// Handle a client request.
// Leaving is handled by ::ServiceError() which reports
// the error code to the client
//
	{
	__FLOG_1(_L("ServiceL:%d"), aMessage.Function());
	switch (aMessage.Function())
		{
		case ELockTop:
			LockTop(aMessage);
			break;
		case EIsEmpty:
			IsEmptyL(aMessage);
			break;
		case EEnqueue:
			EnqueueL(aMessage);
			break;
		case EDequeue:
			DequeueL(aMessage);
			break;
		case ETop:
			TopL(aMessage);
			break;
		case ETopParam:
			TopParamL(aMessage);
			break;
		case EDoEmpty:       // added jo  
			DoEmptyL(aMessage);
			break;
		default:
			__FLOG(_L("!!!PanicClient!!!"));
			PanicClient(aMessage, EPanicIllegalFunction);
			break;
		}
	}

void CSharedQueueSession::ServiceError(const RMessage2& aMessage, TInt aError)
//
// Handle an error from CSharedQueueSession::ServiceL()
// A bad descriptor error implies a badly programmed client, so panic it;
// otherwise use the default handling (report the error to the client)
//
	{
	__FLOG_1(_L("!!!ServiceError:%d!!!"), aError);
	if (aError == KErrBadDescriptor)
		PanicClient(aMessage, EPanicBadDescriptor);
	if (aError == KErrQueueIsEmpty)
		PanicClient(aMessage, EPanicQueueIsEmpty);
	CSession2::ServiceError(aMessage, aError);
	}

void PanicClient(const RMessagePtr2& aMessage, TSharedQueuePanic aPanic)
//
// RMessage::Panic() also completes the message. This is:
// (a) important for efficient cleanup within the kernel
// (b) a problem if the message is completed a second time
//
	{
	_LIT(KPanic, "SharedQueueSession");
	aMessage.Panic(KPanic, aPanic);
	}

