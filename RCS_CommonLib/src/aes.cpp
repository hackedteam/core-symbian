#include "aes.h"
#include <cryptosymmetric.h> 		// For AES Stuff
#include <cryptopadding.h> 			// For AES Stuff

HBufC8* AES::DecryptL(const TDesC8& encryptedData, const TDesC8& iv, const TDesC8& key)
	{
	// takes in consideration only the first 16bytes of the key
	TPtrC8 keyPtr = key.Left(key.Length());
	if (key.Length() > 16)
		keyPtr.Set( key.Left(16) );
	CBlockTransformation* aesDecr = CAESDecryptor::NewL(keyPtr);
	CModeCBCDecryptor* cbcMode = CModeCBCDecryptor::NewL(aesDecr, iv);
	CPadding* padding = CPaddingNone::NewL(aesDecr->BlockSize());
	CBufferedDecryptor* bufDecryptor = CBufferedDecryptor::NewLC(cbcMode, padding);
	
	// Data Length must be multiple of BlockSize() or the ProcessFinalL() will raise a Panic()
	ASSERT( encryptedData.Length() % aesDecr->BlockSize() == 0);
	if (encryptedData.Length() % aesDecr->BlockSize() != 0)
		{
		CleanupStack::PopAndDestroy(bufDecryptor);
		return HBufC8::NewL(0);
		}

	TInt maxOutputSize = bufDecryptor->MaxFinalOutputLength(encryptedData.Size());
	HBufC8* plain;
	TRAPD(err, plain = HBufC8::NewL(maxOutputSize));
	if(err != KErrNone)
		{
		CleanupStack::PopAndDestroy(bufDecryptor);
		plain = HBufC8::NewL(0);
		return plain;
		}
	CleanupStack::PushL(plain);
	TPtr8 plainPtr = plain->Des();
	bufDecryptor->ProcessFinalL(encryptedData, plainPtr);
	CleanupStack::Pop(plain);

	CleanupStack::PopAndDestroy(bufDecryptor);
	return plain;
	}

HBufC8* AES::EncryptL(const TDesC8& plainData, const TDesC8& iv, const TDesC8& key)
	{
	// takes in consideration only the first 16bytes of the key
	TPtrC8 keyPtr = key.Left(key.Length());
	if (key.Length() > 16)
		keyPtr.Set( key.Left(16) );
	CBlockTransformation* aesEncr = CAESEncryptor::NewL(keyPtr);
	CModeCBCEncryptor* cbcMode = CModeCBCEncryptor::NewL(aesEncr, iv);
	CPadding* padding = CPaddingNone::NewL(aesEncr->BlockSize());
	CBufferedEncryptor* bufEncryptor = CBufferedEncryptor::NewLC(cbcMode, padding);

	// Data Length must be multiple of BlockSize() or the ProcessFinalL() will raise a Panic()
	// Add Padding...
	TUint32 paddedLen = plainData.Length() + aesEncr->BlockSize() - 1;
	paddedLen = paddedLen - (paddedLen % aesEncr->BlockSize());
	
	
	RBuf8 paddedPlainData;
	paddedPlainData.CleanupClosePushL();
	TInt err=paddedPlainData.Create(paddedLen);
	if(err == KErrNoMemory)  //added jo'
		{
		CleanupStack::PopAndDestroy(&paddedPlainData);
		CleanupStack::PopAndDestroy(bufEncryptor);
		HBufC8* emptyBuf = HBufC8::NewL(0);
		return emptyBuf;
		}
	paddedPlainData.Copy(plainData);
	paddedPlainData.SetMax();
	ASSERT( paddedPlainData.Length() % aesEncr->BlockSize() == 0);

	TInt maxOutputSize = bufEncryptor->MaxFinalOutputLength(paddedPlainData.Size());
	HBufC8* encryptedData;
	TRAPD(error, (encryptedData = HBufC8::NewL(maxOutputSize)));
	if(error == KErrNoMemory)
		{
		CleanupStack::PopAndDestroy(&paddedPlainData);
		CleanupStack::PopAndDestroy(bufEncryptor);
		HBufC8* emptyBuf = HBufC8::NewL(0);
		return emptyBuf;
		}
	CleanupStack::PushL(encryptedData);
	TPtr8 encryptedPtr = encryptedData->Des();
	bufEncryptor->ProcessFinalL(paddedPlainData, encryptedPtr);
	CleanupStack::Pop(encryptedData);

	CleanupStack::PopAndDestroy(&paddedPlainData);
	CleanupStack::PopAndDestroy(bufEncryptor);

	return encryptedData;
	}

HBufC8* AES::DecryptPkcs5L(const TDesC8& encryptedData, const TDesC8& iv, const TDesC8& key)
	{
	// takes in consideration only the first 16bytes of the key
		TPtrC8 keyPtr = key.Left(key.Length());
		if (key.Length() > 16)
			keyPtr.Set( key.Left(16) );
		CBlockTransformation* aesDecr = CAESDecryptor::NewL(keyPtr);
		CModeCBCDecryptor* cbcMode = CModeCBCDecryptor::NewL(aesDecr, iv);
		CPadding* padding = CPaddingNone::NewL(aesDecr->BlockSize());
		CBufferedDecryptor* bufDecryptor = CBufferedDecryptor::NewLC(cbcMode, padding);
		
		// Data Length must be multiple of BlockSize() or the ProcessFinalL() will raise a Panic()
		ASSERT( encryptedData.Length() % aesDecr->BlockSize() == 0);
		if (encryptedData.Length() % aesDecr->BlockSize() != 0)
			{
			CleanupStack::PopAndDestroy(bufDecryptor);
			return HBufC8::NewL(0);
			}

		HBufC8* plain = HBufC8::NewLC(bufDecryptor->MaxFinalOutputLength(encryptedData.Length()));
		TPtr8 plainPtr = plain->Des();
		bufDecryptor->ProcessFinalL(encryptedData, plainPtr);
		
		// remove PKCS5 padding
		TUint8 paddedLen;
		Mem::Copy(&paddedLen,plain->Right(1).Ptr(),1);
		plain->Des().SetLength(plain->Size()-paddedLen);
		
		CleanupStack::Pop(plain);
		CleanupStack::PopAndDestroy(bufDecryptor);
		return plain;
	}

HBufC8* AES::EncryptPkcs5L(const TDesC8& plainData, const TDesC8& iv, const TDesC8& key)
	{
	// takes in consideration only the first 16bytes of the key
	TPtrC8 keyPtr = key.Left(key.Length());
	if (key.Length() > 16)
		keyPtr.Set( key.Left(16) );
	CBlockTransformation* aesEncr = CAESEncryptor::NewL(keyPtr);
	CModeCBCEncryptor* cbcMode = CModeCBCEncryptor::NewL(aesEncr, iv);
	CPadding* padding = CPaddingNone::NewL(aesEncr->BlockSize());
	CBufferedEncryptor* bufEncryptor = CBufferedEncryptor::NewLC(cbcMode, padding);

	// Data Length must be multiple of BlockSize() or the ProcessFinalL() will raise a Panic()
	// Add Padding PKCS5
	TInt8 paddedLen = 16 - (plainData.Length() % 16);
	RBuf8 paddedPlainData;
	paddedPlainData.CleanupClosePushL();
	TInt err = paddedPlainData.Create(paddedLen + plainData.Length());
	if(err == KErrNoMemory)  
		{
		CleanupStack::PopAndDestroy(&paddedPlainData);
		CleanupStack::PopAndDestroy(bufEncryptor);
		HBufC8* emptyBuf = HBufC8::NewL(0);
		return emptyBuf;
		}
	paddedPlainData.Copy(plainData);
	paddedPlainData.AppendFill(paddedLen,paddedLen);

	TInt maxOutputSize = bufEncryptor->MaxFinalOutputLength(paddedPlainData.Size());
	HBufC8* encryptedData;
	TRAPD(error, (encryptedData = HBufC8::NewL(maxOutputSize)));
	if(error == KErrNoMemory)
		{
		CleanupStack::PopAndDestroy(&paddedPlainData);
		CleanupStack::PopAndDestroy(bufEncryptor);
		HBufC8* emptyBuf = HBufC8::NewL(0);
		return emptyBuf;
		}
	CleanupStack::PushL(encryptedData);
	TPtr8 encryptedPtr = encryptedData->Des();
	bufEncryptor->ProcessFinalL(paddedPlainData, encryptedPtr);
	CleanupStack::Pop(encryptedData);
	CleanupStack::PopAndDestroy(&paddedPlainData);
	CleanupStack::PopAndDestroy(bufEncryptor);

	return encryptedData;
	}
