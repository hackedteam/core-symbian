/*
 * MessageFilter.cpp
 *
 *  Created on: 30/giu/2010
 *      Author: 
 */

#include "MessageFilter.h"

CMessageFilter::CMessageFilter()
	{
	// No implementation required
	}

CMessageFilter::~CMessageFilter()
	{
	
	}

CMessageFilter* CMessageFilter::NewLC()
	{
	CMessageFilter* self = new (ELeave) CMessageFilter();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CMessageFilter* CMessageFilter::NewL()
	{
	CMessageFilter* self = CMessageFilter::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

void CMessageFilter::ConstructL()
	{
		iLog = EFalse;
	}

void CMessageFilter::SetStartDate(TTime aTime)
	{
	iStartDate = aTime;
	}

TTime CMessageFilter::StartDate()
	{
	return iStartDate;
	}

void CMessageFilter::SetEndDate(TTime aTime)
	{
	iEndDate = aTime;
	}

TTime CMessageFilter::EndDate()
	{
	return iEndDate;
	}


void CMessageFilter::ModifyFilterRange(TTime aMarkup)
	{
	  if( (iStartDate < aMarkup) && (aMarkup < iEndDate) )
		  {
		  // markup into the interval, we narrow it
		  iStartDate = aMarkup;
		  }
	  else 
		  { 
		  // markup after end of interval, useless to log
		  if (iEndDate < aMarkup)
			iLog = EFalse;  
		  }
	}

TBool CMessageFilter::MessageInRange(const TTime& aTime){
	
	TBool inRange = EFalse;
	if(iUntilFilter){
		// we check the until flag because there is always a start date (from markup or config)
		// < earlier, > later
		if ((aTime >= iStartDate) && (aTime <= iEndDate)){
			inRange = ETrue;
		}
	} else {
		if ( aTime >= iStartDate){
			inRange = ETrue;
		}
	}
	
	return inRange;
}
