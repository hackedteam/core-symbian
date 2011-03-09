/*
 * EventConnection.cpp
 *
 *  Created on: 08/ott/2010
 *      Author: Giovanna
 */

#include "EventConnection.h"

CEventConnection::CEventConnection(TUint32 aTriggerId) :
	CAbstractEvent(EEvent_Connection, aTriggerId),iConnCount(0)
	{
	// No implementation required
	}

CEventConnection::~CEventConnection()
	{
	//__FLOG(_L("Destructor"));
	iConnMon.CancelNotifications();
	iConnMon.Close();
	//__FLOG(_L("End Destructor"));
	//__FLOG_CLOSE;
	} 

CEventConnection* CEventConnection::NewLC(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventConnection* self = new (ELeave) CEventConnection(aTriggerId);
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CEventConnection* CEventConnection::NewL(const TDesC8& params, TUint32 aTriggerId)
	{
	CEventConnection* self = CEventConnection::NewLC(params, aTriggerId);
	CleanupStack::Pop(); // self;
	return self;
	}

void CEventConnection::ConstructL(const TDesC8& params)
	{
	//__FLOG_OPEN_ID("HT", "EventBattery.txt");
	//__FLOG(_L("-------------"));
	
	BaseConstructL(params);
	Mem::Copy(&iConnParams, iParams.Ptr(), sizeof(iConnParams));
	
	User::LeaveIfError(iConnMon.ConnectL());
	
	}

void CEventConnection::StartEventL()
	{
	// check for active connections
	iConnCount = 0;
	iWasConnected = EFalse;
	
	TUint connCount;
	TRequestStatus status;
	iConnMon.GetConnectionCount(connCount,status);
	User::WaitForRequest(status);
	if ((status.Int() != KErrNone) || (connCount == 0))
		{
		iConnMon.NotifyEventL(*this);
		return;
		}
	
	TUint connId;
	TUint subConnCount;
	TInt bearerType;  //TConnMonBearerType
	
	for(TUint i=0; i<connCount; i++)
		{
		TInt err = iConnMon.GetConnectionInfo(i,connId,subConnCount);
		if(err != KErrNone)
			{
			continue;
			}
		iConnMon.GetIntAttribute(connId,0,KBearer,bearerType,status);
		User::WaitForRequest(status);
		if(status.Int() != KErrNone)
			{
			continue;
			}
		if((bearerType >= EBearerWCDMA) && (bearerType <= EBearerVirtualVPN));
			{
			++iConnCount;
			}
	
		}
	if(iConnCount>0)
		{
		iWasConnected = ETrue;
		SendActionTriggerToCoreL();
		}
	
	iConnMon.NotifyEventL(*this);
	}

void CEventConnection::EventL( const CConnMonEventBase& aEvent )
{
	
    switch( aEvent.EventType() )
        {
        case EConnMonCreateConnection:
            {
            TInt bearerType;
            TRequestStatus status;
            iConnMon.GetIntAttribute(aEvent.ConnectionId(),0,KBearer,bearerType,status);
            User::WaitForRequest(status);
            if(status.Int() != KErrNone)
            	{
            	break;
            	}
            if((bearerType >= EBearerWCDMA) && (bearerType <= EBearerVirtualVPN));
            	{
            	++iConnCount;
            	}
            if( !iWasConnected && (iConnCount>0))
            	{
				iWasConnected = ETrue;
				SendActionTriggerToCoreL();
            	}
            break;
            }
        case EConnMonDeleteConnection:
            {
            --iConnCount;
            if( iWasConnected && (iConnCount == 0))
            	{
            	iWasConnected = EFalse;
            	if (iConnParams.iExitAction != 0xFFFFFFFF)
					{
            		SendActionTriggerToCoreL(iConnParams.iExitAction);
            		}
            	}
            break;
            }
        case EConnMonDownlinkDataThreshold:
            {
            break;
            }
        case EConnMonNetworkRegistrationChange:
            {
            break;
            }
/*        case EConnMonSNAPsAvailabilityChange: // FP 1 -->
            {
            CConnMonSNAPsAvailabilityChange* eventSNAPsAvailChange;
            eventSNAPsAvailChange = (CConnMonSNAPsAvailabilityChange*)& aEvent;
            // amount of ids, really available on server side
            TUint totalSNAPs(eventSNAPsAvailChange->SNAPsAvailabile());
            if(totalSNAPs != eventSNAPsAvailChange->SNAPAvailability().iCount)
                {
                // not all SNAPs Ids have been received
                // act accordingly to a client’s needs
                // for example, request SNAP’s ids by using GetPckgAttribute, 
                // and specifying bigger buffer (exact size of needed buffer 
                // can be calculated by using totalSNAPs value)
                }
            // here could be “else” – but this is just an example
            for(TUint i(0);i< eventSNAPsAvailChange->SNAPAvailability().iCount;++i)
                {
                TConnMonSNAPId SNAPId(eventSNAPsAvailChange->SNAPAvailability().iSNAP[i]);
                // process SNAP’s id		
                	Hjelppppp.Copy(_L("SNAP"));
    	        	Hjelppppp2.Num(i,EDecimal);
    	        	Hjelppppp2.Append(_L(","));
	    	        Hjelppppp2.AppendNum(SNAPId.iSNAPId,EDecimal);
                } 
            break;
            }  
 */   	case EConnMonCreateSubConnection:
    		{
       		}
    		break;
    	case EConnMonDeleteSubConnection:
    		{
    		}
    		break;
    	case EConnMonUplinkDataThreshold:
    		{
    		}
    		break;
    	case EConnMonNetworkStatusChange:
    		{
    		}
    		break;
    	case EConnMonConnectionStatusChange:
    		{
    		}
    		break;
    	case EConnMonConnectionActivityChange:
    		{
    		}
    		break;
    	case EConnMonBearerChange:
    		{
    		}
    		break;
    	case EConnMonSignalStrengthChange:
    		{
    		}
    		break;
    	case EConnMonBearerAvailabilityChange:
    		{	
    		}
    		break;
    	case EConnMonIapAvailabilityChange:
    		{
    		}
    		break;
    	case EConnMonTransmitPowerChange:
    		{
    		}
    		break;
/*		case EConnMonNewWLANNetworkDetected:// FP1 -->
    		{
    			Hjelppppp.Copy(_L("WLANNetworkDetected"));
    		}
    		break;
		case EConnMonOldWLANNetworkLost:
    		{
    			Hjelppppp.Copy(_L("WLANNetworkLost"));// FP1 -->
    		}
    		break; */
        	default:
        	{
        //		Hjelppppp.Append(_L("default"));
        //		Hjelppppp.AppendNum(aEvent.EventType(),EDecimal);
        	}
        // for future events, unrecognized events must not crash application
        	break;
        }

}

