/*
 * EventStandby.h
 *
 *  Created on: 29/set/2010
 *      Author: Giovanna
 */

#ifndef EVENTSTANDBY_H_
#define EVENTSTANDBY_H_

#include "AbstractEvent.h"
#include <hwrmlight.h>

#include <HT\Logging.h>

typedef struct TStandbyStruct 
	{
	TUint32 iExitAction;	// action triggered when exiting standby
	}TStandbyStruct;

/**
 *  CEventStandby
 * 
 */
class CEventStandby : public CAbstractEvent, public MHWRMLightObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventStandby();

	/**
	 * Two-phased constructor.
	 */
	static CEventStandby* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventStandby* NewLC(const TDesC8& params, TUint32 aTriggerId);

protected:
	// From CAbstractEvent
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StartEventL();

private:
	// from MHWRMLightObserver
	virtual void LightStatusChanged(TInt aTarget, CHWRMLight::TLightStatus aStatus);
	/**
	 * Checks if the display is currently off
	 */
	TBool DisplayOff();

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventStandby(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
private:
	TBool iDisplayOff;
	TStandbyStruct iStandbyParams;
	CHWRMLight* iLight;
	__FLOG_DECLARATION_MEMBER
		
	};

#endif /* EVENTSTANDBY_H_ */
