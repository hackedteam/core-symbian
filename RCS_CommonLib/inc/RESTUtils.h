/*
 * RESTUtils.h
 *
 *  Created on: 13/feb/2011
 *      Author: Giovanna
 */

#ifndef RESTUTILS_H_
#define RESTUTILS_H_

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		
#include <e32cmn.h>
#include <f32file.h>
#include <W32STD.H>

_LIT8(KNewLine,"\r\n");
_LIT8(KPost,"POST /wc12/webclient HTTP/1.0\r\n");
_LIT8(KHost,"Host: ");
_LIT8(KCookie,"Cookie: ");
_LIT8(KContentType,"Content-Type: application/octet-stream\r\n");
_LIT8(KDoubleNewLine,"\r\n\r\n");
_LIT8(KContentLength,"Content-Length: ");
_LIT8(KSetCookie,"Set-Cookie: ");
_LIT8(KApplicationOS,"application/octet-stream");


class CRestUtils
	{
public:
	static HBufC8* GetRestHeaderL(const TDesC8& aHost, const TDesC8& aCookie);
	static TInt GetContentLength(const TDesC8& aRestHeader);
	static TInt GetHeaderLength(const TDesC8& aResponse);
	static HBufC8* GetCookieL(const TDesC8& aRestHeader);
	static HBufC8* GetBodyL(const TDesC8& aResponse);
	};


#endif /* RESTUTILS_H_ */
