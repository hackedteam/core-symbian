#include "fileutils.h"
#include <CHARCONV.H>
#include <bautils.h>
#include <f32file.h>
#include <APGCLI.H>
#include "aes.h"

void FileUtils::CompleteWithCurrentDrive(TDes& fileName)
	{
#ifdef __WINSCW__
	fileName.Insert(0, _L("C:"));
	return;
#endif
	TParsePtrC parse(RProcess().FileName());
	fileName.Insert(0, parse.Drive());
	}

void FileUtils::CompleteWithPrivatePathL(RFs& fs, TDes& fileName)
	{
	TFileName fullFileName;
	fs.PrivatePath(fullFileName);
	fullFileName.Append(fileName);
	FileUtils::CompleteWithCurrentDrive(fullFileName);
	fileName = fullFileName;
	}

TInt FileUtils::GetFileSize(RFs& fs, const TDesC& filename)
	{
	TInt size = 0;
	RFile file;
	TInt err = file.Open(fs, filename, EFileShareReadersOrWriters | EFileRead);
	if (err != KErrNone)
		{
		file.Close();
		return 0;
		}
	file.Size(size);
	file.Close();
	return size;
	}

HBufC8* FileUtils::ReadFileContentsL(RFs& fs, const TDesC& filename)
	{
	RFile file;
	User::LeaveIfError(file.Open(fs, filename, EFileShareReadersOnly | EFileRead));
	CleanupClosePushL(file);

	TInt fileSize;
	User::LeaveIfError(file.Size(fileSize));

	HBufC8* buf;
	TRAPD(err,(buf=HBufC8::NewL(fileSize)));
	if(err != KErrNone)
		{
		CleanupStack::PopAndDestroy(&file);
		buf = HBufC8::NewL(0);
		return buf;
		}
	TPtr8 bufPtr = buf->Des();
	file.Read(bufPtr);
	CleanupStack::PopAndDestroy(&file);
	return buf;
	}

void FileUtils::ListFilesInDirectoryL(RFs& fs, const TDesC& search, RPointerArray<HBufC>& array)
	{
	CDirScan* dirScan = CDirScan::NewLC(fs);
	//dirScan->SetScanDataL(search, KEntryAttMatchMask, ESortByName, CDirScan::EScanDownTree);
	// ESortByDate Sort according to files' last modified time and date.
	// By default, most recent last (first  the oldest, last the newest)
	// see also EAscending (default), EDiscending
	dirScan->SetScanDataL(search, KEntryAttMatchMask, ESortByDate, CDirScan::EScanDownTree);

	CDir* dir = NULL;
	dirScan->NextL(dir);
	while (dir)
		{
		CleanupStack::PushL(dir);
		for (TInt i = 0; i < dir->Count(); i++)
			{
			TEntry entry = (*dir)[i];
			TPtrC fullPath = dirScan->FullPath();
			HBufC* fname = HBufC::NewLC(fullPath.Length() + entry.iName.Length() + 2);
			fname->Des().Append(fullPath);
			fname->Des().Append(entry.iName);
			if (entry.IsDir())
				{
				fname->Des().Append(_L("\\"));
				}
			array.AppendL(fname);
			CleanupStack::Pop(fname);
			}
		CleanupStack::PopAndDestroy(dir);
		dirScan->NextL(dir);
		}
	CleanupStack::PopAndDestroy(dirScan);
	}

