/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*

to build: 
 
cc -g -framework IOKit -framework CoreFoundation IOCFSerializeTest.c -o IOCFSerializeTest

cc -g IOCFSerializeTest.c -framework CoreFoundation /local/build/IOKit/IOKit.build/objects-optimized/IOCFSerialize.o \
 /local/build/IOKit/IOKit.build/objects-optimized/IOCFUnserialize.tab.o -o IOCFSerializeTest

to run: 

./IOCFSerializeTest
find /System/Library/Extensions -name Info.plist  | xargs -n 1 ./IOCFSerializeTest

*/

#include <IOKit/IOCFSerialize.h>
#include <IOKit/IOCFUnserialize.h>
#include <CoreFoundation/CFPropertyList.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>

char *testBuffer =

" <?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"
" <!DOCTYPE plist SYSTEM \"file://localhost/System/Library/DTDs/PropertyList.dtd\"> \n"
" <plist version=\"1.0\"> \n"
" <!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"> \n"
" <plist version=\"0.9\"> \n"
" <!-- this is a comment --> \n"    
" <!-- this is a comment with embedded XML statements \n"
" <key>ignore me</key>	<true/> \n"
" --> \n"
    
" <dict> \n"

" <key>key true</key>	<true/> \n"
" <key>key false</key>	<false/> \n"

" <key>key d0</key>	<data> </data> \n"
" <key>key d1</key>	<data>AQ==</data> \n"
" <key>key d2</key>	<data>ASM=</data> \n"
" <key>key d3</key>	<data>ASNF</data> \n"
" <key>key d4</key>	<data ID=\"1\">ASNFZw==</data> \n"

" <key>key i0</key>	<integer></integer> \n"
" <key>key i1</key>	<integer>123456789</integer> \n"
" <key>key i2</key>	<integer>-123456789</integer> \n"
" <key>key i3</key>	<integer size=\"32\" ID=\"2\">0x12345678</integer> \n"

" <key>key s0</key>	<string></string> \n"
" <key>key s1</key>	<string>string 1</string> \n"
" <key>key s2</key>	<string ID=\"3\">string 2</string> \n"
" <key>key mr ©</key>	<string>mac roman copyright ©</string> \n"
" <key>key uft8 \xc2\xa9</key>	<string>utf-8 copyright \xc2\xa9</string> \n"
" <key>key &lt;&amp;&gt;</key>	<string>&lt;&amp;&gt;</string> \n"

" <key>key D0</key>	<dict ID=\"4\"> \n"
"                        </dict> \n"

" <key>key a0</key>	<array> \n"
"                        </array> \n"

" <key>key a1</key>	<array ID=\"5\"> \n"
"                            <string>array string 1</string> \n"
"                            <string>array string 2</string> \n"
"                        </array> \n"

" <key>key r1</key>	<ref IDREF=\"1\"/> \n"
" <key>key r2</key>	<ref IDREF=\"2\"/> \n"
" <key>key r3</key>	<ref IDREF=\"3\"/> \n"
" <key>key r4</key>	<ref IDREF=\"4\"/> \n"
" <key>key r5</key>	<ref IDREF=\"5\"/> \n"

" <key>key e1</key>	<array/> \n"
" <key>key e2</key>	<dict/> \n"
" <key>key e4</key>	<integer/> \n"
" <key>key e5</key>	<string/> \n"
" <key>key e6</key>	<data/> \n"

// CFPropertyListCreateXMLData() can't handle sets
#if 0
" <key>key S0</key>	<set> \n"
"                        </set> \n"
" <key>key S1</key>	<set ID=\"6\"> \n"
"                             <string>set string 1</string> \n"
"                             <string>set string 2</string> \n"
"                         </set> \n"
" <key>key r6</key>	<ref IDREF=\"6\"/> \n"
" <key>key e3</key>	<set/> \n"
#endif
" </dict> \n"
" </plist> \n"
;


int
main(int argc, char **argv)
{
	CFTypeRef	properties0, properties1, properties2, properties3, properties4, properties5;
	CFDataRef	data1, data2, data3, data4;
	CFStringRef  	errorString;
	int i, j;
	int fd = 0;
	char * bufPtr;
	struct stat sb;
	size_t size;
	int usingFile = 0;

	if (argc == 2) {

		if (stat(argv[1], &sb)) exit(1);
		size = (size_t)sb.st_size;
		    
		printf("checking file %s, file size %ld\n", argv[1], size);

		bufPtr = (char *)malloc(size);
		if (!bufPtr) exit(1);

		fd = open(argv[1], O_RDONLY | O_NDELAY, 0);
		if (fd <= 0) exit(1);

		if ((read(fd, bufPtr, size) != size) || errno) exit(1);

		testBuffer = bufPtr;

		usingFile = 1;
	    
	} else {

		printf("running self check testing...\n");
	    
		// random error injection testing, this is painfully slow
#if 1
		char * randomBuffer = (char *)strdup(testBuffer);
		int randomBufferSize = strlen(randomBuffer);

		for (i=0; i < randomBufferSize; i++) { 

			for (j=0; j < 256; j++) { 

				randomBuffer[i] = (char)j;
				
//				printf("testBuffer[%d] = %c, randomBuffer[%d] = %c\n", i, testBuffer[i], i, randomBuffer[i]);

				properties0 = IOCFUnserialize(randomBuffer, kCFAllocatorDefault, 0, &errorString);
				if (properties0) {
					CFRelease(properties0);
					if (errorString) {
						printf("random testing failed - errorString is set\n");
						printf("testBuffer[%d] = %c, randomBuffer[%d] = %c\n", i, testBuffer[i], i, randomBuffer[i]);
						exit(1);
					}
				} else {
					if (errorString) {
						CFRelease(errorString);
					} else {
						printf("random testing failed - errorString is null\n");
						printf("testBuffer[%d] = %c, randomBuffer[%d] = %c\n", i, testBuffer[i], i, randomBuffer[i]);
						exit(1);
					}
				}
			}
			randomBuffer[i] = testBuffer[i];
		}
		if (!usingFile) printf("random syntax error testing successful\n");
#endif		
	}


	// unserialize test buffer and then re-serialize it


	properties1 = IOCFUnserialize(testBuffer, kCFAllocatorDefault, 0, &errorString);
	if (!properties1) {
		CFIndex bufSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(errorString),
		       kCFStringEncodingUTF8) + sizeof('\0');
		char *buffer = malloc(bufSize);
		if (!buffer || !CFStringGetCString(errorString, buffer, bufSize, 
						   kCFStringEncodingUTF8)) {
			exit(1);
		}

		printf("prop1 error: %s\n", buffer);
		CFRelease(errorString);
		exit(1);
	}

	data1 = IOCFSerialize(properties1, kNilOptions);
	if (!data1) {
		printf("serialize on prop1 failed\n");
		exit(1);
	}

	
	// unserialize again, using the previous re-serialization compare resulting objects


	properties2 = IOCFUnserialize((char *)CFDataGetBytePtr(data1), kCFAllocatorDefault, 0, &errorString);
	if (!properties2) {
		CFIndex bufSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(errorString),
		       kCFStringEncodingUTF8) + sizeof('\0');
		char *buffer = malloc(bufSize);
		if (!buffer || !CFStringGetCString(errorString, buffer, bufSize, kCFStringEncodingUTF8)) {
			exit(1);
		}

		printf("prop2 error: %s\n", buffer);
		CFRelease(errorString);
		exit(1);
	}

	if (CFEqual(properties1, properties2)) {
		if (!usingFile) printf("test successful, prop1 == prop2\n");
	} else {
		printf("test failed, prop1 == prop2\n");
//		printf("%s\n", testBuffer);
//		printf("%s\n", (char *)CFDataGetBytePtr(data1));
		exit(1);
	}

	data2 = IOCFSerialize(properties2, kNilOptions);
	if (!data2) {
		printf("serialize on prop2 failed\n");
		exit(1);
	}

	// unserialize again, using the previous re-serialization compare resulting objects

	properties3 = IOCFUnserialize((char *)CFDataGetBytePtr(data2), kCFAllocatorDefault, 0, &errorString);
	if (!properties3) {
		CFIndex bufSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(errorString),
		       kCFStringEncodingUTF8) + sizeof('\0');
		char *buffer = malloc(bufSize);
		if (!buffer || !CFStringGetCString(errorString, buffer, bufSize, kCFStringEncodingUTF8)) {
		    exit(1);
		}

		printf("prop3 error: %s\n", buffer);
		CFRelease(errorString);
		exit(1);
	}

	if (CFEqual(properties2, properties3)) {
		if (!usingFile) printf("test successful, prop2 == prop3\n");
	} else {
		printf("test failed, prop2 == prop3\n");
//		printf("%s\n", (char *)CFDataGetBytePtr(data1));
//		printf("%s\n", (char *)CFDataGetBytePtr(data2));
		exit(1);
	}

	// re-serialize using CF serializer, unserialize again and compare resulting objects

	data3 = CFPropertyListCreateXMLData(NULL, properties3);
	if (!data3) {
		printf("serialize on prop3 failed\n");
		exit(1);
	}

	properties4 = IOCFUnserialize((char *)CFDataGetBytePtr(data3), kCFAllocatorDefault, 0, &errorString);
	if (!properties4) {
		CFIndex bufSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(errorString),
		       kCFStringEncodingUTF8) + sizeof('\0');
		char *buffer = malloc(bufSize);
		if (!buffer || !CFStringGetCString(errorString, buffer, bufSize, kCFStringEncodingUTF8)) {
		    exit(1);
		}

		printf("prop4 error: %s\n", buffer);
//		printf("%s\n", (char *)CFDataGetBytePtr(data3));
		CFRelease(errorString);
		exit(1);
	}

	if (CFEqual(properties3, properties4)) {
		if (!usingFile) printf("test successful, prop3 == prop4\n");
	} else {
		printf("test failed, prop3 == prop4\n");
//		printf("%s\n", (char *)CFDataGetBytePtr(data2));
//		printf("%s\n", (char *)CFDataGetBytePtr(data3));
		exit(1);
	}

	// unserialize test buffer using CF and compare objects

	if (usingFile) {

		data4 = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, testBuffer, size, kCFAllocatorNull);
		if (!data4) {
			printf("serialize on prop 4 failed\n");
			exit(1);
		}

		properties5 = CFPropertyListCreateFromXMLData(kCFAllocatorDefault, data4, kCFPropertyListImmutable, &errorString);
		if (!properties5) {
			CFIndex bufSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(errorString),
			       kCFStringEncodingUTF8) + sizeof('\0');
			char *buffer = malloc(bufSize);
			if (!buffer || !CFStringGetCString(errorString, buffer, bufSize, kCFStringEncodingUTF8)) {
			    exit(1);
			}

			printf("prop5 error: %s\n", buffer);
			CFRelease(errorString);
			exit(1);
		}

		if (CFEqual(properties1, properties5)) {
			if (!usingFile) printf("test successful, prop1 == prop5\n");
		} else {
			printf("test failed, prop3 == prop4\n");
			exit(1);
		}
	}

	CFRelease(data1);
	CFRelease(data2);
	CFRelease(data3);
	CFRelease(properties1);
	CFRelease(properties2);
	CFRelease(properties3);
	CFRelease(properties4);
	if (usingFile) {
		CFRelease(data4);
		CFRelease(properties5);
	}
	return 0;
}
