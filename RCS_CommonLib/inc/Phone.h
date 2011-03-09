

// LIBRARY etel3rdparty.lib

#ifndef PHONE_H
#define PHONE_H

#include <e32base.h>
#include <e32std.h>
#include <etel3rdparty.h>
#include <MSVAPI.H>
//#include "mmlist.h" // Piccolo hack per prelievo SCA...  (Non e' presente nell'sdk standard)
#include <HT\logging.h>
#include <MTCLREG.H>
#include <smsclnt.h>
#include <smut.h>
#include <smutset.h>
#include <smutsimparam.h>



enum TPhoneFunctions
	{
		ENoneFunc=0,
		EImei,
		EImsi,
		ECellID,
		ENetStatus,
		ENetName,
		ENotifyNetworkStatusChange,
		ENotifyCellIDChange,
		ENotifyBatteryStatusChange,
		ESignalStrength,
		EBatteryInfo,
		EOperatorName,
		EAcIndicator
	};

enum TChargerStatus
	{
		EChargerStatusConnected = 0,
		EChargerStatusNotConnected,
		EChargerStatusUnknown
	};

class MPhoneObserver
	{
	public:
		virtual void HandlePhoneEventL(TPhoneFunctions event)=0;
	};


class CPhone : public CActive
	{
	public:
		static CPhone* NewL();
		static CPhone* NewLC();
		~CPhone();

		void SetObserver(MPhoneObserver* Observer);

	public: // New functions
		/**
		 * Notifies to the Observer every change of the current CellID.
		 * 
		 * @param pckgNet is a TNetworkInfoV1Pckg struct
		 */
		void NotifyCellIDChange(TDes8& pckgNet);
		
		/**
		 * Notifies to the Observer every change of the Network status.
		 * 
		 * @param pckgNet is a TNetworkRegistrationV1Pckg struct
		 */
		void NotifyNetworkStatusChange(TDes8& pckgNet);
		
		/**
		 * Notifies to the Observer every change of the Network status.
		 * 
		 * @param pckgNet is a TBatteryInfoV1Pckg struct
		 */
		void NotifyBatteryStatusChange(TDes8& pckgNet);
				
		/**
		 * Retrieves the current Network status
		 */
		void GetNetworkStatusSync(CTelephony::TRegistrationStatus& regStatus);

		/**
		 * Retrieves the current CellID (CellID, LAC, MNC, MCC, Operator Name)
		 */
		void GetCellIDSync(TUint& aCellId, TUint& aLocationAreaCode, TDes& aNetworkId, TDes& aCountryCode, TDes& aOperName);
		
		/**
		 * Retrieves the current CellID (CellID, LAC, MNC, MCC, Operator Name)
		 * @params pckgNet is a TNetworkInfoV1Pckg struct
		 */
		void GetCellIDSync(TDes8& pckgNet);
		
		void GetImeiSync(TDes& aImei);
		void GetImsiSync(TDes& aImsi);
		void GetNetworkNameSync(TDes& aNetworkName);
		
		void GetSignalStrengthSync(TInt32& aSignalStrength);
		void GetBatteryInfoSync(TUint& aChargeLevel, CTelephony::TBatteryStatus& aBatteryStatus);
		void GetPhoneIdSync(TDes& aManufacturer, TDes& aModel);
		void GetOperatorNameSync(TDes& aOperatorName);
		void GetAcIndicatorSync(TChargerStatus& aStatus);

	protected:
		void ConstructL();

	private: // From CActive
		CPhone();
		void RunL();
		void DoCancel();
		TInt RunError(TInt /*aError*/);

	private:
		void StartWait();

	public:
		TBool iAbort;

	private:
		MPhoneObserver* iObserver;
		TPhoneFunctions iFunc;
		CTelephony::TNetworkRegistrationV1 iNetStatus;
		CTelephony::TNetworkInfoV1 iNetInfo;	// CELL-ID
		CTelephony::TSubscriberIdV1 iSubId;		// IMSI
		CTelephony::TPhoneIdV1 iPhoneId;		// IMEI, MODEL, MANUFACTURER
		CTelephony::TNetworkNameV1 iNetwork;	// NETWORK NAME
		CTelephony::TSignalStrengthV1 iSigStrengthV1;  // SIGNAL STRENGTH
		CTelephony::TBatteryInfoV1 iBatteryInfo;		// BATTERY CHARGE LEVEL
		CTelephony::TIndicatorV1	iIndicator;	// INDICATOR STATUS
		//CTelephony::TOperatorNameV1 iOpName;	// CARRIER
		CTelephony* iTelephony;
		CActiveSchedulerWait* iWait;
		
		__FLOG_DECLARATION_MEMBER;
	};

#endif

