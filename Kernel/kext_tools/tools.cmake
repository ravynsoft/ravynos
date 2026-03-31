Resolving paths for sources
add_library(kextcache STATIC
	./FILES/kextcache_main.c
	./staging.m
	./FILES/update_boot.c
	./syspolicy.m
	./compression.c
	./bootcaches.c
	./driverkit.m
	./FILES/mkext1_file.c
	./safecalls.c
	./rosp_staging.m
	./kext_tools_util.c
	./signposts.m
	./fork_program.c
	./kernelcache.c
	./security.c
)
add_library(kextd STATIC
	./pgo.c
	./staging.m
	./kextd_main.c
	./kextd_mig_server.c
	./syspolicy.m
	./kextaudit.c
	./kextd_personalities.c
	./kextd_request.c
	./kextd_serialize_kextload.c
	./kextd_watchvol.c
	./kext_tools_util.c
	./bootcaches.c
	./fork_program.c
	./driverkit.m
	./safecalls.c
	./kextmanager.defs
	./kextmanager_async.defs
	./signposts.m
	./kextd_mach.defs
	./kextd_usernotification.c
	./security.c
)
add_library(kextstat STATIC
	./signposts.m
	./kextstat_main.c
	./kext_tools_util.c
)
add_library(kextunload STATIC
	./signposts.m
	./kextunload_main.c
	./kext_tools_util.c
)
add_library(mkextunpack STATIC
	./signposts.m
	./mkextunpack_main.c
	./compression.c
	./kext_tools_util.c
)
add_library(kextfind STATIC
	./signposts.m
	./kextfind_main.c
	./QEQuery.c
	./kextfind_query.c
	./kextfind_commands.c
	./kextfind_tables.c
	./kextfind_report.c
	./kext_tools_util.c
)
add_library(libBootRoot STATIC
	./signposts.m
	./bootcaches.c
	./fork_program.c
	./kext_tools_util.c
	./safecalls.c
	./FILES/update_boot.c
)
add_library(brtest_standalone STATIC
	./brtest.c
)
add_library(kcgen STATIC
	./signposts.m
	./kcgen_main.c
	./kext_tools_util.c
	./compression.c
	./kernelcache.c
)
add_library(kclist STATIC
	./signposts.m
	./kclist_main.c
	./compression.c
	./kernelcache.c
	./kext_tools_util.c
)
add_library(kextload STATIC
	./driverkit.m
	./security.c
	./staging.m
	./syspolicy.m
	./kextload_main.c
	./kext_tools_util.c
	./signposts.m
	./kextaudit.c
)
add_library(kextlibs STATIC
	./signposts.m
	./kextlibs_main.c
	./kext_tools_util.c
)
add_library(kextutil STATIC
	./driverkit.m
	./signposts.m
	./kextutil_main.c
	./staging.m
	./syspolicy.m
	./kext_tools_util.c
	./security.c
	./kextaudit.c
)
add_library(kextaudit_test STATIC
	./driverkit.m
	./signposts.m
	./syspolicy.m
	./staging.m
	./kext_tools_util.c
	./kextaudit.c
	./security.c
	./tests/kextaudit_test.m
)
add_library(KextAudit STATIC
	./KextAudit/KextAudit.cpp
	./KextAudit/KextAuditUserClient.cpp
)
add_library(kextaudit_darwintest STATIC
	./driverkit.m
	./darwintests/special_darwintests/kextaudit_darwintest.m
	./signposts.m
	./syspolicy.m
	./staging.m
	./kext_tools_util.c
	./kextaudit.c
	./security.c
)
add_library(kcditto STATIC
	./fork_program.c
	./safecalls.c
	./kext_tools_util.c
	./bootcaches.c
	./kcditto_main.m
	./signposts.m
	./rosp_staging.m
)
add_library("kcgen-embedded-host" STATIC
	./signposts.m
	./kcgen_main.c
	./kext_tools_util.c
	./compression.c
	./kernelcache.c
)
add_library("kclist-embedded-host" STATIC
	./signposts.m
	./kclist_main.c
	./compression.c
	./kernelcache.c
	./kext_tools_util.c
)
add_library("kctool-embedded-host" STATIC
	./signposts.m
	./kctool_main.c
	./compression.c
	./kernelcache.c
	./kext_tools_util.c
)
add_library(logkextloadsd STATIC
	 logkextloadsd_main.c
)
add_library(security_test STATIC
	./driverkit.m
	./signposts.m
	./tests/security_test.m
	./staging.m
	./syspolicy.m
	./kext_tools_util.c
	./security.c
)
