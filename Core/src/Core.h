
#ifndef __CORE_H__
#define __CORE_H__

//  Include Files
#include <e32base.h>
#include <HT\Logging.h>			// Logging
#include <HT\SharedQueue.h>		// RSharedQueue
#include <HT\PubSubObserver.h>	// CPubSubObserver
#include <HT\AbstractQueueEndPoint.h>
#include "ConfigFile.h"

class CCore : public CAbstractQueueEndPoint
	{
public:
	static CCore* NewLC();
	virtual ~CCore();

	void LoadConfigAndStartL();
	/**
	 * Create a LOGTYPE_INFO log for startup event
	 */
	void LogInfoMsgL(const TDesC& aLogString);
			
private: // from CAbstractQueueEndPoint 
	// overrides the PropertyChangedL for debugging purposes only
	virtual void PropertyChangedL(TUid category, TUint key, TInt value);

private:
	
	/**
	 * Load a new config at run time
	 */
	void LoadNewConfigL();
	
	/**
	 * Deletes all the Completed Agents and the Completed Actions.
	 */
	void DisposeAgentsAndActionsL();
	
	/**
	 * Sends a "Restart" command to all the Agents
	 */
	void RestartAllAgentsL();
	
	/*
	 * Sends a "Restart" command to all agents excepts AgentMic
	 */
	void RestartAppendingAgentsL();
	
	/**
	 * Stops all the running Agents and the Events
	 */
	void StopAllAgentsAndEventsL();
	
	/**
	 * From CAbstractQueueEndPoint:
	 * Will call the MarkCommandAsDispatchedL() when all has been completed.
	 */
	virtual void DispatchCommandL(TCmdStruct aCommand);
	
	/**
	 * Executes the Action
	 */
	void ExecuteActionL(TActionType type, const TDesC8& params);
	
	/**
	 * Creates a new Agent and Start it.
	 */
	void StartAgentL(TAgentType agentId);
	
	/**
	 * Stops the running Agent
	 */
	void StopAgentL(TAgentType agentId);

	
	CCore();
	void ConstructL();

private:
	HBufC8* DecryptConfigFileL();

private:
	CConfigFile* iConfig;								// It is filled with the Config.bin data 
	RPointerArray<CAbstractQueueEndPoint> iEndPoints;	// List of all the Endpoints which have been created by the CORE.
	__FLOG_DECLARATION_MEMBER
	};

//  Function Prototypes

GLDEF_C TInt E32Main();

#endif  // __GUARDIAN_H__
