/*
 * ACPIRTC: minimal PC/AT CMOS RTC driver implementing IORTC.
 *
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 */

#include "ACPIRTC.h"

#include <architecture/i386/pio.h>
#include <IOKit/IOLib.h>

#define super IORTC
OSDefineMetaClassAndStructors(ACPIRTC, IORTC);

enum {
    kRTCIndexPort   = 0x70,
    kRTCDataPort    = 0x71,
    kRTCRegSeconds  = 0x00,
    kRTCRegMinutes  = 0x02,
    kRTCRegHours    = 0x04,
    kRTCRegWeekday  = 0x06,
    kRTCRegDay      = 0x07,
    kRTCRegMonth    = 0x08,
    kRTCRegYear     = 0x09,
    kRTCRegA        = 0x0A,
    kRTCRegB        = 0x0B,
    kRTCRegCentury  = 0x32,
    kRTCRegA_UIP    = 0x80,
    kRTCRegB_24H    = 0x02,
    kRTCRegB_BINARY = 0x04,
    kRTCRegB_AIE    = 0x20,
    kRTCRegB_SET    = 0x80,
    kRTCNMIMask     = 0x80
};

bool
ACPIRTC::start(IOService *provider)
{
    if (!super::start(provider)) {
        return false;
    }

    fLock = IOSimpleLockAlloc();
    if (!fLock) {
        return false;
    }

    fDidLogFirstRead = false;

    // Publish the RTC resource expected by IOKitInitializeTime().
    registerService();
    IOService::publishResource("IORTC", this);
    kprintf("[ACPIRTC] started, published IORTC resource\n");

    return true;
}

void
ACPIRTC::stop(IOService *provider)
{
    if (fLock) {
        IOSimpleLockFree(fLock);
        fLock = NULL;
    }

    super::stop(provider);
}

UInt8
ACPIRTC::readRTCRegister(UInt8 reg)
{
    UInt8 value;
    IOInterruptState state = IOSimpleLockLockDisableInterrupt(fLock);
    outb(kRTCIndexPort, (reg & 0x7f) | kRTCNMIMask);
    value = inb(kRTCDataPort);
    IOSimpleLockUnlockEnableInterrupt(fLock, state);
    return value;
}

void
ACPIRTC::writeRTCRegister(UInt8 reg, UInt8 value)
{
    IOInterruptState state = IOSimpleLockLockDisableInterrupt(fLock);
    outb(kRTCIndexPort, (reg & 0x7f) | kRTCNMIMask);
    outb(kRTCDataPort, value);
    IOSimpleLockUnlockEnableInterrupt(fLock, state);
}

bool
ACPIRTC::waitForUpdateIdle(void)
{
    for (int i = 0; i < 1000; i++) {
        if ((readRTCRegister(kRTCRegA) & kRTCRegA_UIP) == 0) {
            return true;
        }
        IOSleep(1);
    }

    return false;
}

bool
ACPIRTC::readRTCDateTime(UInt8 *sec, UInt8 *min, UInt8 *hour,
                         UInt8 *mday, UInt8 *mon, UInt8 *year,
                         UInt8 *century, UInt8 *statusB)
{
    UInt8 s1, mi1, h1, d1, mo1, y1, c1;
    UInt8 s2, mi2, h2, d2, mo2, y2, c2;

    for (int tries = 0; tries < 4; tries++) {
        if (!waitForUpdateIdle()) {
            return false;
        }

        *statusB = readRTCRegister(kRTCRegB);
        s1 = readRTCRegister(kRTCRegSeconds);
        mi1 = readRTCRegister(kRTCRegMinutes);
        h1 = readRTCRegister(kRTCRegHours);
        d1 = readRTCRegister(kRTCRegDay);
        mo1 = readRTCRegister(kRTCRegMonth);
        y1 = readRTCRegister(kRTCRegYear);
        c1 = readRTCRegister(kRTCRegCentury);

        if (!waitForUpdateIdle()) {
            return false;
        }

        s2 = readRTCRegister(kRTCRegSeconds);
        mi2 = readRTCRegister(kRTCRegMinutes);
        h2 = readRTCRegister(kRTCRegHours);
        d2 = readRTCRegister(kRTCRegDay);
        mo2 = readRTCRegister(kRTCRegMonth);
        y2 = readRTCRegister(kRTCRegYear);
        c2 = readRTCRegister(kRTCRegCentury);

        if (s1 == s2 && mi1 == mi2 && h1 == h2 && d1 == d2 && mo1 == mo2 && y1 == y2 && c1 == c2) {
            *sec = s2;
            *min = mi2;
            *hour = h2;
            *mday = d2;
            *mon = mo2;
            *year = y2;
            *century = c2;
            return true;
        }
    }

    return false;
}

bool
ACPIRTC::isLeapYear(int year)
{
    if ((year % 400) == 0) {
        return true;
    }
    if ((year % 100) == 0) {
        return false;
    }
    return (year % 4) == 0;
}

UInt8
ACPIRTC::bcdToBin(UInt8 x)
{
    return (UInt8)(((x >> 4) * 10) + (x & 0x0f));
}

UInt8
ACPIRTC::binToBcd(UInt8 x)
{
    return (UInt8)(((x / 10) << 4) | (x % 10));
}

uint64_t
ACPIRTC::ymdhmsToEpoch(int year, int mon, int mday, int hour, int min, int sec)
{
    static const int daysBeforeMonth[12] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };

    if (year < 1970 || mon < 1 || mon > 12 || mday < 1 || mday > 31 ||
        hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        return 0;
    }

    uint64_t days = 0;

    for (int y = 1970; y < year; y++) {
        days += isLeapYear(y) ? 366 : 365;
    }

    days += (uint64_t)daysBeforeMonth[mon - 1];
    if (mon > 2 && isLeapYear(year)) {
        days += 1;
    }

    days += (uint64_t)(mday - 1);

    return days * 86400ULL + (uint64_t)hour * 3600ULL + (uint64_t)min * 60ULL + (uint64_t)sec;
}

void
ACPIRTC::epochToYmdhms(uint64_t epoch, int *year, int *mon, int *mday,
                       int *hour, int *min, int *sec)
{
    static const int monthLengths[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    uint64_t days = epoch / 86400ULL;
    uint64_t rem = epoch % 86400ULL;

    *hour = (int)(rem / 3600ULL);
    rem %= 3600ULL;
    *min = (int)(rem / 60ULL);
    *sec = (int)(rem % 60ULL);

    int y = 1970;
    while (true) {
        int ydays = isLeapYear(y) ? 366 : 365;
        if (days < (uint64_t)ydays) {
            break;
        }
        days -= (uint64_t)ydays;
        y++;
    }

    int m = 1;
    for (int i = 0; i < 12; i++) {
        int mdays = monthLengths[i];
        if (i == 1 && isLeapYear(y)) {
            mdays = 29;
        }

        if (days < (uint64_t)mdays) {
            m = i + 1;
            break;
        }

        days -= (uint64_t)mdays;
    }

    *year = y;
    *mon = m;
    *mday = (int)days + 1;
}

long
ACPIRTC::getGMTTimeOfDay(void)
{
    UInt8 sec, min, hour, mday, mon, year, century, statusB;

    if (!readRTCDateTime(&sec, &min, &hour, &mday, &mon, &year, &century, &statusB)) {
        kprintf("[ACPIRTC] getGMTTimeOfDay: RTC read failed\n");
        return 0;
    }

    bool binary = (statusB & kRTCRegB_BINARY) != 0;
    bool mode24 = (statusB & kRTCRegB_24H) != 0;

    if (!binary) {
        sec = bcdToBin(sec);
        min = bcdToBin(min);
        mday = bcdToBin(mday);
        mon = bcdToBin(mon);
        year = bcdToBin(year);
        century = bcdToBin(century);
    }

    int h = hour;
    if (!mode24) {
        int pm = h & 0x80;
        h &= 0x7f;
        if (!binary) {
            h = bcdToBin((UInt8)h);
        }
        if (pm) {
            h = (h % 12) + 12;
        } else if (h == 12) {
            h = 0;
        }
    } else if (!binary) {
        h = bcdToBin((UInt8)h);
    }

    int fullYear;
    if (century >= 19 && century <= 25) {
        fullYear = century * 100 + year;
    } else {
        // Fallback for machines that do not expose a century register.
        fullYear = (year >= 70) ? (1900 + year) : (2000 + year);
    }

    long epoch = (long)ymdhmsToEpoch(fullYear, mon, mday, h, min, sec);

    if (!fDidLogFirstRead) {
        fDidLogFirstRead = true;
        kprintf("[ACPIRTC] first RTC read ok: %04d-%02d-%02d %02d:%02d:%02d UTC (epoch=%ld)\n",
                fullYear, mon, mday, h, min, sec, epoch);
    }

    return epoch;
}

void
ACPIRTC::setGMTTimeOfDay(long secs)
{
    if (secs < 0) {
        secs = 0;
    }

    int year, mon, mday, hour, min, sec;
    epochToYmdhms((uint64_t)secs, &year, &mon, &mday, &hour, &min, &sec);

    UInt8 statusB = readRTCRegister(kRTCRegB);
    bool binary = (statusB & kRTCRegB_BINARY) != 0;
    bool mode24 = (statusB & kRTCRegB_24H) != 0;

    UInt8 rtcSec = (UInt8)sec;
    UInt8 rtcMin = (UInt8)min;
    UInt8 rtcHour;
    UInt8 rtcDay = (UInt8)mday;
    UInt8 rtcMon = (UInt8)mon;
    UInt8 rtcYear = (UInt8)(year % 100);
    UInt8 rtcCentury = (UInt8)(year / 100);

    if (mode24) {
        rtcHour = (UInt8)hour;
    } else {
        bool pm = hour >= 12;
        int h12 = hour % 12;
        if (h12 == 0) {
            h12 = 12;
        }
        rtcHour = (UInt8)(h12 | (pm ? 0x80 : 0));
    }

    if (!binary) {
        rtcSec = binToBcd(rtcSec);
        rtcMin = binToBcd(rtcMin);
        rtcDay = binToBcd(rtcDay);
        rtcMon = binToBcd(rtcMon);
        rtcYear = binToBcd(rtcYear);
        rtcCentury = binToBcd(rtcCentury);

        if (mode24) {
            rtcHour = binToBcd(rtcHour);
        } else {
            UInt8 pmBit = rtcHour & 0x80;
            rtcHour = (UInt8)(binToBcd((UInt8)(rtcHour & 0x7f)) | pmBit);
        }
    }

    writeRTCRegister(kRTCRegB, (UInt8)(statusB | kRTCRegB_SET));
    writeRTCRegister(kRTCRegSeconds, rtcSec);
    writeRTCRegister(kRTCRegMinutes, rtcMin);
    writeRTCRegister(kRTCRegHours, rtcHour);
    writeRTCRegister(kRTCRegDay, rtcDay);
    writeRTCRegister(kRTCRegMonth, rtcMon);
    writeRTCRegister(kRTCRegYear, rtcYear);
    writeRTCRegister(kRTCRegCentury, rtcCentury);
    writeRTCRegister(kRTCRegWeekday, 1);
    writeRTCRegister(kRTCRegB, (UInt8)(statusB & ~kRTCRegB_SET));
}

void
ACPIRTC::setAlarmEnable(IOOptionBits message)
{
    UInt8 statusB = readRTCRegister(kRTCRegB);

    if (message) {
        statusB |= kRTCRegB_AIE;
    } else {
        statusB &= (UInt8)~kRTCRegB_AIE;
    }

    writeRTCRegister(kRTCRegB, statusB);
}


