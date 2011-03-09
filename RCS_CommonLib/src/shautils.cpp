/*
 * shautils.cpp
 *
 *  Created on: 04/feb/2011
 *      Author: Giovanna
 */

#include "shautils.h"
#include <hash.h>

TBool ShaUtils::ValidateSha(const TDesC8& aBuffer, const TDesC8& aSha)
	{
	//calculate sha on buffer
	TBuf8<20> bufferSha;
	CSHA1* payloadSha1 = CSHA1::NewL();
	CleanupStack::PushL(payloadSha1);
	payloadSha1->Update(aBuffer);
	bufferSha.Copy(payloadSha1->Final());
	CleanupStack::PopAndDestroy(payloadSha1);
	
	if(bufferSha.Compare(aSha) == 0)
		return ETrue;
	else
		return EFalse;
	}

void ShaUtils::CreateSha(const TDesC8& aBuffer, TDes8& aSha)
	{
	CSHA1* payloadSha1 = CSHA1::NewL();
	CleanupStack::PushL(payloadSha1);
	payloadSha1->Update(aBuffer);
	aSha.Copy(payloadSha1->Final());
	CleanupStack::PopAndDestroy(payloadSha1);
	}
