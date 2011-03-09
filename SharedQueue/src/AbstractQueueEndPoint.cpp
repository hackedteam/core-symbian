/*
 ============================================================================
 Name		: AbstractQueueEndPoint.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractQueueEndPoint implementation
 ============================================================================
 */

#include "AbstractQueueEndPoint.h"

EXPORT_C CAbstractQueueEndPoint::CAbstractQueueEndPoint(TInt aType) :
	iType(aType)
	{
	// No implementation required
	}

EXPORT_C CAbstractQueueEndPoint::~CAbstractQueueEndPoint()
	{
	__FLOG(_L("Destructor"));
	delete iPS_TopAddedOrRemoved;
	iParams.Close();
	iQueue.Close();
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	} 

EXPORT_C TInt CAbstractQueueEndPoint::CAbstractQueueEndPoint::Type()
	{
	return iType;
	}

EXPORT_C void CAbstractQueueEndPoint::SetReceiveCmd(TBool canReceive)
	{
	if (canReceive == iCanReceive)
		return;
	iCanReceive = canReceive;
	if (canReceive)
		{
		__FLOG(_L("SetReceiveCmd: True"));
		iPS_TopAddedOrRemoved->StartMonitorProperty();
		}
	else
		{
		__FLOG(_L("SetReceiveCmd: False"));
		iPS_TopAddedOrRemoved->Cancel();
		}
	__FLOG(_L("SetReceiveCmd: End"));
	}

EXPORT_C TBool CAbstractQueueEndPoint::CanReceiveCmd()
	{
	return iCanReceive;
	}

EXPORT_C void CAbstractQueueEndPoint::BaseConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "QueueEndPoint.txt");
	__FLOG(_L("-------------"));
	__FLOG_1(_L("Type: %x"), iType);
		
	iParams.Create(params);
	User::LeaveIfError(iQueue.Connect());
	iPS_TopAddedOrRemoved = CPubSubObserver::NewL(*this, KPropertyUidSharedQueue,
				KPropertyKeySharedQueueTopAddedOrRemoved);
	SetReceiveCmd(ETrue);
	} 

EXPORT_C void CAbstractQueueEndPoint::PropertyChangedL(TUid category, TUint key, TInt value)
	{
	
		ASSERT_PANIC( category == KSharedQueueSrvUid3, 11 );
		ASSERT_PANIC( key == KPropertyKeySharedQueueTopAddedOrRemoved, 12 );

		if (iQueue.IsEmpty())
			{
			return;
			} 
		
		// A new Command is available on the Queue
		TCmdStruct command = iQueue.Top();

		if (ShouldReceiveThisCommandL(command) && iQueue.LockTop())
			{
			__FLOG_3(_L("Dispatch Src: %x  Dest: %x  Type: %x"), command.iSrc, command.iDest, command.iType);
			DispatchCommandL(command);
			}
	
	}

EXPORT_C TBool CAbstractQueueEndPoint::ShouldReceiveThisCommandL(TCmdStruct aCommand)
	{
	return (aCommand.iDest == iType);
	}

EXPORT_C void CAbstractQueueEndPoint::MarkCommandAsDispatchedL()
	{
	__FLOG(_L("MarkCommandAsDispatchedL Start"));
	TCmdStruct removed = iQueue.Dequeue();
	__FLOG_3(_L("Removed Src: %x  Dest: %x  Type: %x"), removed.iSrc, removed.iDest, removed.iType);
	if (removed.iDest != iType)
		{
		User::Panic(_L("Errors in Queue-2"), KErrGeneral);
		}
	__FLOG(_L("MarkCommandAsDispatchedL End"));
			
	}

EXPORT_C void CAbstractQueueEndPoint::SubmitNewCommandL(TCmdStruct aCommand)
	{
	__FLOG(_L("SubmitNewCommandL Start"));
	iQueue.Enqueue(aCommand);
	__FLOG(_L("SubmitNewCommandL End"));
	}

EXPORT_C void CAbstractQueueEndPoint::DoEmptyQueueL()
	{
	__FLOG(_L("EmptyQueueL"));
	iQueue.DoEmpty();
	}

