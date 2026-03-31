#include "DNSCommon.h"
#include "unittest_common.h"

mDNSexport void init_logging_ut(void)
{
#if APPLE_OSX_mDNSResponder
	init_logging();

	/* When doing unit testing, it is likely that some local functions and
	 * variables will not be needed to do unit testing validation. So to get
	 * around compiler warnings about unused functions or variables, each
	 * warning work-around is handled explicitly below.
	 */

	/* The next three LogOperation() are used to trick the compiler into
	 * suppressing unused function and variable warnings. This is done by
	 * outputting the function or variable pointer to a log message.
	 */
	LogOperation("Quiet compiler warnings for KQueueLoop= %p, "
		   "KQWokenFlushBytes= %p, SignalCallback= %p, "
		   "mDNS_StatusCallback= %p, LaunchdCheckin= %p",
		   KQueueLoop, KQWokenFlushBytes,
		   SignalCallback, mDNS_StatusCallback,
		   LaunchdCheckin);
	LogOperation("Quiet compiler warnings for SandboxProcess= %p, "
		   "mDNSDaemonInitialize= %p, HandleSIG= %p, "
		   "PreferencesGetValueInt= %p, PreferencesGetValueBool= %p",
		   SandboxProcess, mDNSDaemonInitialize,
		   HandleSIG, PreferencesGetValueInt,
		   PreferencesGetValueBool);
	LogOperation("Quiet compiler warnings for rrcachestorage= %p, "
		   "NoMulticastAdvertisements= %p",
		   rrcachestorage, NoMulticastAdvertisements);

#endif // APPLE_OSX_mDNSResponder
}
