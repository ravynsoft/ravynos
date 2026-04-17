/*
 * EFINVRAM: minimal EFI-variable backed NVRAM controller.
 */

#ifndef _EFINVRAM_H
#define _EFINVRAM_H

#include <IOKit/IOService.h>
#include <IOKit/IONVRAM.h>
#include <IOKit/IOLocks.h>

class EFINVRAM : public IOService /*IODTNVRAM*/
{
    OSDeclareDefaultStructors(EFINVRAM);

public:
    bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    void stop(IOService *provider) APPLE_KEXT_OVERRIDE;

    IOReturn read(IOByteCount offset, UInt8 *buffer,
                  IOByteCount length);
    IOReturn write(IOByteCount offset, UInt8 *buffer,
                   IOByteCount length);
    void sync(void);

private:
    enum { kNVRAMImageSize = 0x2000 };

    IOLock *fLock;
    UInt8   fImage[kNVRAMImageSize];
    bool    fImageValid;

    IOReturn loadFromEFI(void);
    IOReturn flushToEFI(void);
};

#endif /* _EFINVRAM_H */

