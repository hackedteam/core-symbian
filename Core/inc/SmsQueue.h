/*
 ============================================================================
 Name		: Queue.h
 Author	  : Marco Bellino
 Version	 : 1.0
 Copyright   : Guardian-Mobile
 Description : CSmsQueue declaration
 ============================================================================
 */

#ifndef SMSQUEUE_H
#define SMSQUEUE_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>

// CLASS DECLARATION

/**
 *  CSmsQueue
 * 
 */
class CSmsQueue : public CBase
	{
public:
	// Constructors and destructor

	/**
	 * Destructor.
	 */
	~CSmsQueue();

	/**
	 * Two-phased constructor.
	 */
	static CSmsQueue* NewL();

	/**
	 * Two-phased constructor.
	 */
	static CSmsQueue* NewLC();

	void EnqueueL(const TDesC& filename);
	HBufC* Dequeue();
	void Clear();
private:

	/**
	 * Constructor for performing 1st stage construction
	 */
	CSmsQueue();

	/**
	 * EPOC default constructor for performing 2nd stage construction
	 */
	void ConstructL();

private:
	RPointerArray<HBufC> iQueue;
	};

#endif // QUEUE_H
