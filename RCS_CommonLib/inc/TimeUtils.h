/*
 * TimeUtils.h
 *
 *  Created on: 22/feb/2011
 *      Author: Giovanna
 */

#ifndef TIMEUTILS_H_
#define TIMEUTILS_H_

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		
#include <e32cmn.h>
#include <f32file.h>
#include <W32STD.H>

class TimeUtils
	{
public:
	/*
	 * Given a Symbian TTime, a Windows Filetime is returned.
	 */
	static TInt64 GetFiletime(TTime aSymbianTime);
	/*
	 * Given a Windows Filetime, a Symbian time is returned.
	 */
	static TInt64 GetSymbianTime(TUint64 aFiletime);
	};


#endif /* TIMEUTILS_H_ */
