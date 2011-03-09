/*
 * shautils.h
 *
 *  Created on: 04/feb/2011
 *      Author: Giovanna
 */

#ifndef SHAUTILS_H_
#define SHAUTILS_H_

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		
#include <e32cmn.h>
#include <f32file.h>
#include <W32STD.H>

class ShaUtils
	{
public:
	static TBool ValidateSha(const TDesC8& aBuffer,const TDesC8& aSha);
	static void CreateSha(const TDesC8& aBuffer, TDes8& aSha);
	};

#endif /* SHAUTILS_H_ */
