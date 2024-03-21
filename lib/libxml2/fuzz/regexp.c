/*
 * regexp.c: a libFuzzer target to test the regexp module.
 *
 * See Copyright for the status of this software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <libxml/xmlregexp.h>
#include "fuzz.h"

int
LLVMFuzzerInitialize(int *argc ATTRIBUTE_UNUSED,
                     char ***argv ATTRIBUTE_UNUSED) {
    xmlFuzzMemSetup();
    xmlSetGenericErrorFunc(NULL, xmlFuzzErrorFunc);

    return 0;
}

int
LLVMFuzzerTestOneInput(const char *data, size_t size) {
    xmlRegexpPtr regexp;
    size_t maxAlloc;
    const char *str1;

    if (size > 200)
        return(0);

    xmlFuzzDataInit(data, size);
    maxAlloc = xmlFuzzReadInt(4) % (size * 8 + 1);
    str1 = xmlFuzzReadString(NULL);

    xmlFuzzMemSetLimit(maxAlloc);
    regexp = xmlRegexpCompile(BAD_CAST str1);
    if (xmlFuzzMallocFailed() && regexp != NULL) {
        fprintf(stderr, "malloc failure not reported\n");
        abort();
    }
    /* xmlRegexpExec has pathological performance in too many cases. */
#if 0
    xmlRegexpExec(regexp, BAD_CAST str2);
#endif
    xmlRegFreeRegexp(regexp);

    xmlFuzzMemSetLimit(0);
    xmlFuzzDataCleanup();
    xmlResetLastError();

    return 0;
}

