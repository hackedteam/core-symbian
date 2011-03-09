// CPubSubObserver.h

#ifndef __CPubSubObserver_h__
#define __CPubSubObserver_h__

#include <e32base.h>
#include <e32property.h>

class MPubSubObserver
	{
public:
	/**
	 * Observers will receive changes of the P&S Property Key though this callback.
	 */
	virtual void PropertyChangedL(TUid category, TUint key, TInt value) = 0;
	};

/**
 * This class will Observe a particular P&S Property Key and will notify to the Observers
 * the changes of the P&S Property Key
 */
class CPubSubObserver : public CActive
	{
public:
	static CPubSubObserver* NewL(MPubSubObserver& obs, TUid category, TUint key);
	~CPubSubObserver();
	void StartMonitorProperty();

protected:
	void RunL();
	void DoCancel();

private:
	CPubSubObserver(MPubSubObserver& obs, TUid category, TUint key);
	void ConstructL();

private:
	MPubSubObserver& iObserver;
	TUid iCategory;
	TUint iKey;
	RProperty iProperty;
	};

#endif
