//
//  hfs-tests.mm
//  hfs
//
//  Created by Chris Suter on 8/11/15.
//
//

#include <Foundation/Foundation.h>
#include <unordered_map>
#include <string>
#include <list>
#include <unistd.h>
#include <getopt.h>
#include <err.h>
#include <fcntl.h>
#include <sysexits.h>
#include <stdlib.h>
#include <iostream>
#include <mach-o/dyld.h>
#include <sys/param.h>

#include "hfs-tests.h"
#include "test-utils.h"
#include "disk-image.h"
#include "systemx.h"

#define INSTALL_PATH	"/AppleInternal/CoreOS/tests/hfs/hfs-tests"

typedef std::unordered_map<std::string, test_t *> tests_t;
static tests_t *tests;

static std::list<bool (^)(void)> cleanups;

int test_cleanup(bool (^ cleanup)(void))
{
	cleanups.push_front(cleanup);
	return 0;
}

void do_cleanups(void)
{
	bool progress = true;
	int attempts = 0;
	while (progress || (attempts < 2)) {
		size_t i, sz = cleanups.size();
		
		progress = false;
		for (i = 0; i < sz; i++) {
			bool (^cleanup)(void) = cleanups.front();
			cleanups.pop_front();
			if (cleanup() == false)
				cleanups.push_back(cleanup);
			else
				progress = true;
		}
		
		if (!progress)
			attempts++;
		else
			attempts = 0; // reset
	}
}

void register_test(test_t *test)
{
	if (!tests)
		tests = new tests_t;

	tests->insert({ test->name, test });
}

void usage()
{
	printf("hfs-tests [--test <test>|--plist] <list|run>\n");
	exit(EX_USAGE);
}

static int run_test(test_t *test)
{
	int ret;

	@autoreleasepool {
		test_ctx_t ctx = {};

		std::cout << "[TEST] " << test->name << std::endl
			<< "[BEGIN]" << std::endl;

		std::flush(std::cout);

		if (test->run_as_root && geteuid() != 0 && seteuid(0) != 0) {
			std::cout << "error: " << test->name
				<< ": needs to run as root!" << std::endl;
			ret = 1;
		} else {
			if (!test->run_as_root && geteuid() == 0)
				seteuid(501);
			ret = test->test_fn(&ctx);
			fflush(stdout);
		}
	}

	return ret;
}

int main(int argc, char *argv[])
{
	if (!tests)
		tests = new tests_t;

	@autoreleasepool {
		const char *test = NULL;
		bool plist = false, nospawn = false;
		int ch;

		static struct option longopts[] = {
			{ "test",       required_argument,      NULL,           't' },
			{ "plist",		no_argument,			NULL,			'p' },
			{ "no-spawn",	no_argument,			NULL,			'n' }, // private
			{ NULL,         0,                      NULL,           0 }
		};

		while ((ch = getopt_long(argc, argv, "bf:", longopts, NULL)) != -1) {
			switch (ch) {
				case 't':
					test = optarg;
					break;
				case 'p':
					plist = true;
					break;
				case 'n':
					nospawn = true;
					break;
				default:
					usage();
			}
		}
		
		char progname[MAXPATHLEN];
		uint32_t sz = MAXPATHLEN;
		assert(!_NSGetExecutablePath(progname, &sz));
		
		argc -= optind;
		argv += optind;

		if (argc != 1)
			usage();

		int ret = 0;

		if (!strcmp(argv[0], "list")) {
			if (plist) {
				NSMutableArray *cases = [NSMutableArray new];
				
				for (auto it = tests->begin(); it != tests->end(); ++it) {
					NSMutableDictionary *test_case = [@{
					 @"TestName": @(it->first.c_str()),
					 @"Command": @[ @INSTALL_PATH, @"--test",
								   @(it->first.c_str()), @"run"]
												 } mutableCopy];
					
					test_case[@"AsRoot"] = (id)kCFBooleanTrue;

					[cases addObject:test_case];
				}

				std::cout
					<< (char *)[[NSPropertyListSerialization
								 dataWithPropertyList:@{
						 @"Project": @"hfs",
						 @"Tests": cases
						 }
								 format:NSPropertyListXMLFormat_v1_0
								 options:0
								 error:NULL] bytes] << std::endl;
			} else {
				for (auto it = tests->begin(); it != tests->end(); ++it) {
					std::cout << it->first << std::endl;
				}
			}
		} else if (!strcmp(argv[0], "run")) {
			disk_image_t *di = NULL;
			
			if (!nospawn) {
				// Set up the shared disk image
				assert(!systemx("/bin/rm", SYSTEMX_QUIET, "-rf", SHARED_MOUNT, NULL));
				di = disk_image_get();
			}
			
			if (!test) {
				// Run all tests
				for (auto it = tests->begin(); it != tests->end(); ++it) {
					test_t *test = it->second;
					
					int res = systemx(progname, "--test", test->name, "--no-spawn", "run", NULL);
					
					if (res)
						std::cout << "[FAIL] " << test->name << std::endl;
					else
						std::cout << "[PASS] " << test->name << std::endl;
					
					if (!ret)
						ret = res;
				}
			} else {
				auto it = tests->find(test);
				if (it == tests->end()) {
					std::cout << "unknown test: " << test << std::endl;
					ret = EX_USAGE;
				} else {
					if (nospawn) {
						atexit_b(^{
							do_cleanups();
						});
						ret = run_test(it->second);
					}
					else {
						test_t *test = it->second;
						
						ret = systemx(progname, "--test", test->name, "--no-spawn", "run", NULL);
						
						if (ret)
							std::cout << "[FAIL] " << test->name << std::endl;
						else
							std::cout << "[PASS] " << test->name << std::endl;
					}
				}
			}
			
			if (di) {
				// disk_image_cleanup(di) will free di.
				disk_image_cleanup(di);
				systemx("/bin/rm", SYSTEMX_QUIET, "-rf", "/tmp/mnt", NULL);
			}
			
		}

		return ret;
	}
}
