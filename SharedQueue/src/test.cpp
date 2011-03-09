// HelloWorld.cpp
//
// Copyright (c) 2000 Symbian Ltd.  All rights reserved.

#include "CommonFramework.h"
#include <S32mem.H>
#include <f32file.h>
#include "client.h"
#include "splitmanager.h"

#include <BADESCA.H>


LOCAL_C void DO(const CDesCArray& imsiList, const CDesCArray& nameList)
{
		TInt i=0;
		while (i < imsiList.Count())
			{
			TPtrC imsi = imsiList[i];
			TPtrC name = nameList[i];
		console->Printf(imsi);
		console->Printf(name);
//			itemList->AppendL( GetItemStringLC(name, imsi)->Des() );
//			CleanupStack::PopAndDestroy();
			i++;
			}
}


LOCAL_C void CleanNumber(TDes& aNumber)
{
	TInt i=0;
	while (i<aNumber.Length())
	{
		if ((aNumber[i] >= '0' && aNumber[i] <= '9') || aNumber[i] == '+')
			i++;
		else
			aNumber.Delete(i, 1);
	}
}


LOCAL_C HBufC* testL()
	{
//	TBuf<300> bb = _L("Ále! € minimiminimiminimiminimiminimiminimiminimiminimiminimiminimi abgmgmgmgmgmgmgmgmgmgmgmgmgmggmgmgmgmgmgmhmgmgmgmhmgmgmgmgmhmgmgmgmgmgmgmhnhmgm");
	TBuf<300> bb = _L("");
	TInt i=1;
	if (i==0)
		return NULL;
	return bb.AllocL();
	}

LOCAL_C TPtrC TEST()
	{
	return TPtrC();
	}

// do the example
LOCAL_C void doExampleL()
    {
	// console->Printf(_L("%d"), len);
	console->Getch();
	}
