

#ifndef NETWORK_H
#define NETWORK_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <in_sock.h>
#include <HT\TimeOutTimer.h>
#include <w32std.h>

//#include "Monitor.h"

// CLASS DECLARATION

// CLASS DECLARATION
class MNetworkObserver
	{
	public:
		/**
		 * Notify observer about connection completion
		 */
		virtual void NotifyConnectionCompleteL() = 0;

		/**
		 * Notify observer about disconnection completion
		 */
		virtual void NotifyDisconnectionCompleteL() = 0;

		/**
		 * Notify observer about data received from server
		 * @param aData Received buffer
		 */
		virtual void NotifyDataReceivedL(const TDesC8& aData) = 0;

		/**
		 * Notify observer about sending completion
		 */
		virtual void NotifySendingCompleteL() = 0;

		/**
		 * Notify observer about network errors
		 * @param aError Error code
		 */
		virtual void NotifyNetworkError(TInt aError) = 0;
	};


/**
*  CNetwork
* 
*/
class CNetwork : public CActive, MTimeOutNotifier //, public MMonitorObserver
	{
	public: // Constructors and destructor

		/**
		* Destructor.
		*/
		~CNetwork();

		/**
		* Two-phased constructor.
		*/
		static CNetwork* NewL(RSocketServ& aSocketServ, RConnection& aConnection, MNetworkObserver& aObserver);

		/**
		* Two-phased constructor.
		*/
		static CNetwork* NewLC(RSocketServ& aSocketServ, RConnection& aConnection, MNetworkObserver& aObserver);

	public: // Enums
		enum TConnectionState
			{
			EConnectionStateDnsLookingUp,
			EConnectionStateConnecting,
			EConnectionStateReceiving,
			EConnectionStateSending,
			EConnectionStateDisconnected
			};

		enum { KReadBufferSize = 4096 };

	public: // New methods
		/**
		* Connects to the service
		 * @param aUrl Server URL
		 * @param aPort Server opened port
		 */
		void ConnectToServerL(const TDesC& aUrl, TInt aPort);

		/**
		* Disconnects the object from the network
		*/
		void Disconnect();

		/**
		* Sends data through network
		* @param aDataBuffer Data for sending
		*/
		void SendL(const TDesC8& aDataBuffer);
		
		/**
		 * @return true is the send operation is in progress
		 */
		TBool SendInProgressL();

	protected: // for internal usage
		/**
		* Tries to connect object to the remote end
		* @param aAddr Adressee point
		*/
		void ConnectL(TUint32 aAddr);

		/**
		* Starts data receiving. Received data will be put in the object buffer
		*/
		void DoReceiveL();

		/**
		 * Handle DNS looking up success completion
		 */
		void HandleDnsLookingUpCompleteL();

		/**
		 * Handle connection to server success completion
		 */
		void HandleConnectionCompleteL();

		/**
		 * Handle date received from server
		 */
		void HandleDataReceivedL();

		/**
		 * Handle sending data success completion
		 */
		void HandleSendingCompleteL();

		/**
		 * Handle error happened during network operation
		 * @param aError Error code
		 */
		void HandleConnectionErrorL(TInt aError);
		
		//MMonitorObserver
		//TBool KeyEventCaptured(TWsEvent aEvent);
		
	private: // MTimeOutNotifier
        void TimerExpiredL(TAny* src);

	private: // CActive
		/**
		* Handle completion
		*/
		void RunL();

		/**
		* Handle cancellation of outstanding request
		*/
		void DoCancel();
		

	private:
		/**
		* Constructor for performing 1st stage construction
		*/
		CNetwork(RSocketServ& aSocketServ, RConnection& aConnection, MNetworkObserver& aObserver);

		/**
		* EPOC default constructor for performing 2nd stage construction
		*/
		void ConstructL();

	private: // Data
		
		
		RSocketServ&			iSocketServ;
		RConnection&			iConnection;
		RHostResolver			iResolver;
		TNameEntry				iNameEntry;
		RSocket					iSocket;
		TInt					iPort;
		TConnectionState		iConnectionState;
		TBuf8<KReadBufferSize>	iReadBuffer;
		HBufC8*					iWriteBuffer;
		TSockXfrLength			iBytesRead;
		MNetworkObserver&		iObserver;
		CTimeOutTimer* 			iTimer;
		
	};

#endif // NETWORKLAYER_H
