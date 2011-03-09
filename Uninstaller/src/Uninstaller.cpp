/*
 ============================================================================
 Name		: Uninstaller.cpp
 Author	  : 
 Copyright   : 
 Description : Exe source file
 ============================================================================
 */

//  Include Files  

#include "Uninstaller.h"
#include <e32base.h>
#include <e32std.h>
#include <e32cons.h>			// Console
#include <swinstapi.h>			// Uninstaller
#include <swinstdefs.h>


//  Local Functions

LOCAL_C void MainL()
	{
	User::After(10*1000000);
	// Prepare for uninstall	
	SwiUI::RSWInstLauncher iLauncher ;
	SwiUI::TUninstallOptions iOptions;
	SwiUI::TUninstallOptionsPckg iOptionsPckg; 
	iOptions.iKillApp=SwiUI::EPolicyAllowed;
	iOptionsPckg = iOptions; 
	TUid kUid = {0x200305D7};   //  Note! UID of your SIS file NOT of your app
	
	TInt err = iLauncher.Connect();
	if(err == KErrNone)
		{
		// Uninstall Without Call back request
		TInt a;
		// Silent uninstall
		a=iLauncher.SilentUninstall(kUid, iOptionsPckg,SwiUI::KSisxMimeType) ;
		}
	iLauncher.Close();
	}

LOCAL_C void DoStartL()
	{
	// Create active scheduler (to run active objects)
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);

	MainL();

	// Delete active scheduler
	CleanupStack::PopAndDestroy(scheduler);
	
	}

//  Global Functions

GLDEF_C TInt E32Main()
	{
	// Create cleanup stack
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();
	
	// Run application code inside TRAP harness, wait keypress when terminated
	TRAPD(mainError, DoStartL());

	delete cleanup;
	__UHEAP_MARKEND;
	return mainError;
	}

