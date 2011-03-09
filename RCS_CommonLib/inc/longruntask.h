
// Splitting long running tasks with active objects
// http://wiki.forum.nokia.com/index.php/Splitting_long_running_tasks_with_active_objects

#ifndef LONGTASKAO_H
#define LONGTASKAO_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		


class MLongTaskCallBack 
{
public:
	virtual void DoOneRoundL() = 0;
};


class CLongTaskAO : public CActive
{
public:
	// C++ constructor
	static CLongTaskAO* NewL(MLongTaskCallBack& callBack);
	
	// Cancel and destroy
	~CLongTaskAO();

	// Function for making the initial request
	void NextRound();

private: // From CActive	
	CLongTaskAO(MLongTaskCallBack& callBack);
	
	// Second-phase constructor
	void ConstructL();
	
	// Handle completion
	void RunL();
	
	// How to cancel me
	void DoCancel();
	
	// Override to handle leaves from RunL(). Default implementation causes
	// the active scheduler to panic.
	//void RunError(TInt aError);

private:
	MLongTaskCallBack& iCallBack;
};

#endif
