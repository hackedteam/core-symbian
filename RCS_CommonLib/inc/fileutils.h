//LIBRARY	charconv.lib efsrv.lib bafl.lib euser.lib apgrfx.lib apmime.lib 
#ifndef _FILEUTILS_H
#define _FILEUTILS_H
#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		
#include <e32cmn.h>
#include <f32file.h>


/*#ifdef _DEBUG
_LIT8(KTMP_CONFNAME, "2009093023.tmp");
_LIT8(KCONFNAME, "2009093023");
#else*/
// K_FAKE_CONFNAME reference is there just to make happy the binary patcher for now.
_LIT(KTMP_CONFNAME, "2009093023.tmp");
_LIT(KCONFNAME, "2009093023");
_LIT(K_FAKE_CONFNAME, "c3mdX053du1YJ541vqWILrc4Ff71pViL\x00");
//#endif

class FileUtils
	{
public:
	static void CompleteWithCurrentDrive(TDes& fileName);
	static void CompleteWithPrivatePathL(RFs& fs, TDes& fileName);
	static TInt GetFileSize(RFs& fs, const TDesC& filename);
	static HBufC8* ReadFileContentsL(RFs& fs, const TDesC& filename);
	//static HBufC8* DecryptConfigFileL(RFs& fs);
	static void ListFilesInDirectoryL(RFs& fs, const TDesC& search, RPointerArray<HBufC>& array);
	};

#endif

