/*
 ============================================================================
 Name        : CGPSPosition.h
 Author      : MarCo
 Version     :
 Copyright   : Just Try It
 Description : CCGPSPosition declaration
 ============================================================================
 */

#include <HT\logging.h>

#ifndef GPSPosition_H
#define GPSPosition_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		// For RTimer, link against: euser.lib
#include <lbs.h>
#include <lbssatellite.h>    // jo, for TPositionSatelliteInfo

class MPositionerObserver
	{
public:
	//virtual void HandleGPSPositionL(TPosition position)=0;     // original MB
	virtual void HandleGPSPositionL(TPositionSatelliteInfo position)=0;   // jo
	virtual void HandleGPSErrorL(TInt error)=0;
	};

class CGPSPosition : public CActive
	{
public:
	CGPSPosition(MPositionerObserver& aObserver);
	~CGPSPosition();
	static CGPSPosition* NewL(MPositionerObserver& aObserver);
	static CGPSPosition* NewLC(MPositionerObserver& aObserver);

	// Restituisce True esiste un Modulo GPS che puo' essere utilizzato
	TBool ReceiveData(TInt intervalSec, TInt waitForFixMin=0);

private:
	// From CActive
	void ConstructL();

	TPositionModuleId GetAvailableModuleId();

	// Handle completion
	void RunL();

	// How to cancel me
	void DoCancel();

	// Override to handle leaves from RunL(). Default implementation causes
	// the active scheduler to panic.
	//void RunError(TInt aError);

private:
	enum TCGPSPositionState
		{
		EUninitialized, // Uninitialized
		EInitialized,   // Initalized
		EError		    // Error condition
		};

public:
	TPositionModuleId iModId;

private:
	__FLOG_DECLARATION_MEMBER
	RPositionServer iPosServer;
	RPositioner iPositioner;
	//TPositionInfo iPositionInfo;     // original MB
	TPositionSatelliteInfo iSatPosInfo;    // jo

	MPositionerObserver& iObserver;
	};

#endif
