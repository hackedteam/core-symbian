/*
 * MessageFilter.h
 *
 *  Created on: 30/giu/2010
 *      Author: 
 */

// TODO: complete and extend to use keywords in runtime filters

#ifndef MESSAGEFILTER_H_
#define MESSAGEFILTER_H_

// INCLUDES
#include <e32base.h>

// CLASS DECLARATION

/**
 *  CMessageFilter
 * 
 */
class CMessageFilter : public CBase
	{
	
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CMessageFilter();

	/**
	 * Two-phased constructor.
	 */
	static CMessageFilter* NewL();

	/**
	 * Two-phased constructor.
	 */
	static CMessageFilter* NewLC();

	/**
	 * Methods for filter management
	 */
	void SetStartDate(TTime aTime);
	TTime StartDate();
	void SetEndDate(TTime aTime);
	TTime EndDate();
	void ModifyFilterRange(TTime aMarkup);
	TBool MessageInRange(const TTime& aTime);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CMessageFilter();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

private:
	TTime iStartDate;
	TTime iEndDate;
	
public:
	TBool 	iLog;
	TBool 	iSinceFilter;
	TBool 	iUntilFilter;
		
	// meaningfull only for email filter
	TInt32 	iMaxMessageSize;               	// 0 = accept all messages
	TInt32 	iMaxMessageBytesToLog;			// 0 = takes all message
	
	};


#endif /* MESSAGEFILTER_H_ */
