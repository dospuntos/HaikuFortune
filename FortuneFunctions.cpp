#include "FortuneFunctions.h"

#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <OS.h>
#include <Path.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize the global path to a hardcoded value just in case.
// This happens to be different under Haiku than under previous versions
// of BeOS
BString gFortunePath = "/boot/system/data/fortunes";

FortuneAccess::FortuneAccess(void)
{
}

FortuneAccess::FortuneAccess(const char *folder)
{
	SetFolder(folder);	
}

FortuneAccess::~FortuneAccess(void)
{
	// Free al items in our list
	MakeEmpty();
}

status_t
FortuneAccess::SetFolder(const char *folder)
{
	// Make sure that folder is valid and return B_BAD_VALUE if it isn't.
	BEntry entry(folder);
	if (!entry.Exists() && !entry.IsDirectory())
	{
		printf("Folder not found\n");
		return B_BAD_VALUE;	
	}
	
	// Set the path variable, scan the folder and return B_OK	
	fPath = folder;
	ScanFolder();
	
	return B_OK;
}

status_t
FortuneAccess::GetFortune(BString &target)
{
	// Here's the meat of this class:
	// 1) Return B_NO_INIT if fPath is empty
	if (fPath.CountChars() == 0)
	{
		return B_NO_INIT;	
	}
	
	// 2) Return B_ERROR if the ref list is empty	
	if (fRefList.CountItems() < 1)
	{
		return B_ERROR;
	}
		
	// 3) This line will randomly choose the index of a file in the ref list
	int32 index = int32(float(rand()) / RAND_MAX * fRefList.CountItems());
	
	// 4) Get a pointer to the randomly-selected entry_ref
	entry_ref *ref = static_cast<entry_ref*>(fRefList.ItemAt(index));
	
	// 5) Create and initialize a BFile object in read-only mode
	BFile file(ref, B_READ_ONLY);
	
	// 6) Check to make sure that the BFile's status is B_OK
	if (file.InitCheck() != B_OK)
		return file.InitCheck();
		
	// 7) Set fLastFile to the name property of the ref we just got
	fLastFile = ref->name;
	
	// 8) Get the file's size.
	off_t size;
	file.GetSize(&size);
	
	// 9) If the file is empty, return B_ERROR.
	if (size < 1)
		return B_ERROR;
	
	// 10) Create a BString to hold the data in the file
	BString data;
	
	// 11) Create a char pointer that we'll use in BFile::Read.
	char *buffer = data.LockBuffer(size + 10);
	
	// 12) Initialize the pointer using BString::LockBuffer, passing the file's
	//		size + 10 bytes (for safety) as the size. LockBuffer temporarily gives
	//		you access to the BString's internal char array. We'll need this to
	//		be able to read the file's data into the BString.
	
	// 13) Use BFile::Read() to read the entire file using our new char pointer.
	file.Read(buffer, size);
	
	// 14) Call BString::UnlockBuffer() to invalidate our char pointer and
	// 		allow us to use regular BString methods again.
	data.UnlockBuffer();
	buffer = NULL;
	
	// 15) Use a loop to manually count the number of record separators in the
	//		fortune file. The separator is the string "%\n", so use a
	//		combination of BString::FindFirst and offsets in a loop to count them.
	int32 entrycount = 0;
	int32 entrystart = 0;
	do
	{
		entrystart = data.FindFirst("%\n", entrystart + 1);
		entrycount++;
	} while (entrystart > 0);
	
	// 16) Use this line to randomly choose an entry.
	int32 entry = int32(float(rand()) / RAND_MAX * (entrycount - 1));
	
	// 17) Use FindFirst again to find the starting offset of this
	//		randomly-chosen entry in the file.
	
	entrystart = 0;
	for (int32 i = 0; i < entry; i++)
	{
		entrystart = data.FindFirst("%\n", entrystart + 1);
	}
	
	// 18) Call FindFirst one last time to find the offset of the next separator
	//		so we know how long the fortune is.
	// 19) Create a BString to hold the fortune.
	// 20) Set this new BString to the String() method plus the starting offset
	// 		of the BString holding the file data. This will effectively chop out
	//		everything that is before our fortune in the file. It should look
	//		something like this:
	//		BString fortune = filedata.String() + startingOffset;
	
	BString entrydata;
	entrydata = data.String() + entrystart + 2;
	int32 entrylength = entrydata.FindFirst("%\n");
	// 21) Chop off everything after our fortune in the fortune BString by
	//		calling its Truncate() method.
	//		Hint: length = endingOffset - startingOffset + 2
	
	if (entrylength > 0)
	{
		entrydata.Truncate(entrylength);
	}
	// 22) Set the parameter 'target' to our fortune data and return B_OK
	target = entrydata;
	return B_OK;
}

void
FortuneAccess::ScanFolder(void)
{
	// Use a BDirectory for this. Make sure that it is initialized from fPath
	// properly. Emtpy the ref list so that we're not adding to an existing
	// list. Use BDirectory::GetNextEntry to get the entry for each file in the
	// folder. Use the BEntry to check to make sure that the entry is a file,
	// and, assuming so, make a new entry_ref, send it to BEntry::getRef,
	// and add it to our ref list.
	BDirectory dir(fPath.String());
	if (dir.InitCheck() != B_OK) 
	{
		return;
	}
		
	MakeEmpty();
	entry_ref ref;
	while (dir.GetNextRef(&ref) == B_OK)
	{
		BEntry entry(&ref);
		if (entry.IsFile())
		{
			fRefList.AddItem(new entry_ref(ref));
		}
	}
}

void
FortuneAccess::MakeEmpty(void)
{
	// Iterate through the ref list and delete each entry_ref. After doing
	// this, call BList::MakeEmpty().
	for (int32 i = 0; i < fRefList.CountItems(); i++)
	{
		entry_ref *ref = (entry_ref*)fRefList.ItemAt(i);
		delete ref;
	}
	//fRefList.MakeEmtpy();
}

int32
FortuneAccess::CountFiles(void) const
{
	return fRefList.CountItems();
}

status_t
FortuneAccess::LastFilename(BString &target)
{
	// Return B_NO_INIT if the path variable is empty
	// Set the target parameter to our fLastFile property and return B_OK
	if (fPath.CountChars() == 0)
	{
		return B_NO_INIT;	
	}
	
	target = fLastFile;
	
	return B_OK;
}
