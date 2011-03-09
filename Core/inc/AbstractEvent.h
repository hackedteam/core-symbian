/*
 ============================================================================
 Name		: AbstractEvent.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractEvent declaration
 ============================================================================
 */

#ifndef ABSTRACTEVENT_H
#define ABSTRACTEVENT_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <HT\AbstractQueueEndPoint.h>
// CLASS DECLARATION

/**
 *  CAbstractEvent: 
 */
class CAbstractEvent : public CAbstractQueueEndPoint
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	IMPORT_C ~CAbstractEvent();

	/**
	 * Events MUST implement this method to start their task.
	 */
	IMPORT_C virtual void StartEventL()=0;

protected:
	// From CAbstractQueueEndPoint
	IMPORT_C virtual void BaseConstructL(const TDesC8& params);

	/**
	 * Constructor for performing 1st stage construction
	 */
	IMPORT_C CAbstractEvent(TEventType aType, TUint32 aTriggerId);

	/**
	 * Notify to the CORE that this Event has happened.
	 * Sends a notification through the Queue.
	 * Events should call this method to trigger the Actions.
	 */
	IMPORT_C void SendActionTriggerToCoreL();
	
	/**
	 * Notify to the CORE that this Event has happened.
	 * Sends a notification through the Queue.
	 * Events should call this method to trigger the Actions.
	 * 
	 * An Event can trigger many Actions.
	 */
	IMPORT_C void SendActionTriggerToCoreL(TUint32 aTriggerId);

private:
	// From CAbstractQueueEndPoint
	void DispatchCommandL(TCmdStruct /*aCommand*/);

private:
	// When this Event will happen it will send a notifcation to the CORE using the Trigger Id as source
	TUint32 iTriggerId;
	};

#endif // ABSTRACTEVENT_H
