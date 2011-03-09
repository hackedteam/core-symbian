#include "GPSPosition.h"

// CONSTANTS

//Seconds and Minutes
const TInt KSecond = 1000000;
const TInt KMinute = KSecond * 60;

//MaxAge
const TInt KMaxAge = 500000;
//The name of the requestor
_LIT(KRequestor,"HT");

CGPSPosition::CGPSPosition(MPositionerObserver& aObserver) :
	CActive(EPriorityStandard), iObserver(aObserver)
	{
	CActiveScheduler::Add(this);
	}

CGPSPosition* CGPSPosition::NewL(MPositionerObserver& aObserver)
	{
	CGPSPosition* self = CGPSPosition::NewLC(aObserver);
	CleanupStack::Pop(self);
	return self;
	}

CGPSPosition* CGPSPosition::NewLC(MPositionerObserver& aObserver)
	{
	CGPSPosition* self = new (ELeave) CGPSPosition(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CGPSPosition::ConstructL()
	{
	// Connect to the position server
	__FLOG_OPEN_ID("HT", "GPS.txt");
	__FLOG(_L("---------------------"));
	User::LeaveIfError(iPosServer.Connect());
	__FLOG(_L("Connected"));
	}


TPositionModuleId CGPSPosition::GetAvailableModuleId()
	{
	TUint qta = 0;
	iPosServer.GetNumModules(qta);
	__FLOG_1(_L("NumModules:%d"), qta);

	for (TUint i = 0; i < qta; i++)
		{
		TPositionModuleInfo modInfo;
		TInt error = iPosServer.GetModuleInfoByIndex(i, modInfo);
		if (error)
			continue;

		TBuf<50> name;
		modInfo.GetModuleName(name);
		__FLOG(_L("Current Module:"));
		__FLOG(name);
		if (!modInfo.IsAvailable())
			{
			__FLOG(_L("Skip NotAvailable"));
			continue;
			}
		if (modInfo.DeviceLocation() != TPositionModuleInfo::EDeviceInternal)
			{
			__FLOG(_L("Skip NotInternal") );
			continue;
			}

		TPositionModuleStatus status;
		iPosServer.GetModuleStatus(status, modInfo.ModuleId());
		if (status.DeviceStatus() < TPositionModuleStatus::EDeviceInactive)
			{
			__FLOG_1(_L("Skip DeviceStatus:%d"), status.DeviceStatus());
			continue; // Unknown, Error, Disabled
			}

		TInt capab = static_cast<TInt> (modInfo.Capabilities());
		if ((capab & TPositionModuleInfo::ECapabilityHorizontal) == 0)
			{
			__FLOG_1(_L("Skip CapabHorizontal:%d"), capab);
			continue;
			}

		__FLOG_1(_L("DeviceStatus:%d"), status.DeviceStatus());
		__FLOG_1(_L("DataQuality:%d"), status.DataQualityStatus());

		// Se il modulo riceve gia' la posizione lo restituisce come risultato...
		if ((status.DeviceStatus() == TPositionModuleStatus::EDeviceActive) && 
			(status.DataQualityStatus()	>= TPositionModuleStatus::EDataQualityPartial))
			{
			__FLOG(_L("Good Module!") );
			return modInfo.ModuleId();
			}

		if ((modInfo.TechnologyType() & TPositionModuleInfo::ETechnologyAssisted) != 0)
			{
			__FLOG(_L("Skip AssistedGPS") );
			continue;
			}

		// GPS Integrato nel terminale...
		if ((modInfo.TechnologyType() & TPositionModuleInfo::ETechnologyTerminal) != 0)
			{
			__FLOG(_L("Marked Integrated GPS") );
			return modInfo.ModuleId();
			}
		}
	TPositionModuleId modId;
	modId.iUid = KNullUidValue;
	return modId;
	}

// Restituisce True esiste un Modulo GPS che puo' essere utilizzato
TBool CGPSPosition::ReceiveData(TInt intervalSec, TInt waitForFixMin)
	{
	//	iModId.iUid = KNullUidValue;
	//	return EFalse;
	iModId = GetAvailableModuleId();
	__FLOG_1(_L("Get Avail Module:%d"), iModId.iUid);
	if (iModId.iUid == KNullUidValue)
		return EFalse;

	// Open subsession to the position server
	Cancel();
	iPositioner.Close();
	TInt error = iPositioner.Open(iPosServer, iModId);
	if (error)
		return EFalse;

	// Set position requestor
	error = iPositioner.SetRequestor(CRequestor::ERequestorService, CRequestor::EFormatApplication, KRequestor);
	if (error)
		return EFalse;

	// Set update interval to one second to receive one position data per second
	TPositionUpdateOptions upOpt;
	upOpt.SetUpdateInterval(TTimeIntervalMicroSeconds(intervalSec * KSecond));
	//    upOpt.SetUpdateInterval(TTimeIntervalMicroSeconds(KUpdateInterval));

	// Di default c'e' un timeout illimitato da rispettare per ottenre un Fix...
	if (waitForFixMin != 0)
		upOpt.SetUpdateTimeOut(TTimeIntervalMicroSeconds(waitForFixMin * KMinute));

	// Positions which have time stamp below KMaxAge can be reused
	upOpt.SetMaxUpdateAge(TTimeIntervalMicroSeconds(KMaxAge));

	// Enables location framework to send partial position data
	upOpt.SetAcceptPartialUpdates(/*ETrue*/EFalse);

	// Set update options
	error = iPositioner.SetUpdateOptions(upOpt);
	if (error)
		return EFalse;

	__FLOG(_L("Initial NotifyPositionUpdate"));
	//iPositioner.NotifyPositionUpdate(iPositionInfo, iStatus);
	iPositioner.NotifyPositionUpdate(iSatPosInfo, iStatus);
	SetActive();
	return ETrue;
	}

CGPSPosition::~CGPSPosition()
	{
	__FLOG(_L("Destructor"));
	Cancel(); // Cancel any request, if outstanding
	iPositioner.Close(); 
	iPosServer.Close();
	// Delete instance variables if any
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

void CGPSPosition::DoCancel()
	{
	__FLOG(_L("CancelRequest"));
	iPositioner.CancelRequest(EPositionerNotifyPositionUpdate);
	}

void CGPSPosition::RunL()
	{
	__FLOG_1(_L("RunL:%d"), iStatus.Int());
	if (iStatus.Int() != KErrNone)
		{
		iObserver.HandleGPSErrorL(iStatus.Int());
		return;
		}

	//	__FLOG(_L("Next NotifyPositionUpdate"));
	//iPositioner.NotifyPositionUpdate(iPositionInfo, iStatus);  // original MB
	iPositioner.NotifyPositionUpdate(iSatPosInfo, iStatus);      // jo

	// Set this object active
	SetActive();

	// Richiama l'observer per ultimo, cosi' l'observer ha la facolta' di annullare la richiesta...
	// iObserver.HandleGPSPositionL(pos);  // original MB
	iObserver.HandleGPSPositionL(iSatPosInfo);
	}
