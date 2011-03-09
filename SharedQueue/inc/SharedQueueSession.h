
#ifndef __T_SESSION_H__
#define __T_SESSION_H__

#include <e32base.h>
#include "SharedQueueSrv.h"

//class CLicense;


#include <HT\logging.h>

class CSharedQueueSession : public CSession2
	{
public:
	CSharedQueueSession();
	void CreateL();
private:
	TUint ClientUID(const RMessagePtr2& aMessage);

	void IsEmptyL(const RMessage2& aMessage);
	void CSharedQueueSession::LockTop(const RMessage2& aMessage);
	void TopL(const RMessage2& aMessage);
	void TopParamL(const RMessage2& aMessage);
	void DequeueL(const RMessage2& aMessage);
	void EnqueueL(const RMessage2& aMessage);
	void DoEmptyL(const RMessage2& aMessage);                           // added jo

	void SendMessageL(const RMessage2& aMessage);
	virtual ~CSharedQueueSession();
	inline CSharedQueueSrv& Server();
	void ServiceL(const RMessage2& aMessage);
	void ServiceError(const RMessage2& aMessage, TInt aError);

private:
	__FLOG_DECLARATION_MEMBER
	};

#endif

