/*
 ============================================================================
 Name		: AbstractQueueEndPoint.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractQueueEndPoint declaration
 ============================================================================
 */

#ifndef ABSTRACTQUEUEENDPOINT_H
#define ABSTRACTQUEUEENDPOINT_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <HT\PubSubObserver.h>
#include <HT\SharedQueue.h>
#include <HT\SharedQueueCliSrv.h>
#include <HT\logging.h>

// CLASS DECLARATION

/**
 *  CAbstractQueueEndPoint
 * 
 */
class CAbstractQueueEndPoint : public CBase, public MPubSubObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	IMPORT_C ~CAbstractQueueEndPoint();


	/**
	 * Constructor for performing 1st stage construction
	 */
	IMPORT_C CAbstractQueueEndPoint(TInt aType);

	/**
	 * This tells to the EndPoint if it can receive commands from the queue or not
	 * When it is created an EndPoint can receive commands by default.
	 * It is possible to change its status in order ignore further commands coming from the Queue
	 */
	IMPORT_C void SetReceiveCmd(TBool canReceive);

	/**
	 * Checks if this EndPoint can receive commands from the Queue
	 * @return current Active status of this EndPoint
	 */
	IMPORT_C TBool CanReceiveCmd();
	
	/**
	 * @return the Type associated to this EndPoint
	 */
	IMPORT_C TInt Type();
	
protected:
	// From MPubSubObserver
	/**
	 * Observers will receive changes of the P&S Property Key though this callback.
	 * Listen for changes of the TOP element of the Queue
	 */
	IMPORT_C virtual void PropertyChangedL(TUid category, TUint key, TInt value);

	/**
	 * Will be called when a new Command is available.
	 * The default implementation will just compare the TCommand.iDest value with this iComponent value.
	 * Subcalsses could override this method for choosing to receive or ignore Commands using different policies.
	 * 
	 * @param aCommand the new Command available
	 * @return must return true if the Command will be received by this EndPoint
	 */
	IMPORT_C virtual TBool ShouldReceiveThisCommandL(TCmdStruct aCommand);
	
	/**
	 * Will be called when the command must be executed.
	 * Subclasses MUST implement this method to process the Command.
	 * Eventually they MUST call the MarkCommandAsAccomplishedL() to notify that the Command has been processed.
	 */
	IMPORT_C virtual void DispatchCommandL(TCmdStruct aCommand) = 0;
	
	/**
	 * Signal to the Queue that the Command has been handled and processed.
	 * Subclasses MUST call this method after the Command has been processed.
	 */
	IMPORT_C void MarkCommandAsDispatchedL();
	
	/**
	 * Submit a new Command to the Queue.
	 */
	IMPORT_C void SubmitNewCommandL(TCmdStruct aCommand);
	
	/**
	 * Empty the queue.
	 */
	IMPORT_C void DoEmptyQueueL();
	
	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	IMPORT_C virtual void BaseConstructL(const TDesC8& params = KNullDesC8());

protected:	
	RBuf8 iParams;
	
private:
	RSharedQueue iQueue;
	TBool iCanReceive;
	TInt iType;
	CPubSubObserver* iPS_TopAddedOrRemoved;	
	__FLOG_DECLARATION_MEMBER
	};

#endif // ABSTRACTQUEUEENDPOINT_H
