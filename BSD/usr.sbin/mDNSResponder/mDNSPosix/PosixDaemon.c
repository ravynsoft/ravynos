/*
 * Copyright (c) 2003-2019 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

    File:		daemon.c

    Contains:	main & associated Application layer for mDNSResponder on Linux.

 */

#if __APPLE__
// In Mac OS X 10.5 and later trying to use the daemon function gives a “‘daemon’ is deprecated”
// error, which prevents compilation because we build with "-Werror".
// Since this is supposed to be portable cross-platform code, we don't care that daemon is
// deprecated on Mac OS X 10.5, so we use this preprocessor trick to eliminate the error message.
#define daemon yes_we_know_that_daemon_is_deprecated_in_os_x_10_5_thankyou
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>

#if __APPLE__
#undef daemon
extern int daemon(int, int);
#endif

#include "mDNSEmbeddedAPI.h"
#include "mDNSPosix.h"
#include "mDNSUNP.h"        // For daemon()
#include "uds_daemon.h"
#include "PlatformCommon.h"
#include "posix_utilities.h"    // For getLocalTimestamp()

#define CONFIG_FILE "/etc/mdnsd.conf"
static domainname DynDNSZone;                // Default wide-area zone for service registration
static domainname DynDNSHostname;

#define RR_CACHE_SIZE 500
static CacheEntity gRRCache[RR_CACHE_SIZE];
static mDNS_PlatformSupport PlatformStorage;

mDNSlocal void mDNS_StatusCallback(mDNS *const m, mStatus result)
{
    (void)m; // Unused
    if (result == mStatus_NoError)
    {
        // On successful registration of dot-local mDNS host name, daemon may want to check if
        // any name conflict and automatic renaming took place, and if so, record the newly negotiated
        // name in persistent storage for next time. It should also inform the user of the name change.
        // On Mac OS X we store the current dot-local mDNS host name in the SCPreferences store,
        // and notify the user with a CFUserNotification.
    }
    else if (result == mStatus_ConfigChanged)
    {
        udsserver_handle_configchange(m);
    }
    else if (result == mStatus_GrowCache)
    {
        // Allocate another chunk of cache storage
        CacheEntity *storage = malloc(sizeof(CacheEntity) * RR_CACHE_SIZE);
        if (storage) mDNS_GrowCache(m, storage, RR_CACHE_SIZE);
    }
}

// %%% Reconfigure() probably belongs in the platform support layer (mDNSPosix.c), not the daemon cde
// -- all client layers running on top of mDNSPosix.c need to handle network configuration changes,
// not only the Unix Domain Socket Daemon

static void Reconfigure(mDNS *m)
{
    mDNSAddr DynDNSIP;
    const mDNSAddr dummy = { mDNSAddrType_IPv4, { { { 1, 1, 1, 1 } } } };;
    mDNS_SetPrimaryInterfaceInfo(m, NULL, NULL, NULL);
    if (ParseDNSServers(m, uDNS_SERVERS_FILE) < 0)
        LogMsg("Unable to parse DNS server list. Unicast DNS-SD unavailable");
    ReadDDNSSettingsFromConfFile(m, CONFIG_FILE, &DynDNSHostname, &DynDNSZone, NULL);
    mDNSPlatformSourceAddrForDest(&DynDNSIP, &dummy);
    if (DynDNSHostname.c[0]) mDNS_AddDynDNSHostName(m, &DynDNSHostname, NULL, NULL);
    if (DynDNSIP.type) mDNS_SetPrimaryInterfaceInfo(m, &DynDNSIP, NULL, NULL);
    mDNS_ConfigChanged(m);
}

// Do appropriate things at startup with command line arguments. Calls exit() if unhappy.
mDNSlocal void ParseCmdLinArgs(int argc, char **argv)
{
    if (argc > 1)
    {
        if (0 == strcmp(argv[1], "-debug")) mDNS_DebugMode = mDNStrue;
        else printf("Usage: %s [-debug]\n", argv[0]);
    }

    if (!mDNS_DebugMode)
    {
        int result = daemon(0, 0);
        if (result != 0) { LogMsg("Could not run as daemon - exiting"); exit(result); }
#if __APPLE__
        LogMsg("The POSIX mdnsd should only be used on OS X for testing - exiting");
        exit(-1);
#endif
    }
}

mDNSlocal void DumpStateLog()
// Dump a little log of what we've been up to.
{
    char timestamp[64]; // 64 is enough to store the UTC timestmp
    
    mDNSu32 major_version = _DNS_SD_H / 10000;
    mDNSu32 minor_version1 = (_DNS_SD_H - major_version * 10000) / 100;
    mDNSu32 minor_version2 = _DNS_SD_H % 100;

    getLocalTimestamp(timestamp, sizeof(timestamp));
    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "---- BEGIN STATE LOG ---- (%s mDNSResponder Build %d.%02d.%02d)", timestamp, major_version, minor_version1, minor_version2);

    udsserver_info_dump_to_fd(STDERR_FILENO);

    getLocalTimestamp(timestamp, sizeof(timestamp));
    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "---- END STATE LOG ---- (%s mDNSResponder Build %d.%02d.%02d)", timestamp, major_version, minor_version1, minor_version2);
}

mDNSlocal mStatus MainLoop(mDNS *m) // Loop until we quit.
{
    sigset_t signals;
    mDNSBool gotData = mDNSfalse;

    mDNSPosixListenForSignalInEventLoop(SIGINT);
    mDNSPosixListenForSignalInEventLoop(SIGTERM);
    mDNSPosixListenForSignalInEventLoop(SIGUSR1);
    mDNSPosixListenForSignalInEventLoop(SIGPIPE);
    mDNSPosixListenForSignalInEventLoop(SIGHUP) ;

    for (; ;)
    {
        // Work out how long we expect to sleep before the next scheduled task
        struct timeval timeout;
        mDNSs32 ticks;

        // Only idle if we didn't find any data the last time around
        if (!gotData)
        {
            mDNSs32 nextTimerEvent = mDNS_Execute(m);
            nextTimerEvent = udsserver_idle(nextTimerEvent);
            ticks = nextTimerEvent - mDNS_TimeNow(m);
            if (ticks < 1) ticks = 1;
        }
        else    // otherwise call EventLoop again with 0 timemout
            ticks = 0;

        timeout.tv_sec = ticks / mDNSPlatformOneSecond;
        timeout.tv_usec = (ticks % mDNSPlatformOneSecond) * 1000000 / mDNSPlatformOneSecond;

        (void) mDNSPosixRunEventLoopOnce(m, &timeout, &signals, &gotData);

        if (sigismember(&signals, SIGHUP )) Reconfigure(m);
        if (sigismember(&signals, SIGUSR1)) DumpStateLog();
        // SIGPIPE happens when we try to write to a dead client; death should be detected soon in request_callback() and cleaned up.
        if (sigismember(&signals, SIGPIPE)) LogMsg("Received SIGPIPE - ignoring");
        if (sigismember(&signals, SIGINT) || sigismember(&signals, SIGTERM)) break;
    }
    return EINTR;
}

int main(int argc, char **argv)
{
    mStatus err;

    ParseCmdLinArgs(argc, argv);

    LogMsg("%s starting", mDNSResponderVersionString);

    err = mDNS_Init(&mDNSStorage, &PlatformStorage, gRRCache, RR_CACHE_SIZE, mDNS_Init_AdvertiseLocalAddresses,
                    mDNS_StatusCallback, mDNS_Init_NoInitCallbackContext);

    if (mStatus_NoError == err)
        err = udsserver_init(mDNSNULL, 0);

    Reconfigure(&mDNSStorage);

    // Now that we're finished with anything privileged, switch over to running as "nobody"
    if (mStatus_NoError == err)
    {
        const struct passwd *pw = getpwnam("nobody");
        if (pw != NULL)
        {
            if (setgid(pw->pw_gid) < 0)
            {
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                          "WARNING: mdnsd continuing as group root because setgid to \"nobody\" failed with " PUB_S, strerror(errno));
            }
            if (setuid(pw->pw_uid) < 0)
            {
                LogMsg("WARNING: mdnsd continuing as root because setuid to \"nobody\" failed with %s", strerror(errno));
            }
        }
        else
        {
            LogMsg("WARNING: mdnsd continuing as root because user \"nobody\" does not exist");
        }
    }

    if (mStatus_NoError == err)
        err = MainLoop(&mDNSStorage);

    LogMsg("%s stopping", mDNSResponderVersionString);

    mDNS_Close(&mDNSStorage);

    if (udsserver_exit() < 0)
        LogMsg("ExitCallback: udsserver_exit failed");

 #if MDNS_DEBUGMSGS > 0
    printf("mDNSResponder exiting normally with %d\n", err);
 #endif

    return err;
}

//		uds_daemon support		////////////////////////////////////////////////////////////

mStatus udsSupportAddFDToEventLoop(int fd, udsEventCallback callback, void *context, void **platform_data)
/* Support routine for uds_daemon.c */
{
    // Depends on the fact that udsEventCallback == mDNSPosixEventCallback
    (void) platform_data;
    return mDNSPosixAddFDToEventLoop(fd, callback, context);
}

int udsSupportReadFD(dnssd_sock_t fd, char *buf, int len, int flags, void *platform_data)
{
    (void) platform_data;
    return recv(fd, buf, len, flags);
}

mStatus udsSupportRemoveFDFromEventLoop(int fd, void *platform_data)        // Note: This also CLOSES the file descriptor
{
    mStatus err = mDNSPosixRemoveFDFromEventLoop(fd);
    (void) platform_data;
    close(fd);
    return err;
}

mDNSexport void RecordUpdatedNiceLabel(mDNSs32 delay)
{
    (void)delay;
    // No-op, for now
}

#if _BUILDING_XCODE_PROJECT_
// If the process crashes, then this string will be magically included in the automatically-generated crash log
const char *__crashreporter_info__ = mDNSResponderVersionString_SCCS + 5;
asm (".desc ___crashreporter_info__, 0x10");
#endif

// For convenience when using the "strings" command, this is the last thing in the file
#if mDNSResponderVersion > 1
mDNSexport const char mDNSResponderVersionString_SCCS[] = "@(#) mDNSResponder-" STRINGIFY(mDNSResponderVersion) " (" __DATE__ " " __TIME__ ")";
#elif MDNS_VERSIONSTR_NODTS
mDNSexport const char mDNSResponderVersionString_SCCS[] = "@(#) mDNSResponder (Engineering Build)";
#else
mDNSexport const char mDNSResponderVersionString_SCCS[] = "@(#) mDNSResponder (Engineering Build) (" __DATE__ " " __TIME__ ")";
#endif
