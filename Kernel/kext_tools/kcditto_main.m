#import <Foundation/Foundation.h>
#import <IOKit/kext/OSKext.h>
#import <IOKit/kext/OSKextPrivate.h>
#import <Bom/Bom.h>
#import <APFS/APFS.h>

#import <stdio.h>
#import <stdbool.h>
#import <sysexits.h>
#import <unistd.h>

#import "bootcaches.h"
#import "kext_tools_util.h"
#import "rosp_staging.h"

bool removeTempScript(void)
{
    bool result = false;
    if (remove(_kOSKextDeferredBootcachesInstallScriptPath) < 0) {
        LOG_ERROR("Error removing %s: %d (%s)",
                _kOSKextDeferredBootcachesInstallScriptPath,
                errno, strerror(errno));
        goto finish;
    }
    result = true;
finish:
    return result;
}

int main(int argc, char **argv)
{
    (void)argv;

    int result = EX_SOFTWARE;

    if (argc != 1) {
        fprintf(stderr, "kcditto takes no arguments and should not be run directly.\n");
        fprintf(stderr, "kcditto installs previously-rebuilt kernelcaches onto the System volume.\n");
        result = EX_USAGE;
        goto finish;
    }

    result = copyKernelsInVolume("/");
    if (result != EX_OK) {
        LOG_ERROR("Error copying kernels (standalone)...");
        goto finish;
    }

    if (!removeTempScript()) {
        LOG_ERROR("Error removing temp script (standalone)...");
        result = EX_SOFTWARE;
        goto finish;
    }
finish:
    return result;
}
