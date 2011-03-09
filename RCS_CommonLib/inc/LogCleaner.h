


#ifndef __LOGCLEANER_H__
#define __LOGCLEANER_H__

#include <e32base.h>
#include <HT\logging.h>
#include <logeng.h>
#include <logcli.h>
#include <logview.h>
#include <logwrap.h>
//#include <logviewchangeobserver.h>


class CLogCleaner : public CActive
	{

	enum TLogStatus
		{
		EHandle_NotifyChange=0,
		EHandle_SetFilter,
		EHandle_DeleteEvent
		};

	public:

		IMPORT_C static CLogCleaner* NewL(RFs& aFS);
		IMPORT_C virtual ~CLogCleaner();			
		IMPORT_C void StartCleaner(const TDesC& aNumber);

	private:
		CLogCleaner();
		/** Inizializza il telefono da richiamare prima di tutto */
		void ConstructL(RFs& aFS);

		void NotifyChange();
		void DeleteMostRecentEventL();

		// from CActive
		TInt RunError(TInt aError);
		void RunL();
		void DoCancel();

	private:
		CLogViewEvent* iLogView;	// own
		CLogFilter* iFilter;	// own
		CLogClient* iLogClient;	// own
		TLogStatus iStato;
		__FLOG_DECLARATION_MEMBER
	};


#endif



