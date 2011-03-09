#include "processes.h"

TBool Processes::RenameIfNotRunning(const TDesC& newName)
	{
	RMutex mutex;
	TInt result = mutex.CreateGlobal(newName);
	// This check already grants the mutual exclusion from the critical section
	// However, we call the mutex.Wait() just in case...
	if (result != KErrNone)
		{
		return EFalse;
		}
	mutex.Wait();

	// Critical Section
	TBool renamed = EFalse;
	if (!Processes::IsRunning(newName))
		{
		User::RenameProcess(newName);
		renamed = ETrue;
		}
	// Critical Section

	mutex.Signal();
	mutex.Close();
	return renamed;
	}

TBool Processes::IsRunning(const TDesC& exeName)
	{
	TFileName procName;
	procName.Copy(exeName);
	procName.Trim();
	procName.Append(_L("*"));

	TFindProcess processFinder(procName);
	TFullName fullName;
	if (processFinder.Next(fullName) == KErrNone)
		{
		return ETrue;
		}
	return EFalse;
	}

