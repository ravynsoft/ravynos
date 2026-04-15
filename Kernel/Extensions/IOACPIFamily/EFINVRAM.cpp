/*
 * EFINVRAM: minimal EFI-variable backed NVRAM controller.
 */

#include "EFINVRAM.h"

#include <IOKit/IOLib.h>

extern "C" {
#include <pexpert/pexpert.h>
#include <pexpert/i386/boot.h>
#include <pexpert/i386/efi.h>
#include <i386/pal_routines.h>
}

#define super IOService
OSDefineMetaClassAndStructors(EFINVRAM, IOService /*IODTNVRAM*/);

extern void *gPEEFIRuntimeServices;

static const EFI_GUID kEFINVRAMVendorGuid = APPLE_VENDOR_GUID;
static EFI_CHAR16 kEFINVRAMVarName[] = {
    'r','a','v','y','n','O','S','-','N','V','R','A','M', 0
};

static inline void
prepareStackArg5(uint64_t *stackWords, uint64_t arg5)
{
    /* x64 EFI ABI: 32-byte shadow space, then 5th argument. */
    bzero(stackWords, sizeof(uint64_t) * 6);
    stackWords[4] = arg5;
}

bool
EFINVRAM::start(IOService *provider)
{
    if (!super::start(provider)) {
        return false;
    }

    fLock = IOLockAlloc();
    if (!fLock) {
        return false;
    }

    bzero(fImage, sizeof(fImage));
    fImageValid = false;

    (void)loadFromEFI();

    registerService();
    IOService::publishResource("IONVRAM", this);
    kprintf("[EFINVRAM] started\n");

    return true;
}

void
EFINVRAM::stop(IOService *provider)
{
    if (fLock) {
        IOLockFree(fLock);
        fLock = NULL;
    }

    super::stop(provider);
}

IOReturn
EFINVRAM::read(IOByteCount offset, UInt8 *buffer, IOByteCount length)
{
    if (!buffer || (offset + length) > kNVRAMImageSize) {
        return kIOReturnBadArgument;
    }

    IOLockLock(fLock);
    if (!fImageValid) {
        (void)loadFromEFI();
    }
    bcopy(fImage + offset, buffer, length);
    IOLockUnlock(fLock);

    return kIOReturnSuccess;
}

IOReturn
EFINVRAM::write(IOByteCount offset, UInt8 *buffer, IOByteCount length)
{
    if (!buffer || (offset + length) > kNVRAMImageSize) {
        return kIOReturnBadArgument;
    }

    IOLockLock(fLock);
    if (!fImageValid) {
        (void)loadFromEFI();
    }

    bcopy(buffer, fImage + offset, length);
    fImageValid = true;

    IOReturn ret = flushToEFI();
    IOLockUnlock(fLock);

    return ret;
}

void
EFINVRAM::sync(void)
{
    IOLockLock(fLock);
    if (fImageValid) {
        (void)flushToEFI();
    }
    IOLockUnlock(fLock);
}

IOReturn
EFINVRAM::loadFromEFI(void)
{
    EFI_RUNTIME_SERVICES_64 *rt = (EFI_RUNTIME_SERVICES_64 *)gPEEFIRuntimeServices;
    struct pal_efi_registers regs;
    uint64_t stackWords[6];
    uint64_t efiStatus = 0;
    uint32_t attrs = 0;
    uint64_t dataSize = kNVRAMImageSize;
    kern_return_t kr;

    if (!rt || !rt->GetVariable) {
        return kIOReturnNotReady;
    }

    regs.rcx = (uint64_t)(uintptr_t)kEFINVRAMVarName;
    regs.rdx = (uint64_t)(uintptr_t)&kEFINVRAMVendorGuid;
    regs.r8  = (uint64_t)(uintptr_t)&attrs;
    regs.r9  = (uint64_t)(uintptr_t)&dataSize;
    regs.rax = 0;

    prepareStackArg5(stackWords, (uint64_t)(uintptr_t)fImage);

    kr = pal_efi_call_in_64bit_mode((uint64_t)rt->GetVariable,
        &regs,
        stackWords,
        sizeof(stackWords),
        &efiStatus);

    if (kr != KERN_SUCCESS) {
        return kIOReturnUnsupported;
    }

    if (efiStatus == EFI_SUCCESS) {
        if (dataSize < kNVRAMImageSize) {
            bzero(fImage + dataSize, kNVRAMImageSize - (uint32_t)dataSize);
        }
        fImageValid = true;
        return kIOReturnSuccess;
    }

    if (efiStatus == EFI_NOT_FOUND) {
        bzero(fImage, sizeof(fImage));
        fImageValid = true;
        return kIOReturnSuccess;
    }

    return kIOReturnIOError;
}

IOReturn
EFINVRAM::flushToEFI(void)
{
    EFI_RUNTIME_SERVICES_64 *rt = (EFI_RUNTIME_SERVICES_64 *)gPEEFIRuntimeServices;
    struct pal_efi_registers regs;
    uint64_t stackWords[6];
    uint64_t efiStatus = 0;
    const uint32_t attrs = EFI_VARIABLE_NON_VOLATILE |
        EFI_VARIABLE_BOOTSERVICE_ACCESS |
        EFI_VARIABLE_RUNTIME_ACCESS;
    kern_return_t kr;

    if (!rt || !rt->SetVariable) {
        return kIOReturnNotReady;
    }

    regs.rcx = (uint64_t)(uintptr_t)kEFINVRAMVarName;
    regs.rdx = (uint64_t)(uintptr_t)&kEFINVRAMVendorGuid;
    regs.r8  = (uint64_t)attrs;
    regs.r9  = (uint64_t)kNVRAMImageSize;
    regs.rax = 0;

    prepareStackArg5(stackWords, (uint64_t)(uintptr_t)fImage);

    kr = pal_efi_call_in_64bit_mode((uint64_t)rt->SetVariable,
        &regs,
        stackWords,
        sizeof(stackWords),
        &efiStatus);

    if (kr != KERN_SUCCESS) {
        return kIOReturnUnsupported;
    }

    return (efiStatus == EFI_SUCCESS) ? kIOReturnSuccess : kIOReturnIOError;
}

