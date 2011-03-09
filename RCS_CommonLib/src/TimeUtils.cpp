/*
 * TimeUtils.cpp
 *
 *  Created on: 22/feb/2011
 *      Author: Giovanna
 */

#include "TimeUtils.h"

TInt64 TimeUtils::GetFiletime(TTime aSymbianTime)
	{
	_LIT(KInitialTime,"16010000:000000");
	TTime initialTime;
	initialTime.Set(KInitialTime);
			
	TTimeIntervalMicroSeconds interval;
	interval=aSymbianTime.MicroSecondsFrom(initialTime);
		
	return interval.Int64()*10; 
		
	}

/*
 * A filetime is a 64-bit value that represents the number of 100-nanosecond intervals 
 * that have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC).
 * Please also note that in defining KInitialTime the month and day values are offset from zero.
 * 
 */
TInt64 TimeUtils::GetSymbianTime(TUint64 aFiletime)
	{

	_LIT(KFiletimeInitialTime,"16010000:000000");

	TTime initialFiletime;
	initialFiletime.Set(KFiletimeInitialTime);

	TInt64 interval;
	interval = initialFiletime.Int64();

	TInt64 date = aFiletime/10;

	return (interval + date);
	}
