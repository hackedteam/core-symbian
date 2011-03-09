/*
 * EventAc.h
 *
 *  Created on: 26/set/2010
 *      Author: Giovanna
 */

#ifndef EVENTAC_H_
#define EVENTAC_H_

#include "AbstractEvent.h"
#include <HT\Phone.h>
#include <HT\Logging.h>


typedef struct TAcStruct 
	{
	TUint32 iExitAction;
	}TAcStruct;

/**
 *  CEventAc
 * 
 */
class CEventAc : public CAbstractEvent, public MPhoneObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventAc();

	/**
	 * Two-phased constructor.
	 */
	static CEventAc* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventAc* NewLC(const TDesC8& params, TUint32 aTriggerId);

protected:
	// From CAbstractEvent
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StartEventL();

private:
	// From MPhoneObserver
	virtual void HandlePhoneEventL(TPhoneFunctions event);

	/**
	 * Checks if the device is currently Connected to charger
	 */
	TBool ConnectedToCharger();

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventAc(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
private:
	TBool iWasConnectedToCharger;
	CPhone* iPhone;
	TAcStruct iAcParams;
	CTelephony::TBatteryInfoV1Pckg iBatteryInfoPckg;
	CTelephony::TBatteryInfoV1 iBatteryInfo; 			// Battery Info
	__FLOG_DECLARATION_MEMBER
		
	};

#endif /* EVENTAC_H_ */
