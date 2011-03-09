
// INCLUDE FILES
#include "TimeOutTimer.h"

// ========================= MEMBER FUNCTIONS ==================================

// -----------------------------------------------------------------------------
// CTimeOutTimer::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CTimeOutTimer* CTimeOutTimer::NewL(MTimeOutNotifier& aTimeOutNotify, const TInt aPriority )
    {
    CTimeOutTimer* self = CTimeOutTimer::NewLC(aTimeOutNotify, aPriority );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CTimeOutTimer::NewLC()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CTimeOutTimer* CTimeOutTimer::NewLC( MTimeOutNotifier& aTimeOutNotify, const TInt aPriority )
    {
    CTimeOutTimer* self = new ( ELeave ) CTimeOutTimer( aTimeOutNotify, aPriority );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CTimeOutTimer::CTimeOutTimer()
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CTimeOutTimer::CTimeOutTimer( MTimeOutNotifier& aTimeOutNotify, const TInt aPriority )
: CTimer( aPriority ), iNotify( aTimeOutNotify )
    {
    // No implementation required
    }

// -----------------------------------------------------------------------------
// CTimeOutTimer::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CTimeOutTimer::ConstructL()
    {
    CTimer::ConstructL();
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CTimeOutTimer::~CTimeOutTimer()
// Destructor.
// -----------------------------------------------------------------------------
//
CTimeOutTimer::~CTimeOutTimer()
    {
	Cancel();
    }

// -----------------------------------------------------------------------------
// CTimeOutTimer::RunL()
// Called when operation completes.
// -----------------------------------------------------------------------------
//
void CTimeOutTimer::RunL()
    {
    // Timer request has completed, so notify the timer's owner
    // User::LeaveIfError( iStatus.Int() );
    iNotify.TimerExpiredL( this );
    }

// End of File
