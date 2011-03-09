/*
 * Monitor.h
 *
 *  Created on: 
 *      Author: 
 */

#ifndef MONITOR_H_
#define MONITOR_H_

#include <w32std.h>

class MMonitorObserver
{
public:
	virtual TBool KeyEventCaptured(TWsEvent aEvent)=0;	
};
 
 
 
class CEventCapturer : public CActive 
{
public:
	static CEventCapturer* NewL(MMonitorObserver& aObserver);
	static CEventCapturer* NewLC(MMonitorObserver& aObserver);
	virtual ~CEventCapturer();
	void Listen();
private:
	CEventCapturer(MMonitorObserver& aObserver);
	void ConstructL();
	void RunL();
	void DoCancel();
private:
	MMonitorObserver& 	iObserver;
	RWsSession     	iWsSession;
	RWindowGroup    iWg;
	TBool			iContinue;
};
#endif /* MONITOR_H_ */
