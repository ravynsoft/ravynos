/**
 *******************************************************************************
 * Copyright (C) 2007,2012 International Business Machines Corporation, Apple Inc.,*
 * and others.  All Rights Reserved.                                           *
 *******************************************************************************
 */

#define __STDC_LIMIT_MACROS 1
#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION && U_PLATFORM_IS_DARWIN_BASED

#include "brkeng.h"
#include "dictbe.h"
#include "aaplbfct.h"
#include "unicode/uscript.h"
#include "unicode/uniset.h"
#include "unicode/ucnv.h"
#include "unicode/uchar.h"
#include <limits.h>
#include <unistd.h>
#include <glob.h>
#include <strings.h>
#include <NSSystemDirectories.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
// The following is now already included by platform.h (included indirectly by
// utypes.h) if U_PLATFORM_IS_DARWIN_BASED but it doesn't hurt to re-include here
#include <TargetConditionals.h>

U_NAMESPACE_BEGIN

/*
 ******************************************************************
 */

AppleLanguageBreakFactory::AppleLanguageBreakFactory(UErrorCode &status)
: ICULanguageBreakFactory(status)
{
}

AppleLanguageBreakFactory::~AppleLanguageBreakFactory() {
}

#if !TARGET_OS_EMBEDDED
#if 0
// need to update loadDictionaryMatcherFor implementation below

// Helper function that makes a length-delimited buffer look NUL-terminated
static __attribute__((always_inline)) inline UChar nextUChar(const UChar *&p, ptrdiff_t &l) {
	if (l > 0) {
		l -= 1;
		return *p++;
	}
	else {
		return 0;
	}
}

// Add a file's worth of words to the supplied mutable dictionary
static void addDictFile(MutableTrieDictionary *to, const char *path) {
	UErrorCode status = U_ZERO_ERROR;
	off_t fileLength;
	const char *dictRawData = (const char *) -1;
	const UChar *dictData = NULL;
	ptrdiff_t dictDataLength = 0;
	UChar *dictBuffer = NULL;
	const char *encoding = NULL;
	int32_t		signatureLength = 0;
	
	// Open the dictionary file
	int dictFile = open(path, O_RDONLY, 0);
	if (dictFile == -1) {
		status = U_FILE_ACCESS_ERROR;
	}
	
	// Determine its length
	if (U_SUCCESS(status)) {
		fileLength = lseek(dictFile, 0, SEEK_END);
		(void) lseek(dictFile, 0, SEEK_SET);
		if (fileLength < 0 || fileLength > PTRDIFF_MAX) {
			status = U_FILE_ACCESS_ERROR;
		}
	}
	
	// Map it
	if (U_SUCCESS(status)) {
		dictRawData = (const char *) mmap(0, (size_t) fileLength, PROT_READ, MAP_SHARED, dictFile, 0);
		if ((intptr_t)dictRawData == -1) {
			status = U_FILE_ACCESS_ERROR;
		}
	}
	
	// No longer need the file descriptor open
	if (dictFile != -1) {
		(void) close(dictFile);
	}
	
	// Look for a Unicode signature
	if (U_SUCCESS(status)) {
		encoding = ucnv_detectUnicodeSignature(dictRawData, fileLength, &signatureLength, &status);
	}
	
	// If necessary, convert the data to UChars
	if (U_SUCCESS(status) && encoding != NULL) {
		UConverter *conv = ucnv_open(encoding, &status);
		// Preflight to get buffer size
		uint32_t destCap = ucnv_toUChars(conv, NULL, 0, dictRawData, fileLength, &status);
		if (status == U_BUFFER_OVERFLOW_ERROR) {
			status = U_ZERO_ERROR;
		}
		if (U_SUCCESS(status)) {
			dictBuffer = new UChar[destCap+1];
		}
		(void) ucnv_toUChars(conv, dictBuffer, destCap+1, dictRawData, fileLength, &status);
		dictData = dictBuffer;
		dictDataLength = destCap;
		if (U_SUCCESS(status) && dictData[0] == 0xFEFF) {	// BOM? Skip it
			dictData += 1;
			dictDataLength -= 1;
		}
		
		ucnv_close(conv);
	}
	
	// If it didn't need converting, just assume it's native-endian UTF-16, no BOM
	if (U_SUCCESS(status) && dictData == NULL) {
		dictData = (const UChar *) dictRawData;
		dictDataLength = fileLength/sizeof(UChar);
	}
	
	// OK, we now have a pointer to native-endian UTF-16. Process it as one word per line,
	// stopping at the first space.
	if (U_SUCCESS(status)) {
		UnicodeSet breaks(UNICODE_STRING_SIMPLE("[[:lb=BK:][:lb=CR:][:lb=LF:][:lb=NL:]]"), status);
		const UChar *candidate = dictData;
		int32_t length = 0;
		UChar uc = nextUChar(dictData, dictDataLength);
		while (U_SUCCESS(status) && uc) {
			while (uc && !u_isspace(uc)) {
				length += 1;
				uc = nextUChar(dictData, dictDataLength);
			}
			
			if (length > 0) {
				to->addWord(candidate, length, status);
			}
			
			// Find beginning of next line
			// 1. Skip non-line-break characters
			while (uc && !breaks.contains(uc)) {
				uc = nextUChar(dictData, dictDataLength);
			}
			// 2. Skip line break characters
			while (uc && breaks.contains(uc)) {
				uc = nextUChar(dictData, dictDataLength);
			}
			
			// Prepare for next line
			candidate = dictData-1;
			length = 0;
		}
	}

	// Unmap the file if we mapped it
	if ((intptr_t) dictRawData != -1) {
		(void) munmap((void *)dictRawData, (size_t) fileLength);
	}
	
	// Delete any temporary buffer
	delete [] dictBuffer;
}

#if U_IS_BIG_ENDIAN
	static const char	sArchType[] = "";
#else
	static const char	sArchType[] = ".le";	// little endian
#endif

#endif
#endif

/*
In ICU50,
ICULanguageBreakFactory changes from 
  virtual const CompactTrieDictionary *loadDictionaryFor(UScriptCode script, int32_t breakType);
to
  virtual DictionaryMatcher *loadDictionaryMatcherFor(UScriptCode script, int32_t breakType);
and CompactTrieDictionary no longer exists. Need to work out  new implementation below.
*/

DictionaryMatcher *
AppleLanguageBreakFactory::loadDictionaryMatcherFor(UScriptCode script) {
	DictionaryMatcher *icuDictMatcher = ICULanguageBreakFactory::loadDictionaryMatcherFor(script);
#if !TARGET_OS_EMBEDDED
#if 0
// need to update loadDictionaryMatcherFor implementation below
	// We only look for a user dictionary if there is actually an ICU dictionary
	if (icuDictMatcher != NULL) {
		UErrorCode status = U_ZERO_ERROR;
		const char *scriptName = uscript_getName(script);
		char path[256];			// PATH_MAX is overkill in this case
		char cachePath[128];
		char cacheTargetPath[256];
		glob_t dirGlob;
		glob_t fileGlob;
		struct stat cacheStat;
		struct stat dictStat;
		bool cacheGood = true;
		int globFlags = (GLOB_NOESCAPE|GLOB_NOSORT|GLOB_TILDE);
		const CompactTrieDictionary *cacheDict = NULL;
		
		// Iterate the dictionary directories and accumulate in dirGlob
		NSSearchPathEnumerationState state = NSStartSearchPathEnumeration(NSLibraryDirectory, (NSSearchPathDomainMask) (NSUserDomainMask|NSLocalDomainMask|NSNetworkDomainMask));
		while ((state = NSGetNextSearchPathEnumeration(state, path)) != 0) {
			// First get the directory itself. We should never overflow, but use strlcat anyway
			// to avoid a crash if we do.
			strlcat(path, "/Dictionaries", sizeof(path));
			if (!glob(path, globFlags, NULL, &dirGlob)) {
				globFlags |= GLOB_APPEND;
			}
		}
		
		// If there are no Dictionaries directories, ignore any cache file and return the ICU
		// standard dictionary
		// TODO: Delete the cache?
		if (dirGlob.gl_pathc == 0) {
			globfree(&dirGlob);
			return icuDictMatcher;
		}
		
		// See if there is a cache file already; get its mod time
		// TODO: should we be using geteuid() here instead of getuid()?
		state = NSStartSearchPathEnumeration(NSCachesDirectory, NSLocalDomainMask);
		state = NSGetNextSearchPathEnumeration(state, cachePath);	// Just use first one
		// Create the cache file name. We should never overflow, but use snprintf to avoid a crash
		// if we do.
		snprintf(cacheTargetPath, sizeof(cacheTargetPath), "%s/com.apple.ICUUserDictionaryCache%s.%s.%d", cachePath, sArchType, scriptName, getuid());
		if (stat(cacheTargetPath, &cacheStat) || cacheStat.st_mode != (S_IFREG|S_IRUSR|S_IWUSR)) {
			cacheGood = false;		// No file or bad permissions or type
		}
		
		// Stat the dictionary folders, and glob the dictionary files
		globFlags &= ~GLOB_APPEND;
		char **pathsp = dirGlob.gl_pathv;
		const char *dictpath;
		while ((dictpath = *pathsp++) != NULL) {
			// Stat the directory -- ignore if stat failure
			if (!stat(dictpath, &dictStat)) {
				// Glob the dictionaries in the directory
				snprintf(path, sizeof(path), "%s/*-%s.txt", dictpath, scriptName);
				if (!glob(path, globFlags, NULL, &fileGlob)) {
					globFlags |= GLOB_APPEND;
				}
				// If the directory has been modified after the cache file, we need to rebuild;
				// a dictionary might have been deleted.
				if (cacheGood && (dictStat.st_mtimespec.tv_sec > cacheStat.st_mtimespec.tv_sec || (dictStat.st_mtimespec.tv_sec == cacheStat.st_mtimespec.tv_sec && dictStat.st_mtimespec.tv_nsec > cacheStat.st_mtimespec.tv_nsec))) {
					cacheGood = false;
				}
			}
		}
		
		// No longer need the directory glob
		globfree(&dirGlob);
		
		// If there are no dictionaries, ignore the cache file and return the ICU dictionary
		// TODO: Delete the cache?
		if (fileGlob.gl_pathc == 0) {
			globfree(&fileGlob);
			return icuDictMatcher;
		}
		
		// Now compare the last modified stamp for the cache against all the dictionaries
		pathsp = fileGlob.gl_pathv;
		while (cacheGood && (dictpath = *pathsp++)) {
			// Stat the dictionary -- ignore if stat failure
			if (!stat(dictpath, &dictStat) && (dictStat.st_mtimespec.tv_sec > cacheStat.st_mtimespec.tv_sec || (dictStat.st_mtimespec.tv_sec == cacheStat.st_mtimespec.tv_sec && dictStat.st_mtimespec.tv_nsec > cacheStat.st_mtimespec.tv_nsec))) {
				cacheGood = false;
			}
		}
		
		// Do we need to build the dictionary cache?
		if (!cacheGood) {
			// Create a mutable dictionary from the ICU dictionary
			MutableTrieDictionary *sum = icuDictMatcher->cloneMutable(status);
			pathsp = fileGlob.gl_pathv;
			while (U_SUCCESS(status) && (dictpath = *pathsp++)) {
				// Add the contents of a file to the sum
				addDictFile(sum, dictpath);
			}
			
			// Create a compact (read-only) dictionary
			CompactTrieDictionary compact(*sum, status);
			delete sum;
			
			if (U_SUCCESS(status)) {
				// Open a temp file to write out the cache
				strlcat(cachePath, "/temp.XXXXXXXXXX", sizeof(cachePath));
				int temp = mkstemp(cachePath);
				if (temp == -1) {
					status = U_FILE_ACCESS_ERROR;
				}
				size_t dictSize = compact.dataSize();
				if (U_SUCCESS(status) && write(temp, compact.data(), dictSize) != dictSize) {
					status = U_FILE_ACCESS_ERROR;
				}
				// Rename the temp file to the cache. Note that race conditions here are
				// fine, as the file system operations are atomic. If an outdated version wins
				// over a newer version, it will get rebuilt at the next app launch due to the
				// modification time checks above. We don't care that any given app launch gets
				// the most up-to-date cache (impossible since we can't lock all the Dictionaries
				// directories), only that the cache (eventually) reflects the current state of
				// any user dictionaries. That will happen on the next app launch after changes
				// to the user dictionaries quiesce.
				if (U_SUCCESS(status)) {
					if (rename(cachePath, cacheTargetPath)) {
						status = U_FILE_ACCESS_ERROR;
						(void) unlink(cachePath);	// Clean up the temp file
					}
				}
				if (temp != -1) {
					close(temp);
				}
			}
		}

		// Done with dictionary paths; release memory allocated by glob()
		globfree(&fileGlob);
		
		// Map the cache and build the dictionary
		if (U_SUCCESS(status)) {
			int cache = open(cacheTargetPath, O_RDONLY, 0);
			off_t length;
			const void *cacheData = (const void *) -1;
			if (cache == -1) {
				status = U_FILE_ACCESS_ERROR;
			}
			if (U_SUCCESS(status)) {
				length = lseek(cache, 0, SEEK_END);
				(void) lseek(cache, 0, SEEK_SET);
				if (length < 0 || length > PTRDIFF_MAX) {
					status = U_FILE_ACCESS_ERROR;
				}
			}
			
			// Map the cache. Note: it is left mapped until process exit. This is the normal
			// behavior anyway, so it shouldn't be an issue.
			if (U_SUCCESS(status)) {
				cacheData = mmap(0, (size_t) length, PROT_READ, MAP_SHARED, cache, 0);
				if ((intptr_t)cacheData == -1) {
					status = U_FILE_ACCESS_ERROR;
				}
			}
			// We can close the cache file now that it's mapped (or not)
			if (cache != -1) {
				(void) close(cache);
			}
			// If all was successful, try to create the dictionary. The constructor will
			// check the magic number for us.
			if (U_SUCCESS(status)) {
				cacheDict = new CompactTrieDictionary(cacheData, status);
			}
			if (U_FAILURE(status) && (intptr_t)cacheData != -1) {
				// Clean up the mmap
				(void) munmap((void *)cacheData, (size_t) length);
			}
		}
		
		// If we were successful, free the ICU dictionary and return ours
		if (U_SUCCESS(status)) {
			delete icuDictMatcher;
			return cacheDict;
		}
		else {
			delete cacheDict;
		}
	}
#endif
#endif
	return icuDictMatcher;
}

U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_BREAK_ITERATION && U_PLATFORM_IS_DARWIN_BASED */
