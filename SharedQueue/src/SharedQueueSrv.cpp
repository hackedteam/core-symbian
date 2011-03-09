#include "SharedQueueErr.h"
#include "SharedQueue.pan"
#include "SharedQueueSrv.h"
#include "SharedQueueSession.h"
#include <e32des8.h>
#include <f32file.h>
#include <COEMAIN.H>
#include <e32property.h>
#include <HT\Processes.h>
//#include <HT\FileUtils.h>

CSharedQueueSrv* CSharedQueueSrv::NewLC()
	{
	CSharedQueueSrv* server = new (ELeave) CSharedQueueSrv(EPriorityNormal);
	CleanupStack::PushL(server);
	server->ConstructL();
	return server;
	}

CSharedQueueSrv::CSharedQueueSrv(TInt aPriority) :
	CServer2(aPriority)
	{
	}

void CSharedQueueSrv::ConstructL()
	{
	__FLOG_OPEN("HT", "Server.txt");
	__FLOG(_L("----CONSTR----"));
	TRAPD(err, StartL(KSharedQueueSrvName));

	if (err != KErrNone)
		{
		__FLOG(_L("ERRORE STARTL SERVER"));
		User::Leave(err);
		}

	static _LIT_SECURITY_POLICY_PASS(KAllowAllPolicy);
	TInt ris = 0;
	ris = RProperty::Define(KPropertyUidSharedQueue, KPropertyKeySharedQueueTopAddedOrRemoved, RProperty::EInt,
			KAllowAllPolicy, KAllowAllPolicy);
	iTopIsLocked = EFalse;
	__FLOG_1(_L("PS_Define:%d"), ris);
	}

TBool CSharedQueueSrv::LockTop()
	{
	if (iTopIsLocked)
		return EFalse;
	__FLOG(_L("LOCK TOP"));	
	iTopIsLocked = ETrue;
	return ETrue;
	}

void CSharedQueueSrv::UnlockTop()
	{
	__FLOG(_L("UNLOCK TOP"));
	iTopIsLocked = EFalse;
	}


TBool CSharedQueueSrv::IsEmpty()
	{
	return (iArray.Count() <= 0);
	}

TCmdStruct CSharedQueueSrv::TopL()
	{
	if (IsEmpty())
		User::Leave(KErrQueueIsEmpty);
	TCmdStruct top = iArray[0];
//	__FLOG_2(_L("Top Dest: %x  Type: %x"), top.iDest, top.iType);
	return top;
	}

HBufC8* CSharedQueueSrv::TopParamL()
	{
	if (IsEmpty())
		User::Leave(KErrQueueIsEmpty);
	return iParams[0];
	}

TCmdStruct CSharedQueueSrv::DequeueL()
	{
	if (IsEmpty())
		User::Leave(KErrQueueIsEmpty);
	TCmdStruct res = TopL();
	__FLOG_2(_L("Remove Dest: %x  Type: %x"), res.iDest, res.iType);
	iArray.Remove(0);
	UnlockTop();
	
	if (!IsEmpty())
		{
		TCmdStruct newTop = TopL();
		__FLOG_2(_L(" NewTop Dest: %x  Type: %x"), newTop.iDest, newTop.iType);	
		} else
		{
		__FLOG(_L(" Queue Empty!"));				
		}
	
	// Removes the parameters too
	HBufC8* buf = iParams[0];
	iParams.Remove(0);
	delete buf;

	RProperty::Set(KPropertyUidSharedQueue, KPropertyKeySharedQueueTopAddedOrRemoved, 1);
	return res;
	}

void CSharedQueueSrv::DoEmptyL()  // added jo
	{
		UnlockTop();
		while(!IsEmpty() )
			{
				iArray.Remove(0);
				HBufC8* buf = iParams[0];
				iParams.Remove(0);
				delete buf;
			}
	}

void CSharedQueueSrv::EnqueueL(TCmdStruct aCmd, const TDesC8& params)
	{
	iArray.Append(aCmd);
	iParams.Append(params.AllocL());
	RProperty::Set(KPropertyUidSharedQueue, KPropertyKeySharedQueueTopAddedOrRemoved, 1);
	return;
	}

CSession2* CSharedQueueSrv::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const

//
// Cretae a new client session. This should really check the version number.
//
	{
	//	__FLOG(_L("NewSessionL"));
	// Check we're the right version
	if (!User::QueryVersionSupported(
			TVersion(KServMajorVersionNumber, KServMinorVersionNumber, KServBuildVersionNumber), aVersion))
		{
		//	__FLOG(_L("Version NOT Supported"));
		User::Leave(KErrNotSupported);
		}

	return new (ELeave) CSharedQueueSession();
	}

void CSharedQueueSrv::AddSession()
	{
	iSessionCount++;
	__FLOG_1(_L("AddSession: %d"), iSessionCount);
	}

void CSharedQueueSrv::DropSession()
	{
	iSessionCount--;
	__FLOG_1(_L("DropSession: %d"), iSessionCount);
	if (iSessionCount <= 0)
		{
		CActiveScheduler::Stop();
		}
	}

CSharedQueueSrv::~CSharedQueueSrv()
	{
	__FLOG_1(_L("Destructor:%d"), iSessionCount );

	// This array has the ownership of the HBufC elements, so we must cleanup them using ResetAndDestroy();
	iParams.ResetAndDestroy();
	iParams.Close();
	iArray.Close();
	TInt ris = 0;
	ris = RProperty::Delete(KPropertyKeySharedQueueTopAddedOrRemoved); // Quando fa il delete viene segnalato il cambio ai Subscriber... Ai quali viene riportato il valore 0
	__FLOG_1(_L("PS_Delete:%d"), ris);
	ris = 0;
	__FLOG_1(_L("EndDestructor:%d"), iSessionCount );
	__FLOG_CLOSE;
	}

TInt E32Main()
	{
	return CSharedQueueSrv::EntryPoint(NULL);
	}

TInt CSharedQueueSrv::EntryPoint(TAny* /*aNone*/)
	{
	CTrapCleanup* cleanupStack = CTrapCleanup::New();
	if (!cleanupStack)
		{
		User::Panic(KSharedQueueSrvName, ESrvCreateTrapCleanup);
		}
	TRAPD(leave, ThreadMainL());
	if (leave)
		{
		User::Panic(KSharedQueueSrvName, ESrvCreateServer);
		}

	delete cleanupStack;
	cleanupStack = NULL;

	return KErrNone;
	}

void CSharedQueueSrv::ThreadMainL()
	{
	if (!Processes::RenameIfNotRunning(KSharedQueueSrvName))
		{
		// non dovrebbe mai accadere perche' il client non dovrebbe lanciarlo se c'e' gia' un server in esecuzione...
		RSemaphore semaphore;
		User::LeaveIfError(semaphore.OpenGlobal(KSharedQueueSrvImg));
		semaphore.Signal();
		semaphore.Close();
		return;
		}
	CActiveScheduler* activeScheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(activeScheduler);

	CActiveScheduler::Install(activeScheduler);

	CSharedQueueSrv::NewLC();
	//
	// Initialisation complete, now signal the client
	RSemaphore semaphore;
	User::LeaveIfError(semaphore.OpenGlobal(KSharedQueueSrvImg));

	semaphore.Signal();
	semaphore.Close();

	CActiveScheduler::Start();

	CleanupStack::PopAndDestroy(2);
	}
