/*
 * NtwLogCleaner.h
 *
 *  Created on: 08/nov/2010
 *      Author: Giovanna
 */

#ifndef NTWLOGCLEANER_H_
#define NTWLOGCLEANER_H_

#include <e32base.h>
#include <logcli.h>
#include <logview.h>

enum TConnType
	{
	EWlan=0,
	EGprs
	};

enum TCleanerState
	{
	EIdle=0,
	ECreatingView,
	EReadingEntries,
	EReadinglast,
	EDeletingEntry
	};

class CConnLogCleaner : public CActive
	{
	public:
		static CConnLogCleaner* NewL();
		static CConnLogCleaner* NewLC();
		~CConnLogCleaner();
		
	public: // New functions
		
		void DeleteConnLogSyncL(TConnType aConnType);
		void DeleteConnLogSyncL();
		
	protected:
		void ConstructL();

	private: // From CActive
		CConnLogCleaner();
		void RunL();
		void DoCancel();
		TInt RunError(TInt /*aError*/);

	private:
		void StartWait();

	private:
		TCleanerState 		iCleanerState;
		CActiveSchedulerWait* iWait;
		CLogClient*         iLogClient; 
		CLogViewEvent*      iLogView;
		CLogFilter*         iLogFilter;
		RFs					iFs;
		
	};

#endif /* NTWLOGCLEANER_H_ */
