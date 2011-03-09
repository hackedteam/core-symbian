#ifndef __T_CLIENT_H__
#define __T_CLIENT_H__

#include <e32std.h>
#include <e32base.h>
#include <HT\SharedQueueCliSrv.h>

class RSharedQueue : public RSessionBase
	{
public:
	IMPORT_C TInt Connect();

	IMPORT_C TCmdStruct Top();
	IMPORT_C HBufC8* TopParamL();
	IMPORT_C TBool LockTop();
	IMPORT_C TBool IsEmpty();
	IMPORT_C void Enqueue(TCmdStruct aCmd, const TDesC8& params = KNullDesC8());
	IMPORT_C void Enqueue(TCmdType aType, TInt aSrc, TInt aDest, const TDesC8& params = KNullDesC8());
	IMPORT_C TCmdStruct Dequeue();
	IMPORT_C void DoEmpty();
	};

#endif

