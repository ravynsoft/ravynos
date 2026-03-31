#include <stdint.h>
#include <OpenDirectory/OpenDirectory.h>
#include <DirectoryService/DirectoryService.h>
#include <unistd.h>
#include <pthread.h>

#define test_assume(test, obj) \
	({ \
		if (!obj) { \
			printf(test " - FAIL\n"); \
		} else { \
			printf(test " - PASS\n"); \
		} \
		obj; \
	})

#define test_assume_and_release(test, obj) \
	{ \
		if (test_assume(test, obj)) { \
			CFRelease(obj); \
			obj = NULL; \
		} \
	}

uint32_t    gTestCase       = 0;
bool		gLogErrors      = false;
bool		gVerbose        = false;
char        *gLogPath       = NULL;
char        *gAdminAccount  = NULL;
char        *gAdminPassword = NULL;
char        *gProxyHost     = NULL;
char        *gProxyPassword = NULL;
char        *gProxyUser     = NULL;
uint32_t    gTestTimes      = 1;
uint32_t    gCopies         = 1;

void usage(char *argv[])
{
	fprintf(stderr, "Usage:  TestAppCF -a adminPass -p adminPass [-v] [-e] [-l path] [-N runNumber]\n", argv[0]);
	fprintf(stderr, "                 [-c copies] [-t times] [-u proxyUser -P proxyPass -h proxyHost]\n");
	fprintf(stderr, "             -v  verbose output\n");
	fprintf(stderr, "             -e  output error log\n");
	fprintf(stderr, "             -l  log path (default \"./Logs/<runNumber>/\"\n");
	fprintf(stderr, "             -N  test run number\n");
	fprintf(stderr, "             -c  number of copies to fork for stress testing\n");
	fprintf(stderr, "             -t  number of times to run the test\n");
	fprintf(stderr, "             -u  Proxy username\n");
	fprintf(stderr, "             -P  Proxy username password\n");
	fprintf(stderr, "             -h  Proxy host\n");
	fprintf(stderr, "             -a  local admin account\n");
	fprintf(stderr, "             -p  local admin password\n");
}

void parseOptions(int argc, char *argv[])
{
	int                 ch;

	gTestCase = (uint32_t) CFAbsoluteTimeGetCurrent();

	while ((ch = getopt(argc, argv, "vel:N:c:t:u:P:h:a:p:")) != -1) {
		switch (ch) {
			case 'v':
				gVerbose = true;
				break;
			case 'e':
				gLogErrors = true;
				break;
			case 'l':
				gLogPath = optarg;
				break;
			case 'N':
				gTestCase = strtol(optarg, NULL, 10);
				break;
			case 'c':
				if (NULL != optarg) {
					gCopies = strtol(optarg, NULL, 10);
					if (gCopies > 1024) {
						gCopies = 1024;
					}
				} else {
					usage(argv);
				}
				break;
			case 't':
				if (NULL != optarg) {
					gTestTimes = strtol(optarg, NULL, 10);
					if (0 == gTestTimes) {
						gTestTimes = 1;
					}
				} else {
					usage(argv);
				}
				break;
			case 'u':   // proxy user
				gProxyUser = optarg;
				break;
			case 'P':   // proxy password
				gProxyPassword = optarg;
				break;
			case 'h':   // host
				gProxyHost = optarg;
				break;
			case 'a':   // admin
				gAdminAccount = optarg;
				break;
			case 'p':   // local admin password
				gAdminPassword = optarg;
				break;
			case '?':
			default:
				usage(argv);
				exit(1);
		}
	}

	if (NULL == gAdminAccount || NULL == gAdminPassword) {
		usage(argv);
		exit(1);
	} else if (NULL != gProxyHost || NULL != gProxyUser || NULL != gProxyHost) {
		if (NULL == gProxyHost || NULL == gProxyUser || NULL == gProxyHost) {
			usage(argv);
			exit(1);
		}
	}
}

void searchCallback(ODContextRef inContext, CFMutableArrayRef inResults, CFErrorRef inResultCode)
{
	if (NULL != inResults) {
		printf("Got %d results\n", (int) CFArrayGetCount(inResults));
	} else {
		printf("Got no more results\n");
	}
}

bool doTestMembership(ODNodeRef inNodeRef, ODRecordType inGroupType, ODRecordType inMemberType)
{
	CFErrorRef  error           = NULL;
	ODRecordRef testmember      = NULL;
	ODRecordRef testcontainer   = NULL;
	bool        bReturn         = false;

	CFArrayRef emptyValue = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	bool (^testFailure)(bool, const char *, CFErrorRef *) = ^(bool bSuccess, const char *testCase, CFErrorRef *inError) {
		bool bFailed = true;

		CFErrorRef theError = (*inError);
		if (theError != NULL) {
			if (bSuccess == false) {
				printf("%s - FAIL", testCase);
			} else {
				printf("%s - FAIL (error but status says success)", testCase);
			}

			CFStringRef errorDesc = CFErrorCopyDescription(theError);
			const char *errorStr = "no error details";
			char buffer[512];

			if (errorDesc != NULL) {
				CFStringGetCString(errorDesc, buffer, sizeof(buffer), kCFStringEncodingUTF8);
				errorStr = buffer;
			}

			printf(" - (%d) - %s\n", (int) CFErrorGetCode(theError), errorStr);
		} else {
			if (bSuccess == false) {
				printf("%s - FAIL - failed but no Error\n", testCase);
			} else {
				printf("%s - SUCCESS\n", testCase);
				bFailed = false;
			}
		}

		return bFailed;
	};

	char groupType[64];
	char memberType[64];

	CFStringGetCString(inGroupType, groupType, sizeof(groupType), kCFStringEncodingUTF8);

	if (inMemberType != NULL) {
		CFStringGetCString(inMemberType, memberType, sizeof(memberType), kCFStringEncodingUTF8);

		printf("\n-- Container '%s' - Member '%s'\n", groupType, memberType);

		testmember = ODNodeCreateRecord(inNodeRef, inMemberType, CFSTR("testmember"), NULL, &error);
		if (testFailure(testmember != NULL, "ODNodeCreateRecord 'testmember'", &error) == true) {
			goto fail;
		}
	} else {
		printf("\n-- Container '%s' - Member 'NULL'\n", groupType);
	}

	testcontainer = ODNodeCreateRecord(inNodeRef, inGroupType, CFSTR("testcontainer"), NULL, &error);
	if (testFailure(testcontainer != NULL, "ODNodeCreateRecord 'testcontainer'", &error) == true) {
		goto fail;
	}

	if (inMemberType == NULL) {
		if (ODRecordAddMember(testcontainer, NULL, &error) == true) {
			printf("ODRecordAddMember (add NULL member) - FAIL (succeeded)\n");
		} else {
			printf("ODRecordAddMember (add NULL member) - SUCCESS (got error)\n");
		}
		goto finish;
	}

	bool (^badMixture)(void) = ^(void) {
		if ((inGroupType == kODRecordTypeGroups && (inMemberType == kODRecordTypeUsers || inMemberType == kODRecordTypeGroups)) ||
		        (inGroupType == kODRecordTypeComputerGroups && (inMemberType == kODRecordTypeComputers || inMemberType == kODRecordTypeComputerGroups)) ||
		(inGroupType == kODRecordTypeComputerLists && (inMemberType == kODRecordTypeComputers || inMemberType == kODRecordTypeComputerLists))) {
			return (bool) false;
		}

		return (bool) true;
	};

	bool (^addMember)(const char *) = ^(const char *testCase) {
		char errorStr[256];
		CFErrorRef localErr = NULL;

		snprintf(errorStr, sizeof(errorStr), "ODRecordAddMember (%s)", testCase);

		if (ODRecordAddMember(testcontainer, testmember, &localErr) == false) {
			// need to check types, may expected failure
			if (badMixture() == false) {
				if (testFailure(false, errorStr, &localErr) == true) {
					return (bool) false;
				}
			} else {
				printf("%s - SUCCESS (got error)\n", errorStr);
				CFRelease(localErr);
				localErr = NULL;
			}
		} else {
			printf("%s - SUCCESS\n", errorStr);
		}

		return (bool) true;
	};

	//
	// add member with valid UUID
	if (addMember("has UUID") == false) {
		goto fail;
	}

	//
	// add member without UUID nor UID
	if (testFailure(ODRecordSetValue(testmember, kODAttributeTypeGUID, emptyValue, &error),
	                "ODRecordSetValue testmember (remove GUID)", &error) == true) {
		goto fail;
	}

	// can only do this test if we are users cause group-in-group requires UUID or GID
	if (inMemberType == kODRecordTypeUsers && addMember("no UUID nor UID/GID, if applicable") == false) {
		goto fail;
	}

	ODAttributeType idAttr = NULL;

	if (inMemberType == kODRecordTypeUsers) {
		idAttr = kODAttributeTypeUniqueID;
	} else if (inMemberType == kODRecordTypeGroups) {
		idAttr = kODAttributeTypePrimaryGroupID;
	}

	if (idAttr != NULL) {
		//
		// set ID so we have a synthetic UUID
		if (testFailure(ODRecordSetValue(testmember, idAttr, CFSTR("199"), &error),
		                "ODRecordSetValue testmember (set UID)", &error) == true) {
			goto fail;
		}

		if (addMember("synthetic UUID") == false) {
			goto fail;
		}
	}

	ODAttributeType mbrAttrib   = NULL;
	ODAttributeType clearAttrib = NULL;

	if (inMemberType == kODRecordTypeUsers || inMemberType == kODRecordTypeComputers) {
		if (inGroupType == kODRecordTypeComputerLists) {
			mbrAttrib = kODAttributeTypeComputers;
		} else {
			mbrAttrib = kODAttributeTypeGroupMembers;
		}
	} else if (inMemberType == kODRecordTypeGroups || inMemberType == kODRecordTypeComputerGroups) {
		mbrAttrib = kODAttributeTypeNestedGroups;
	} else if (inMemberType == kODRecordTypeComputerLists) {
		mbrAttrib = kODAttributeTypeGroup;
	} else {
		goto finish;
	}

	if (testFailure(ODRecordSetValue(testmember, kODAttributeTypeGUID, CFSTR("12345678-1234-1234-123456789011"), &error),
	                "ODRecordSetValue testmember (put a UUID back)", &error) == true) {
		goto fail;
	}

	// now add to fake GUIDs
	CFTypeRef values[] = { CFSTR("12345678-1234-1234-123456789012"), CFSTR("12345678-1234-1234-123456789013"), CFSTR("12345678-1234-1234-123456789014") };
	CFArrayRef cfGUIDList = CFArrayCreate(kCFAllocatorDefault, values, sizeof(values) / sizeof(CFTypeRef), &kCFTypeArrayCallBacks);
	if (testFailure(ODRecordSetValue(testcontainer, mbrAttrib, cfGUIDList, &error),
	                "ODRecordSetValue testcontainer (adding fake values)", &error) == true) {
		goto fail;
	}

	if (inGroupType == kODRecordTypeGroups &&
	        inMemberType == kODRecordTypeUsers &&
	        testFailure(ODRecordSetValue(testcontainer, kODAttributeTypeGroupMembership, emptyValue, &error),
	                    "ODRecordSetValue testcontainer (removing name list)", &error) == true) {
		goto fail;
	}

	if (addMember("adding member (missing legacy membership, if applicable)") == false) {
		goto fail;
	} else if (badMixture() == true) {
		printf("Stopping test because incompatible mix to do addition validation\n");
		goto finish;
	}


	CFArrayRef groupMembers = ODRecordCopyValues(testcontainer, mbrAttrib, &error);
	if (testFailure(groupMembers != NULL, "ODRecordCopyValues testcontainer", &error) == true) {
		goto fail;
	}

	// should have 4 members now
	CFIndex count = CFArrayGetCount(groupMembers);
	printf("ODRecordCopyValues testcontainer (expected 4 got %d) - %s\n", (int) count, (count == 4 ? "SUCCESS" : "FAIL"));
	if (count != 4) {
		goto fail;
	}

	if (addMember("adding member (twice)") == false) {
		goto fail;
	}

	groupMembers = ODRecordCopyValues(testcontainer, mbrAttrib, &error);
	if (testFailure(groupMembers != NULL, "ODRecordCopyValues testcontainer", &error) == true) {
		goto fail;
	}

	// should still have 4 members
	count = CFArrayGetCount(groupMembers);
	printf("ODRecordCopyValues testcontainer (expected 4 got %d) - %s\n", (int) count, (count == 4 ? "SUCCESS" : "FAIL"));
	if (count != 4) {
		goto fail;
	}

	// TODO: do we need to care about the name membership count?

finish:

	bReturn = true;

fail:

	ODRecordDelete(testmember, NULL);
	ODRecordDelete(testcontainer, NULL);

	return bReturn;
};

void *doTest(void *inData)
{
	ODSessionRef        cfRef               = NULL;
	ODContextRef        cfContext           = 0;
	ODQueryRef          cfQuery             = NULL;
	CFArrayRef          cfResults           = NULL;
	CFStringRef         cfProxyHost         = NULL;
	CFStringRef         cfProxyUser         = NULL;
	CFStringRef         cfProxyPassword     = NULL;
	int                 ii;
	CFErrorRef          cfError;
	ODNodeRef           cfNodeRef           = NULL;
	CFStringRef         cfRecordName        = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("TestCFFramework%ld"), (long) pthread_self());
	CFStringRef         cfGroupName         = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("TestGroup%ld"), (long) pthread_self());
	CFStringRef         cfAddRecordName     = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("TestAddRecord%ld"), (long) pthread_self());
	CFStringRef         cfAddRecordAlias    = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("TestAddRecordAlias%ld"), (long) pthread_self());
	ODNodeRef           cfLocalNodeRef		= NULL;

	if (NULL != gProxyHost) {
		cfProxyHost     = CFStringCreateWithCString(kCFAllocatorDefault, gProxyHost, kCFStringEncodingUTF8);
		cfProxyUser     = CFStringCreateWithCString(kCFAllocatorDefault, gProxyUser, kCFStringEncodingUTF8);
		cfProxyPassword = CFStringCreateWithCString(kCFAllocatorDefault, gProxyPassword, kCFStringEncodingUTF8);
	}

	CFStringRef cfAdminAccount  = CFStringCreateWithCString(kCFAllocatorDefault, gAdminAccount, kCFStringEncodingUTF8);
	CFStringRef cfAdminPassword = CFStringCreateWithCString(kCFAllocatorDefault, gAdminPassword, kCFStringEncodingUTF8);

	for (ii = 0; ii < gTestTimes; ii++) {
		// test all of the APIs
		if (NULL != gProxyHost) {
			CFMutableDictionaryRef  cfOptions   = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
			                                                                &kCFTypeDictionaryValueCallBacks);

			CFDictionarySetValue(cfOptions, kODSessionProxyAddress, cfProxyHost);
			CFDictionarySetValue(cfOptions, kODSessionProxyUsername, cfProxyUser);
			CFDictionarySetValue(cfOptions, kODSessionProxyPassword, cfProxyPassword);

			cfRef = ODSessionCreate(kCFAllocatorDefault, cfOptions, NULL);
			if (NULL != cfRef) {
				printf("ODSessionCreate (with options) proxy - PASS\n");

				CFRelease(cfRef);
				cfRef = NULL;
			} else {
				printf("ODSessionCreate (with options) proxy - FAIL\n");
			}

			CFRelease(cfOptions);
		} else {
			printf("ODSessionCreate (with options) proxy - SKIP\n");
		}

		cfRef = ODSessionCreate(kCFAllocatorDefault, NULL, NULL);
		test_assume_and_release("ODSessionCreate", cfRef);

		cfResults = ODSessionCopyNodeNames(NULL, NULL, NULL);
		test_assume_and_release("ODSessionCopyNodeNames", cfResults);

		cfNodeRef = ODNodeCreateWithNodeType(kCFAllocatorDefault, kODSessionDefault, kODNodeTypeAuthentication, &cfError);
		if (NULL != cfNodeRef) {
			printf("ODNodeCreate - PASS\n");

			CFArrayRef  cfNodes = ODNodeCopySubnodeNames(cfNodeRef, NULL);
			if (NULL != cfNodes) {
				printf("ODNodeCopySubnodeNames (Auth search node) - PASS\n");

				CFArrayRef  cfUnreachable = ODNodeCopyUnreachableSubnodeNames(cfNodeRef, NULL);
				if (cfUnreachable != NULL) {
					printf("ODNodeCopyUnreachableSubnodeNames - FAIL - returned %d\n", (int) CFArrayGetCount(cfUnreachable));

					CFRelease(cfUnreachable);
					cfUnreachable = NULL;
				} else {
					printf("ODNodeCopyUnreachableSubnodeNames - PASS\n");
				}

				CFRelease(cfNodes);
				cfNodes = NULL;
			} else {
				printf("ODNodeCopySubnodeNames (Auth search node) - FAIL\n");
			}

			CFRelease(cfNodeRef);
			cfNodeRef = NULL;
		} else {
			printf("ODNodeCreateWithNodeType with kODTypeAuthenticationSearchNode - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
			CFRelease(cfError);
		}

		cfNodeRef = ODNodeCreateWithName(kCFAllocatorDefault, kODSessionDefault, CFSTR("/LDAPv3/odsnowleo.apple.com"), &cfError);
		if (NULL != cfNodeRef) {
			printf("ODNodeCreateWithName with /LDAPv3/odsnowleo.apple.com - PASS\n");

			ODNodeRef cfNodeRef2 = ODNodeCreateCopy(kCFAllocatorDefault, cfNodeRef, NULL);
			if (NULL != cfNodeRef2) {
				printf("ODNodeCreateCopy - PASS\n");
				CFRelease(cfNodeRef2);
				cfNodeRef2 = NULL;
			} else {
				printf("ODNodeCreateCopy - FAIL\n");
			}

			CFRelease(cfNodeRef);
			cfNodeRef = NULL;
		} else {
			printf("ODNodeCreateWithName with /LDAPv3/odsnowleo.apple.com - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
			CFRelease(cfError);
		}

		cfLocalNodeRef = ODNodeCreateWithNodeType(kCFAllocatorDefault, kODSessionDefault, kODNodeTypeLocalNodes, &cfError);
		if (NULL != cfLocalNodeRef) {
			CFTypeRef   cfValues[] = { cfAdminAccount };
			CFArrayRef cfSearchValue = CFArrayCreate(kCFAllocatorDefault, cfValues, 1, &kCFTypeArrayCallBacks);

			printf("ODNodeCreateWithNodeType with kODTypeLocalNode - PASS\n");

			cfQuery = ODQueryCreateWithNode(kCFAllocatorDefault, cfLocalNodeRef, CFSTR(kDSStdRecordTypeUsers), CFSTR(kDSNAttrRecordName),
			                                kODMatchEqualTo, cfSearchValue, NULL, 0, NULL);
			CFRelease(cfSearchValue);

			if (NULL != cfQuery) {
				printf("ODQueryCreateWithNode - PASS\n");

				cfResults = ODQueryCopyResults(cfQuery, false, NULL);

				if (NULL != cfResults) {
					printf("ODQueryCopyResults returned non-NULL - PASS\n");

					if (CFArrayGetCount(cfResults) != 0) {
						printf("ODQueryCopyResults returned results (%d) - PASS\n", (int) CFArrayGetCount(cfResults));
					} else {
						printf("ODQueryCopyResults returned results - FAIL\n");
					}

					CFRelease(cfResults);
					cfResults = NULL;
				} else {
					printf("ODQueryCopyResults returned NULL - FAIL\n");
				}

				CFRelease(cfQuery);
				cfQuery = NULL;
			} else {
				printf("ODQueryCreateWithNode - FAIL\n");
			}

			ODRecordRef cfRecord = ODNodeCopyRecord(cfLocalNodeRef, CFSTR(kDSStdRecordTypeUsers), cfAdminAccount, NULL, NULL);
			if (NULL != cfRecord) {
				printf("ODNodeCopyRecord - PASS\n");

				CFDictionaryRef cfPolicy = ODRecordCopyPasswordPolicy(kCFAllocatorDefault, cfRecord, NULL);
				if (NULL != cfPolicy) {
					printf("ODRecordCopyPasswordPolicy - PASS\n");

					CFRelease(cfPolicy);
					cfPolicy = NULL;
				} else {
					printf("ODRecordCopyPasswordPolicy - FAIL\n");
				}

				if (ODRecordVerifyPassword(cfRecord, cfAdminPassword, NULL) == true) {
					printf("ODRecordVerifyPassword - PASS\n");
				} else {
					printf("ODRecordVerifyPassword - FAIL\n");
				}

				CFStringRef     cfAttribute = CFSTR(kDSAttributesAll);
				CFArrayRef      cfAttribs   = CFArrayCreate(kCFAllocatorDefault, (const void **) &cfAttribute, 1, &kCFTypeArrayCallBacks);
				CFDictionaryRef cfValues    = ODRecordCopyDetails(cfRecord, cfAttribs, NULL);
				CFRelease(cfAttribs);
				if (NULL != cfValues) {
					printf("ODRecordCopyDetails (kDSAttributesAll) - non-NULL - PASS\n");

					if (CFDictionaryGetCount(cfValues) != 0) {
						printf("ODRecordCopyDetails (kDSAttributesAll) - has values - PASS\n");
					} else {
						printf("ODRecordCopyDetails (kDSAttributesAll) - has values - FAIL\n");
					}

					CFRelease(cfValues);
					cfValues = NULL;
				} else {
					printf("ODRecordCopyDetails - FAIL\n");
				}

				CFArrayRef cfAttribValues = ODRecordCopyValues(cfRecord, CFSTR(kDS1AttrNFSHomeDirectory), NULL);
				if (NULL != cfAttribValues) {
					printf("ODRecordCopyValues - PASS\n");

					CFRelease(cfAttribValues);
					cfAttribValues = NULL;
				} else {
					printf("ODRecordCopyValues - FAIL\n");
				}

//#warning needs ODNodeAuthenticateExtended
//                dsStatus = ODNodeAuthenticateExtended( dsNodeRef, CFSTR(kDSStdRecordTypeUsers), CFSTR(), inAuthItems,
//                                                                    &outAuthItems, &dsContext );
//                if( eDSNoErr == dsStatus )
//                {
//                    printf( "ODNodeAuthenticateExtended - PASS\n" );
//                }
//                else
//                {
//                    printf( "ODNodeAuthenticateExtended - FAIL\n" );
//                }

				if (ODRecordSetNodeCredentials(cfRecord, cfAdminAccount, cfAdminPassword, NULL)) {
					printf("ODRecordSetNodeCredentials - PASS\n");
				} else {
					printf("ODRecordSetNodeCredentials - FAIL\n");
				}
				CFRelease(cfRecord);
			} else {
				printf("ODNodeCopyRecord - FAIL\n");
			}

			CFStringRef cfNodeName = ODNodeGetName(cfLocalNodeRef);
			if (NULL != cfNodeName) {
				printf("ODNodeGetName - PASS\n");
			} else {
				printf("ODNodeGetName - FAIL\n");
			}

			CFDictionaryRef cfNodeInfo = ODNodeCopyDetails(cfLocalNodeRef, NULL, NULL);
			if (NULL != cfNodeInfo) {
				printf("ODNodeCopyDetails - PASS\n");

				CFRelease(cfNodeInfo);
				cfNodeInfo = NULL;
			} else {
				printf("ODNodeCopyDetails - FAIL\n");
			}

			CFArrayRef cfRecTypes = ODNodeCopySupportedRecordTypes(cfLocalNodeRef, NULL);
			if (NULL != cfRecTypes) {
				printf("ODNodeCopySupportedRecordTypes - PASS\n");

				CFRelease(cfRecTypes);
				cfRecTypes = NULL;
			} else {
				printf("ODNodeCopySupportedRecordTypes - FAIL\n");
			}

			CFArrayRef cfAttribTypes = ODNodeCopySupportedAttributes(cfLocalNodeRef, NULL, NULL);
			if (NULL != cfAttribTypes) {
				printf("ODNodeCopySupportedAttributes - PASS\n");

				CFRelease(cfAttribTypes);
				cfAttribTypes = NULL;
			} else {
				printf("ODNodeCopySupportedAttributes - FAIL\n");
			}

			if (ODNodeSetCredentials(cfLocalNodeRef, CFSTR(kDSStdRecordTypeUsers), cfAdminAccount, cfAdminPassword, NULL)) {
				ODRecordRef    cfRecord = NULL;

				printf("ODNodeSetCredentials - PASS\n");

				cfRecord = ODNodeCreateRecord(cfLocalNodeRef, CFSTR(kDSStdRecordTypeUsers), cfRecordName, NULL, NULL);
				if (NULL != cfRecord) {
					printf("ODNodeCreateRecord - PASS\n");

					ODRecordRef cfRecordTemp = ODNodeCreateRecord(cfLocalNodeRef, CFSTR(kDSStdRecordTypeUsers), cfRecordName, NULL, &cfError);
					if (NULL != cfRecordTemp) {
						printf("ODNodeCreateRecord (create duplicate) - FAIL record returned\n");
					} else {
						CFIndex errCode = CFErrorGetCode(cfError);
						if (errCode == kODErrorRecordAlreadyExists) {
							printf("ODNodeCreateRecord (create duplicate) - PASS\n");
						} else {
							printf("ODNodeCreateRecord (create duplicate) - FAIL (%d)\n", (int) errCode);
							CFRelease(cfError);
						}
					}

					if (ODRecordDelete(cfRecord, &cfError)) {
						printf("ODRecordDelete - PASS\n");
					} else {
						printf("ODRecordDelete - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
					}

					CFRelease(cfRecord);
					cfRecord = NULL;
				} else {
					printf("ODNodeCreateRecord - FAIL\n");
				}

				CFMutableDictionaryRef cfAttributes = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
				                                                                &kCFTypeDictionaryValueCallBacks);
				CFStringRef     cfHome[]    = { CFSTR("/Users/blah"), NULL };
				CFStringRef     cfNames[]   = { cfAddRecordName, cfAddRecordAlias, NULL };

				CFArrayRef      cfHomeAttrib    = CFArrayCreate(kCFAllocatorDefault, (const void **) &cfHome, 1, &kCFTypeArrayCallBacks);
				CFArrayRef      cfNamesAttrib   = CFArrayCreate(kCFAllocatorDefault, (const void **) &cfNames, 2, &kCFTypeArrayCallBacks);

				CFDictionarySetValue(cfAttributes, CFSTR(kDS1AttrNFSHomeDirectory), cfHomeAttrib);
				CFDictionarySetValue(cfAttributes, CFSTR(kDSNAttrRecordName), cfNamesAttrib);

				CFRelease(cfNamesAttrib);
				CFRelease(cfHomeAttrib);

				cfRecord = ODNodeCreateRecord(cfLocalNodeRef, CFSTR(kDSStdRecordTypeUsers), cfAddRecordName, cfAttributes, &cfError);
				if (NULL != cfRecord) {
					printf("ODNodeCreateRecord (with attributes) - PASS\n");

					if (NULL != cfRecord) {
						printf("ODNodeCreateRecord (record returned) - PASS\n");

						CFArrayRef cfValue = ODRecordCopyValues(cfRecord, CFSTR(kDS1AttrNFSHomeDirectory), NULL);
						if (NULL != cfValue && CFArrayGetCount(cfValue) == 1) {
							printf("ODRecordCopyValues (value exist) - PASS\n");
						} else {
							printf("ODRecordCopyValues (value exist) - FAIL\n");
						}

						if (NULL != cfValue) {
							CFRelease(cfValue);
						}

						if (ODRecordChangePassword(cfRecord, NULL, CFSTR("test"), &cfError)) {
							printf("ODRecordChangePassword - PASS\n");
						} else {
							printf("ODRecordChangePassword - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
							CFRelease(cfError);
						}

						if (ODRecordChangePassword(cfRecord, CFSTR("test"), CFSTR("test2"), &cfError)) {
							printf("ODRecordChangePassword - PASS\n");
						} else {
							printf("ODRecordChangePassword - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
							CFRelease(cfError);
						}

						if (ODRecordSetValue(cfRecord, CFSTR(kDSNAttrState), CFSTR("Test street"), &cfError)) {
							printf("ODRecordSetValue - PASS\n");
						} else {
							printf("ODRecordSetValue - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
							CFRelease(cfError);
						}

						if (ODRecordAddValue(cfRecord, CFSTR(kDSNAttrState), CFSTR("Test street 2"), &cfError)) {
							printf("ODRecordAddValue - PASS\n");
						} else {
							printf("ODRecordAddValue - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
							CFRelease(cfError);
						}

						if (ODRecordRemoveValue(cfRecord, CFSTR(kDSNAttrState), CFSTR("Test street 2"), &cfError)) {
							printf("ODRecordRemoveValue - PASS\n");
						} else {
							printf("ODRecordRemoveValue - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
						}

						CFArrayRef  cfArray = CFArrayCreate(kCFAllocatorDefault, NULL, 0, &kCFTypeArrayCallBacks);
						if (ODRecordSetValue(cfRecord, CFSTR(kDSNAttrState), cfArray, &cfError)) {
							printf("ODRecordSetValue (remove attribute) - PASS\n");

							CFArrayRef cfValues = ODRecordCopyValues(cfRecord, CFSTR(kDSNAttrState), NULL);
							if (NULL == cfValues || CFArrayGetCount(cfValues) == 0) {
								printf("ODRecordSetValue (attrib gone) - PASS\n");
							} else {
								printf("ODRecordSetValue (attrib gone) - FAIL\n");
							}

							if (NULL != cfValues) {
								CFRelease(cfValues);
							}
						} else {
							printf("ODRecordSetValue (deleting attrib) - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
						}

						CFRelease(cfArray);

						ODRecordDelete(cfRecord, NULL);
					} else {
						printf("ODNodeAddRecordWithAttributes (record returned) - FAIL\n");
					}

					CFRelease(cfRecord);
					cfRecord = NULL;
				} else {
					printf("ODNodeCreateRecord (with attributes) - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
					CFRelease(cfError);
				}

				CFRelease(cfAttributes);
				cfAttributes = NULL;

				// test various combinations
				ODRecordType groupTypes[] = { kODRecordTypeConfiguration, kODRecordTypeUsers, kODRecordTypeGroups, kODRecordTypeComputerGroups, kODRecordTypeComputerLists };
				ODRecordType userTypes[] = { NULL, kODRecordTypeUsers, kODRecordTypeComputers, kODRecordTypeGroups,
				                             kODRecordTypeComputerGroups, kODRecordTypeComputerLists, kODRecordTypeConfiguration
				                           };

				int groupX;
				int userX;
				bool bSuccess = true;
				for (groupX = 0; bSuccess == true && groupX < (sizeof(groupTypes) / sizeof(ODRecordType)); groupX++) {
					for (userX = 0; bSuccess == true && userX < (sizeof(userTypes) / sizeof(ODRecordType)); userX++) {
						bSuccess = doTestMembership(cfLocalNodeRef, groupTypes[groupX], userTypes[userX]);
					}
				}

				if (bSuccess == false) {
					goto fail;
				}

				// now test membership checks
				ODRecordRef cfGroup = ODNodeCopyRecord(cfLocalNodeRef, kODRecordTypeGroups, CFSTR("admin"), NULL, NULL);
				if (NULL != cfGroup) {
					printf("\n--Test Memberships\n");
					printf("ODNodeCopyRecord (admin) - PASS\n");

					cfRecord = ODNodeCopyRecord(cfLocalNodeRef, kODRecordTypeUsers, cfAdminAccount, NULL, NULL);

					if (ODRecordContainsMember(cfGroup, cfRecord, NULL) == true) {
						printf("ODRecordContainsMember - PASS\n");
					} else {
						printf("ODRecordContainsMember - FAIL\n");
					}

					// test failure - group in user
					if (ODRecordContainsMember(cfRecord, cfGroup, NULL) == false) {
						printf("ODRecordContainsMember (group is member of user (failure)) - PASS (got error)\n");
					} else {
						printf("ODRecordContainsMember (group is member of user (failure)) - FAIL (no error)\n");
					}

					// test failure - group in group
					if (ODRecordContainsMember(cfRecord, cfGroup, NULL) == false) {
						printf("ODRecordContainsMember (group is member of group (failure)) - PASS (got error)\n");
					} else {
						printf("ODRecordContainsMember (group is member of group (failure)) - FAIL (no error)\n");
					}

					CFRelease(cfGroup);
					cfGroup = NULL;
				} else {
					printf("ODNodeCopyRecord (admin) - FAIL\n");
				}
			} else {
				printf("ODNodeSetCredentials - FAIL\n");
			}

			CFRelease(cfLocalNodeRef);
			cfLocalNodeRef = NULL;
		} else {
			printf("ODNodeCreateWithNodeType with kODTypeLocalNode - FAIL (%d)\n", (int) CFErrorGetCode(cfError));
			CFRelease(cfError);
		}
	}

fail:
	if (gProxyHost) {
		CFRelease(cfProxyPassword);
		CFRelease(cfProxyHost);
		CFRelease(cfProxyUser);
	}
	CFRelease(cfAdminAccount);
	CFRelease(cfAdminPassword);
	CFRelease(cfRecordName);
	CFRelease(cfGroupName);
	CFRelease(cfAddRecordName);
	CFRelease(cfAddRecordAlias);

	return NULL;
}

int main(int argc, char *argv[])
{
	parseOptions(argc, argv);

	ODNodeRef   cfLocalNodeRef = ODNodeCreateWithNodeType(kCFAllocatorDefault, kODSessionDefault, kODNodeTypeLocalNodes, NULL);

	if (gCopies > 0) {

		int         ii;
		void        *junk;
		pthread_t   *threads    = (pthread_t *) calloc(gCopies, sizeof(pthread_t));


		for (ii = 0; ii < gCopies; ii++) {
			while (pthread_create(&threads[ii], NULL, doTest, cfLocalNodeRef) != 0) {
				usleep(1000);
			}
		}

		// now wait for each thread to finish
		for (ii = 0; ii < gCopies; ii++) {
			pthread_join(threads[ii], &junk);
		}

	} else {
		doTest(cfLocalNodeRef);
	}

	return 0;
}
