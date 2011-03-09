#include "Phone.h"
#include <MTCLREG.H> 
#include <smsclnt.h>
#include <smut.h>
#include <smutset.h>
#include <smutsimparam.h>
#include <e32math.h>

CPhone::CPhone() :
	CActive(EPriorityStandard)
	{
	iFunc = ENoneFunc;
	}

CPhone* CPhone::NewL()
	{
	CPhone* self = CPhone::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

CPhone* CPhone::NewLC()
	{
	CPhone* self = new (ELeave) CPhone();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CPhone::ConstructL()
	{
	CActiveScheduler::Add(this);
	__FLOG_OPEN_ID("HT", "Phone.txt");
	__FLOG(_L("---------"));

	// Nell'emulatore UIQ CTelephony non e' supportata...
#ifndef __WINSCW__
	iTelephony = CTelephony::NewL();
#endif

	iWait = new (ELeave) CActiveSchedulerWait();
	iAbort = EFalse;
	}

void CPhone::SetObserver(MPhoneObserver* Observer)
	{
	iObserver = Observer;
	}

CPhone::~CPhone()
	{
	__FLOG(_L("Destructor"));
	Cancel();
	delete iTelephony;
	delete iWait;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

void CPhone::DoCancel()
	{
	__FLOG_1(_L("DoCancel: %d"), iFunc);
	if (iWait->IsStarted()) // ***
		iWait->AsyncStop(); // ***

	iAbort = ETrue;
	//__FLOG_1(_L("DoCancel:%d"), iFunc);
	switch (iFunc)
		{
		case EImei:
			iTelephony->CancelAsync(CTelephony::EGetPhoneIdCancel);
			break;
		case EImsi:
			iTelephony->CancelAsync(CTelephony::EGetSubscriberIdCancel);
			break;
		case ENetStatus:
			iTelephony->CancelAsync(CTelephony::EGetNetworkRegistrationStatusCancel);
			break;
		case ECellID:
			iTelephony->CancelAsync(CTelephony::EGetCurrentNetworkInfoCancel);
			break;
		case ENotifyCellIDChange:
			iTelephony->CancelAsync(CTelephony::ECurrentNetworkInfoChangeCancel);
			break;
		case ENotifyNetworkStatusChange:
			iTelephony->CancelAsync(CTelephony::ENetworkRegistrationStatusChangeCancel);
			break;
		case ENotifyBatteryStatusChange:
			iTelephony->CancelAsync(CTelephony::EBatteryInfoChangeCancel);
			break;
		case ENetName:
			iTelephony->CancelAsync(CTelephony::EGetCurrentNetworkNameCancel);
			break;
		case ESignalStrength:
			iTelephony->CancelAsync(CTelephony::EGetSignalStrengthCancel);
			break;
		case EBatteryInfo:
			iTelephony->CancelAsync(CTelephony::EGetBatteryInfoCancel);
			break;
		case EOperatorName:
			iTelephony->CancelAsync(CTelephony::EGetOperatorNameCancel);
			break;
		case EAcIndicator:
			iTelephony->CancelAsync(CTelephony::EGetIndicatorCancel);
			break;
		default:
			break;
		}
	iFunc = ENoneFunc;
	}

TInt CPhone::RunError(TInt /*aError*/)
	{
	return KErrNone;
	}

void CPhone::GetImeiSync(TDes& aImei)
	{
	__FLOG(_L("GetImeiSync"));
	if (IsActive())
		{
		Cancel();
		}

#ifdef __WINSCW__
	aImei = _L("356962014461702");
	return;
#endif

	CTelephony::TPhoneIdV1Pckg phoneIdPckg(iPhoneId);
	iTelephony->GetPhoneId(iStatus, phoneIdPckg);

	iFunc = EImei;
	SetActive();
	StartWait();
	iFunc = ENoneFunc;

	iPhoneId.iSerialNumber.Trim();
	aImei = iPhoneId.iSerialNumber.Left(15);
	}

void CPhone::GetImsiSync(TDes& aImsi)
	{
	__FLOG(_L("GetImsiSync"));
	if (IsActive())
		{
		Cancel();
		}

#ifdef __WINSCW__
	aImsi = _L("123456789012345");
	return;
#endif
	CTelephony::TSubscriberIdV1Pckg subIdPckg(iSubId);
	iTelephony->GetSubscriberId(iStatus, subIdPckg);

	iFunc = EImsi;
	SetActive();
	StartWait();
	iFunc = ENoneFunc;

	aImsi = iSubId.iSubscriberId;
	}

void CPhone::GetNetworkNameSync(TDes& aNetworkName)
	{
	__FLOG(_L("GetNetworkNameSync"));
	if (IsActive())
		{
		Cancel();
		}

#ifdef __WINSCW__
	aNetworkName = _L("FakeNet");
	return;
#endif
	CTelephony::TNetworkNameV1Pckg netPckg(iNetwork);
	iTelephony->GetCurrentNetworkName(iStatus, netPckg);

	iFunc = ENetName;
	SetActive();
	StartWait();
	iFunc = ENoneFunc;

	aNetworkName = iNetwork.iNetworkName;
	__FLOG(_L("NetworkName:"));
	__FLOG(aNetworkName);
	}

void CPhone::GetNetworkStatusSync(CTelephony::TRegistrationStatus& regStatus)
	{
	__FLOG(_L("GetNetworkStatusSync"));
	if (IsActive())
		{
		Cancel();
		}

#ifdef __WINSCW__
	regStatus = CTelephony::ERegisteredOnHomeNetwork;
	return;
#endif

	CTelephony::TNetworkRegistrationV1Pckg netStatPckg(iNetStatus);
	iNetStatus.iRegStatus = CTelephony::ERegistrationUnknown;
	iTelephony->GetNetworkRegistrationStatus(iStatus, netStatPckg);

	iFunc = ENetStatus;
	SetActive();
	StartWait();
	iFunc = ENoneFunc;

	regStatus = iNetStatus.iRegStatus;
	}


void CPhone::NotifyCellIDChange(TDes8& pckgNet)
	{
	__FLOG(_L("NotifyNetworkInfoChange"));
	if (IsActive())
		{
		Cancel();
		}

#ifdef __WINSCW__
	return;
#endif
	iTelephony->NotifyChange(iStatus, CTelephony::ECurrentNetworkInfoChange, pckgNet);
	iFunc = ENotifyCellIDChange;
	SetActive();	
	}

void CPhone::NotifyNetworkStatusChange(TDes8& pckgNet)
	{
	__FLOG(_L("NotifyNetworkStatusChange"));
	if (IsActive())
		{
		Cancel();
		}

#ifdef __WINSCW__
	return;
#endif
	iTelephony->NotifyChange(iStatus, CTelephony::ENetworkRegistrationStatusChange, pckgNet);
	iFunc = ENotifyNetworkStatusChange;
	SetActive();
	}

//	iPhone.NotifyNetworkRegistrationStatusChange(iStatus, iRegStatus);

void CPhone::NotifyBatteryStatusChange(TDes8& pckgBattery)
	{
	__FLOG(_L("NotifyBatteryStatusChange"));
	if (IsActive())
		{
		Cancel();
		}

#ifdef __WINSCW__
	return;
#endif
	iTelephony->NotifyChange(iStatus, CTelephony::EBatteryInfoChange, pckgBattery);
	iFunc = ENotifyBatteryStatusChange;
	SetActive();
	}

void CPhone::GetCellIDSync(TDes8& pckgNet)
	{
	__FLOG(_L("GetCellIDSync"));
#ifdef __WINSCW__
	return;
#endif 
	iTelephony->GetCurrentNetworkInfo(iStatus, pckgNet);
	iFunc = ECellID;
	SetActive();
	StartWait();
	iFunc = ENoneFunc;
	}


void CPhone::GetCellIDSync(TUint& aCellId, TUint& aLocationAreaCode, TDes& aNetworkId, TDes& aCountryCode,
		TDes& aOperName)
	{
	__FLOG(_L("GetCellID"));
	if (IsActive())
		{
		Cancel();
		}

#ifdef __WINSCW__
	aCellId = 1000;
	aLocationAreaCode = 2000;
	aCountryCode = _L("11");
	aNetworkId = _L("22");
	aOperName = _L("name");
	return;
#endif
	CTelephony::TNetworkInfoV1Pckg netInfoPckg(iNetInfo);
	GetCellIDSync(netInfoPckg);
	
	//	if (iNetInfo.iAreaKnown) 
	//	{
	aCellId = iNetInfo.iCellId;    
	aCellId = aCellId & 0xFFFF;	// jo
	aLocationAreaCode = iNetInfo.iLocationAreaCode;
	aCountryCode = iNetInfo.iCountryCode;
	aOperName = iNetInfo.iLongName;
	aNetworkId = iNetInfo.iNetworkId;
	//	}
	__FLOG(_L("CountryCode:"));
	__FLOG(aCountryCode);
	__FLOG(_L("NetId:"));
	__FLOG(aNetworkId);
	__FLOG(_L("OperLongName:"));
	__FLOG(iNetInfo.iLongName);
	__FLOG(_L("OperShortName:"));
	__FLOG(iNetInfo.iShortName);
	}

void CPhone::GetSignalStrengthSync(TInt32& aSignalStrength)
	{
	if (IsActive())
		{
		Cancel();
		}

	CTelephony::TSignalStrengthV1Pckg sigStrengthPckg(iSigStrengthV1);
	iTelephony->GetSignalStrength(iStatus,sigStrengthPckg);
	iFunc = ESignalStrength; 
	SetActive();
	StartWait();
	iFunc = ENoneFunc;
	aSignalStrength = iSigStrengthV1.iSignalStrength;
	}


void CPhone::GetBatteryInfoSync(TUint& aChargeLevel, CTelephony::TBatteryStatus& aBatteryStatus)
	{
	if (IsActive())
		{
		Cancel();
		}

	CTelephony::TBatteryInfoV1Pckg batteryInfoPckg(iBatteryInfo);
	iTelephony->GetBatteryInfo(iStatus,batteryInfoPckg);
	iFunc = EBatteryInfo; 
	SetActive();
	StartWait();
	iFunc = ENoneFunc;
	aChargeLevel = iBatteryInfo.iChargeLevel;
	aBatteryStatus = iBatteryInfo.iStatus;
	}

void CPhone::GetPhoneIdSync(TDes& aManufacturer, TDes& aModel)
	{
	if (IsActive())
		{
		Cancel();
		}

	CTelephony::TPhoneIdV1Pckg phoneIdPckg(iPhoneId);
	iTelephony->GetPhoneId(iStatus,phoneIdPckg);
	iFunc = EImei; 
	SetActive();
	StartWait();
	iFunc = ENoneFunc;
	aManufacturer.Copy(iPhoneId.iManufacturer);
	aModel.Copy(iPhoneId.iModel);
	}

void CPhone::GetOperatorNameSync(TDes& aOperatorName)
	{
	if (IsActive())
		{
		Cancel();
		}
	/*
	CTelephony::TOperatorNameV1Pckg operatorNamePckg(iOpName);
	iTelephony->GetOperatorName(iStatus,operatorNamePckg);
	iFunc = EOperatorName; 
	SetActive();
	StartWait();
	iFunc = ENoneFunc;
	aOperatorName.Copy(iOpName.iOperatorName);
	*/
	CTelephony::TNetworkInfoV1Pckg netInfoPckg(iNetInfo);
	iTelephony->GetCurrentNetworkInfo(iStatus,netInfoPckg);
	iFunc = ECellID;
	SetActive();
	StartWait();
	iFunc = ENoneFunc;
	aOperatorName.Copy(iNetInfo.iShortName);
	if (aOperatorName.Length() == 0)
		{
			aOperatorName.Copy(iNetInfo.iLongName);
		}
		
	}

void CPhone::GetAcIndicatorSync(TChargerStatus& aStatus)
	{
	if (IsActive())
		{
		Cancel();
		}
	CTelephony::TIndicatorV1Pckg indicatorPckg(iIndicator);
	iTelephony->GetIndicator(iStatus,indicatorPckg);
	iFunc = EAcIndicator;
	SetActive();
	StartWait();
	iFunc = ENoneFunc;
	if(iIndicator.iCapabilities & CTelephony::KIndChargerConnected)
		{
	    //We can detect when a charger is connected
	    if(iIndicator.iIndicator & CTelephony::KIndChargerConnected)
	    	{
	        //Charger is connected
	        aStatus = EChargerStatusConnected;  
	        }
		else
	        {
	        //Charger is not connected
			aStatus = EChargerStatusNotConnected;
	        }
	    }
	    else
	    {
	    //We do not know whether or not a charger is connected
	    aStatus = EChargerStatusUnknown;
	    }
	}

void CPhone::RunL()
	{
	__FLOG(_L("RunL"));
	if (iWait->IsStarted())
		iWait->AsyncStop();

	TPhoneFunctions completed = iFunc;
	// NOTA: Impostando iFunc su None e' possibile richiamare Cancel senza problemi...
	// Inoltre, bisogna impostarlo prima di chiamare la CallBack altrim.
	// Una successiva chiamata Asincrona fatta dalla CallBack non servita' a nulla
	// perche' subito dopo viene impostato iFunc a None.
	iFunc = ENoneFunc;

	if (iStatus.Int() != KErrNone) 
		{
		__FLOG_2(_L("RunL Error:%d Func:%d"), iStatus.Int(), completed);
		}

	switch (completed)
		{
		case EImei:
			{
			break;
			}
		case EImsi:
			{
			break;
			}
		case EBatteryInfo:
			break;
		case ENotifyCellIDChange:
		case ENotifyNetworkStatusChange:
		case ENotifyBatteryStatusChange:
			{
			if (iObserver)
				iObserver->HandlePhoneEventL(completed);
			// CallBack
			break;
			}
		default:
			break;
		}
	}

void CPhone::StartWait()
	{
	iAbort = EFalse;
	if (iWait->IsStarted() != (TInt) ETrue)
		{
		//__FLOG(_L("StartWait"));
		iWait->Start();
		}
	}

