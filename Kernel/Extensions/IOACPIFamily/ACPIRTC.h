/*
 * ACPIRTC: minimal PC/AT CMOS RTC driver implementing IORTC.
 *
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 */

#ifndef _ACPIRTC_H
#define _ACPIRTC_H

#include <IOKit/rtc/IORTCController.h>
#include <IOKit/IOLocks.h>

class ACPIRTC : public IORTC
{
    OSDeclareDefaultStructors(ACPIRTC);

public:
    bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    void stop(IOService *provider) APPLE_KEXT_OVERRIDE;

    long getGMTTimeOfDay(void) APPLE_KEXT_OVERRIDE;
    void setGMTTimeOfDay(long secs) APPLE_KEXT_OVERRIDE;
    void setAlarmEnable(IOOptionBits message) APPLE_KEXT_OVERRIDE;

private:
    IOSimpleLock *fLock;
    bool          fDidLogFirstRead;

    UInt8 readRTCRegister(UInt8 reg);
    void writeRTCRegister(UInt8 reg, UInt8 value);
    bool waitForUpdateIdle(void);
    bool readRTCDateTime(UInt8 *sec, UInt8 *min, UInt8 *hour,
                         UInt8 *mday, UInt8 *mon, UInt8 *year,
                         UInt8 *century, UInt8 *statusB);

    static bool isLeapYear(int year);
    static UInt8 bcdToBin(UInt8 x);
    static UInt8 binToBcd(UInt8 x);
    static uint64_t ymdhmsToEpoch(int year, int mon, int mday,
                                  int hour, int min, int sec);
    static void epochToYmdhms(uint64_t epoch, int *year, int *mon, int *mday,
                              int *hour, int *min, int *sec);
};

#endif /* _ACPIRTC_H */

