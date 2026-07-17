/*
 * IOUSBLog.h - PureDarwin reconstruction (real Apple header was 0 bytes).
 * USBLog/USBError/USBTrace* are used pervasively throughout the surviving
 * IOUSBFamily .cpp files. Real logging (kprintf-gated), tracing as no-ops -
 * we don't have a kdebug trace-tag registry to match USBTracepoints.h
 * against, and functionality doesn't depend on it.
 */

#ifndef _IOKIT_USB_IOUSBLOG_H
#define _IOKIT_USB_IOUSBLOG_H

#include <IOKit/IOLib.h>

#ifndef IOUSBFAMILY_LOG_LEVEL
#define IOUSBFAMILY_LOG_LEVEL 3
#endif

#define USBLog(LEVEL, FORMAT, ARGS...) \
    do { if ((LEVEL) <= IOUSBFAMILY_LOG_LEVEL) { IOLog(FORMAT "\n", ## ARGS); } } while (0)

#define USBError(LEVEL, FORMAT, ARGS...) \
    do { IOLog(FORMAT "\n", ## ARGS); } while (0)

#define USBTrace(tag, subtag, a, b, c, d) do { } while (0)
#define USBTrace_Start(tag, subtag, a, b, c, d) do { } while (0)
#define USBTrace_End(tag, subtag, a, b, c, d) do { } while (0)

#define USBStringFromReturn(ret) ""

#endif /* !_IOKIT_USB_IOUSBLOG_H */
