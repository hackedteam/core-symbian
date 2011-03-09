// LIBRARY ezip.lib ws32.lib charconv.lib euser.lib efsrv.lib bafl.lib apgrfx.lib msgs.lib 


#ifndef AES_H
#define AES_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		
#include <e32cmn.h>
#include <f32file.h>
#include <W32STD.H>

class AES
	{
public:
	//for evidences encryption/decryption
	static HBufC8* DecryptL(const TDesC8& encryptedData, const TDesC8& iv, const TDesC8& key);
	static HBufC8* EncryptL(const TDesC8& plainData, const TDesC8& iv, const TDesC8& key);
	
	//for protocol encryption/decryption
	static HBufC8* DecryptPkcs5L(const TDesC8& encryptedData, const TDesC8& iv, const TDesC8& key);
	static HBufC8* EncryptPkcs5L(const TDesC8& plainData, const TDesC8& iv, const TDesC8& key);

	};

#endif
