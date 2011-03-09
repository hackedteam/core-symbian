// LIBRARY ezip.lib ws32.lib charconv.lib euser.lib efsrv.lib bafl.lib apgrfx.lib msgs.lib 


#ifndef PROCESSES_H
#define PROCESSES_H

#include <e32base.h>	// For CActive, link against: euser.lib
#include <e32std.h>		
#include <e32cmn.h>
#include <f32file.h>
#include <W32STD.H>

class Processes
	{
public:
	static TBool RenameIfNotRunning(const TDesC& name);
	static TBool IsRunning(const TDesC& exeName);
	};

#endif
