#include <darwintest.h>

#include <IOKit/IOKitLib.h>

T_DECL(IOMasterPort,
       "check if one can retrieve mach port for communicating with IOKit",
       T_META_NAMESPACE("IOKitUser.IOKitLib")
       )
{
    mach_port_t masterPort = MACH_PORT_NULL;

    T_EXPECT_MACH_SUCCESS(IOMasterPort(MACH_PORT_NULL, &masterPort), NULL);
    T_EXPECT_NE(MACH_PORT_NULL, masterPort, NULL);
}
