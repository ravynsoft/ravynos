#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define PLIST_PATH "/System/Library/CoreServices/SystemVersion.plist"
#define BUFSIZE 256

char* getValue(const void* plist, CFStringRef key)
{
	const void* value = NULL;
	char        valbuf[BUFSIZE];

	if (!CFDictionaryGetValueIfPresent(
			plist,
			key,
			&value)) {
		fprintf(stderr, "system plist is missing keys\n");
		return NULL;
	}

	if (!value)
		return "(null)";

	if (!CFStringGetCString((CFStringRef)value, 
				valbuf, 
				BUFSIZE,
				kCFStringEncodingUTF8)) {
		fprintf(stderr, "failed to convert value\n");
		return NULL;
	}

	return strdup(valbuf);
}

int main(int argc, char* argv[])
{
	struct stat       stbuf;
	int               fd;
	void*             map = NULL;
	CFDataRef         xmldata = NULL;
	CFPropertyListRef plist = NULL;
	int               rc = 1;
	char*             prodName = NULL;
	char*             prodVer = NULL;
	char*             prodBuild = NULL;


	fd = open(PLIST_PATH, O_RDONLY);
	if (fd < 0) {
		perror(PLIST_PATH);
		return rc;
	}

	if (fstat(fd, &stbuf) < 0) {
		perror("stat");
		close(fd);
		return rc;
	}

	map = mmap(0, stbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (map == NULL) {
		perror("mmap");
		return rc;
	}

	xmldata = CFDataCreateWithBytesNoCopy(NULL,
	                                      map,
					      stbuf.st_size,
					      kCFAllocatorNull);
	if (!xmldata) {
		fprintf(stderr, "failed to create CFData container\n");
		goto out;
	}

	CFIndex format;
	plist = CFPropertyListCreateWithData(
		NULL,    /* allocator */
		xmldata, /* data */
		kCFPropertyListImmutable, /* options */
		&format, /* format out */
		NULL);   /* error out */
	if (!plist || !CFPropertyListIsValid(plist, format)) {
		fprintf(stderr, "failed to parse plist\n");
		goto out;
	}

	prodName = getValue(plist, CFSTR("ProductName"));
	prodVer = getValue(plist, CFSTR("ProductVersion"));
	prodBuild = getValue(plist, CFSTR("ProductBuildVersion"));

	if (argc > 1) {
		if (!strcmp(argv[1], "-productName")) {
			printf("%s\n", prodName);
		}
		else if (!strncmp(argv[1], "-productVersion", 15)) {
			printf("%s\n", prodVer);
		}
		else if (!strcmp(argv[1], "-buildVersion")) {
			printf("%s\n", prodBuild);
		}
	}
	else {

		/* no options: print all */
		printf("ProductName: %s\nProductVersion: %s\nBuildVersion: %s\n\n",
			prodName, prodVer, prodBuild);
	}

	rc = 0;

out:
	if (plist) CFRelease(plist);
	if (xmldata) CFRelease(xmldata);
	if (map) munmap(map, stbuf.st_size);
	if (prodName) free((void*)prodName);
	if (prodVer) free((void*)prodVer);
	if (prodBuild) free((void*)prodBuild);

	return rc;
}

