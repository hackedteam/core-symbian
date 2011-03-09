
#include "SharedQueue.h"
#include "SharedQueueSrv.h"
#include "SharedQueueCliSrv.h"
#include "SharedQueueErr.h"
#include <S32MEM.H>
#include <HT\logging.h>

const TInt KServerDefaultMessageSlots = 1; //Number of message slots available per session.

LOCAL_C TInt CreateServerProcess(RSemaphore& sem)
	{
	const TUidType serverUid(KNullUid, KNullUid, KSharedQueueSrvUid3);

	RProcess server;
	TInt err = server.Create(KSharedQueueSrvImg, KNullDesC, serverUid);
	if (err != KErrNone)
		{
		return err;
		}

	server.Resume();
	err = sem.Wait(5000000);
	TInt r = (server.ExitType() == EExitPanic) ? KErrGeneral : err;
	server.Close();
	return r;
	}

LOCAL_C TInt StartServer()
	{
	// Il FindServer non e' suffic. perche' il nome del server sara' assegnato diverso tempo dopo la creazione del processo... 
	TFindServer find(KSharedQueueSrvName);
	TFullName name;
	TInt err = find.Next(name); // KErrNone if a matching server is found, KErrNotFound otherwise. 
	if (err != KErrNone)
		{
		// Probably the Server is not Running
		RSemaphore sem;
		err = sem.CreateGlobal(KSharedQueueSrvImg, 0);
		if (err != KErrNone)
			{
			// Server is already Running...
			return KErrNone;
			}
		// Server not Running		
		err = CreateServerProcess(sem);
		sem.Close();
		}
	return err;
	}

EXPORT_C TInt RSharedQueue::Connect()
	{
	TInt i = 0;
	TInt errno;
	do
		{
		errno = StartServer();
		i++;
		}
	while (errno != KErrNone && i < 5);
	if (errno == KErrNone)
		{
		errno = CreateSession(KSharedQueueSrvName, TVersion(KServMajorVersionNumber, KServMinorVersionNumber,
				KServBuildVersionNumber), KServerDefaultMessageSlots);
		}
	return errno;
	}

EXPORT_C void RSharedQueue::Enqueue(TCmdType aType, TInt aSrc, TInt aDest, const TDesC8& params)
	{
	TCmdStruct cmd(aType, aSrc, aDest);
	Enqueue(cmd, params);
	}

EXPORT_C void RSharedQueue::Enqueue(TCmdStruct aCmd, const TDesC8& params)
	{
	TPckgBuf<TCmdStruct> pckg(aCmd);
	TIpcArgs args(&pckg, &params);
	SendReceive(EEnqueue, args);
	}

EXPORT_C TCmdStruct RSharedQueue::Dequeue()
	{
	TPckgBuf<TCmdStruct> pckg;
	TIpcArgs args(&pckg);
	SendReceive(EDequeue, args);
	TCmdStruct cmd = pckg();
	return cmd;
	}

EXPORT_C TCmdStruct RSharedQueue::Top()
	{
	TPckgBuf<TCmdStruct> pckg;
	TIpcArgs args(&pckg);
	SendReceive(ETop, args);
	TCmdStruct cmd = pckg();
	return cmd;
	}

EXPORT_C HBufC8* RSharedQueue::TopParamL()
	{
	HBufC8* buffer = HBufC8::NewLC(500);
	buffer->Des().Zero();

	TInt err = KErrNone;
	do
		{
		TIpcArgs args(&buffer->Des());
		err = SendReceive(ETopParam, args);
		if (err == KErrUnderflow)
			{
			// Increase the buffer size to receive the parameters from server...
			TInt maxLen = buffer->Des().MaxLength();
			CleanupStack::PopAndDestroy(buffer);
			buffer = HBufC8::NewLC(maxLen * 2);
			}
		}
	while (err == KErrUnderflow);

	CleanupStack::Pop(buffer);
	return buffer;
	}

EXPORT_C TBool RSharedQueue::IsEmpty()
	{
	TPckgBuf<TBool> pckg;
	TIpcArgs args(&pckg);
	SendReceive(EIsEmpty, args);
	TBool res = pckg();
	return res;
	}



EXPORT_C TBool RSharedQueue::LockTop()
	{
	TPckgBuf<TBool> pckg;
	TIpcArgs args(&pckg);
	SendReceive(ELockTop, args);
	TBool res = pckg();
	return res;
	}

EXPORT_C void RSharedQueue::DoEmpty()   // added j
	{
	  
		SendReceive(EDoEmpty);
	}

