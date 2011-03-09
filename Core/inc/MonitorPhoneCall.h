/*
 * PhoneCallMonitor.h
 *
 *  Created on: 06/ott/2010
 *      Author: Giovanna
 */

#ifndef PHONECALLMONITOR_H_
#define PHONECALLMONITOR_H_

#include <e32base.h>
#include <Etel3rdParty.h> 


class MCallMonCallBack
{
public:	
	virtual void NotifyConnectedCallStatusL(const TDesC& aNumber)=0; //aNumber.Length() == 0 when private number calling
	virtual void NotifyDisconnectedCallStatusL()=0;
};

class CPhoneCallMonitor : public CActive
  {

public: // public constructors & destructor
  	
	static CPhoneCallMonitor* NewLC(MCallMonCallBack &aCallBack);
  	static CPhoneCallMonitor* NewL(MCallMonCallBack &aCallBack);
  	~CPhoneCallMonitor();
  	void StartListeningForEvents();
  	/*
  	 * ActiveCall check if there's an active/connected call; only if ETrue, aNumber is meaningfull;; and only 
  	 * in that case, if aNumber.Length() == 0, then aNumber is a private number. 
  	 */
  	TBool ActiveCall(TDes& aNumber);
  	//CTelephony::TCallStatus	PreviousStatus(void){return iPreviousStatus;};

protected:
  	// from CActive
  	void DoCancel();
  	void RunL();
  	TInt RunError(TInt /*aError*/);

private: 
  	// private constructors
  	void ConstructL();
  	CPhoneCallMonitor(MCallMonCallBack &aCallBack);
	// private internal functions
  	
private:
  	
	MCallMonCallBack&	iCallBack;
  	CTelephony* 		iTelephony;
  	//CTelephony::TCallStatus			iPreviousStatus;
  	CTelephony::TCallStatusV1 		iCallStatus;
 	CTelephony::TCallStatusV1Pckg 	iCallStatusPckg;
};

#endif /* PHONECALLMONITOR_H_ */
