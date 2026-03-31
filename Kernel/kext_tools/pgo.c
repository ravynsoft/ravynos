
#include <assert.h>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/syslimits.h>
#include <sys/stat.h>
#include <pthread.h>

#include <sys/pgo.h>

#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>

#define PGO_FILENAME_BASE "/var/log/pgo"
#define PGO_FILE_MODE 0755

static
char *pgo_read_metadata(void *buffer, size_t size, char *key)
{
    size_t keylen = strlen(key);

    struct pgo_metadata_footer footer;
    if (size < sizeof(footer))
    {
        return NULL;
    }

    memcpy(&footer, buffer + size - sizeof(footer), sizeof(footer));

    char *pairs = (buffer + size) - ntohl(footer.offset_to_pairs);
    if ((void*)pairs < buffer) {
        return NULL;
    }

    uint32_t num_pairs = ntohl(footer.number_of_pairs);
    while (num_pairs > 0 && (void*)pairs < buffer + size)
    {
        size_t pairlen = strnlen(pairs, ((char*)buffer + size) - pairs);
        if ((void*)(pairs + pairlen) >= buffer + size)
        {
            return NULL;
        }
        if (0 == memcmp(key, pairs, keylen) && pairs[keylen] == '=')
        {
            return pairs + keylen + 1;
        }
        num_pairs--;
        pairs += pairlen + 1;
    }
    return NULL;
}

static
void *pgo_thread_main(void *context)
{
    OSKextRef kext = (OSKextRef) context;
    void *buffer = NULL;
    int fd = -1;

    uuid_t uuid;

    CFDataRef uuid_dataref = OSKextCopyUUIDForArchitecture(kext, NULL);
    if (uuid_dataref)
    {
        assert(CFDataGetLength(uuid_dataref) == sizeof(uuid));
        memcpy(&uuid, CFDataGetBytePtr(uuid_dataref), sizeof(uuid));
        CFRelease(uuid_dataref);
    }
    else
    {
        goto fail;
    }

    CFStringRef id = OSKextGetIdentifier(kext);
    if (!id)
    {
        goto fail;
    }

    char id_rep[NAME_MAX];
    Boolean b = CFStringGetFileSystemRepresentation(id, id_rep, sizeof(id_rep));
    if (!b)
    {
        goto fail;
    }

    ssize_t size = grab_pgo_data(&uuid, PGO_METADATA, NULL, 0);
    if (size < 0)
    {
        OSKextLog(kext, kOSKextLogErrorLevel, "failed to get size of pgo buffer: %s", strerror(errno));
        goto fail;
    }

    buffer = malloc(size);
    if (!buffer) {
        OSKextLog(kext, kOSKextLogErrorLevel, "failed to allocate pgo buffer");
        goto fail;
    }

    size= grab_pgo_data(&uuid, PGO_METADATA | PGO_WAIT_FOR_UNLOAD, buffer, size);
    if (size < 0)
    {
        OSKextLog(kext, kOSKextLogErrorLevel, "failed to get size of pgo buffer: %s", strerror(errno));
        goto fail;
    }

    char *instance = pgo_read_metadata(buffer, size, "INSTANCE");
    if (!instance) {
        OSKextLog(kext, kOSKextLogErrorLevel, "no metadata in pgo buffer");
    }

    mkdir(PGO_FILENAME_BASE, PGO_FILE_MODE);

    char filename[PATH_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", PGO_FILENAME_BASE, id_rep);
    mkdir (filename, PGO_FILE_MODE);

    snprintf(filename, sizeof(filename), "%s/%s/%s", PGO_FILENAME_BASE, id_rep, instance);
    fd = fopen(filename, "w");

    fd = open(filename, O_WRONLY|O_CREAT);
    if (fd < 0)
    {
        OSKextLog(kext, kOSKextLogErrorLevel, "error (%s) while opening pgo output file: %s",
                  strerror(errno), filename);
        goto fail;
    }

    unsigned char *p = buffer;
    ssize_t r;
    while (size > 0) {
        errno = 0;
        r = write(fd, p, size);
        if (r > 0) {
            p += r;
            size -= r;
        } else {
            OSKextLog(kext, kOSKextLogErrorLevel, "error writing pgo file: %s", strerror(errno));
            goto fail;
        }
    }

fail:
    CFRelease(kext);
    if (buffer) {
        free(buffer);
    }
    if (fd > 0) {
        close(fd);
    }
    return NULL;
}

void pgo_start_thread(OSKextRef kext)
{
    CFRetain(kext);
    pthread_t thread;
    int r;
    r = pthread_create(&thread, NULL, pgo_thread_main, kext);
    if (r)
    {
        OSKextLog(kext, kOSKextLogErrorLevel, "failed to create thread");
        CFRelease(kext);
    }
}

bool pgo_scan_kexts(CFArrayRef array)
{
    CFIndex i;
    bool found = false;

    for (i = 0; i < CFArrayGetCount(array); i++)
    {
        CFDictionaryRef dict = (CFDictionaryRef) CFArrayGetValueAtIndex(array, i);

        assert(CFGetTypeID(dict) == CFDictionaryGetTypeID());

        CFStringRef id  = CFDictionaryGetValue(dict, CFSTR("CFBundleIdentifier"));
        CFStringRef ver = CFDictionaryGetValue(dict, CFSTR("CFBundleVersion"));

        if (!id || !ver ||
            CFGetTypeID(id) != CFStringGetTypeID() ||
            CFGetTypeID(ver) != CFStringGetTypeID())
        {
            continue;
        }

        OSKextRef kext = OSKextGetKextWithIdentifierAndVersion(id, OSKextParseVersionCFString(ver));

        if (!kext) {
            continue;
        }

        CFDataRef uuid = OSKextCopyUUIDForArchitecture(kext, NULL);
        if (!uuid) {
            continue;
        }
        assert(CFDataGetLength(uuid) == sizeof(uuid_t));
        ssize_t size = grab_pgo_data((uuid_t*)CFDataGetBytePtr(uuid), PGO_METADATA, NULL, 0);
        if (size < 0) {
            continue;
        }
        pgo_start_thread(kext);
        found = true;
    }

    return found;
}
