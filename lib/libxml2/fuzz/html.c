/*
 * html.c: a libFuzzer target to test several HTML parser interfaces.
 *
 * See Copyright for the status of this software.
 */

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/catalog.h>
#include "fuzz.h"

int
LLVMFuzzerInitialize(int *argc ATTRIBUTE_UNUSED,
                     char ***argv ATTRIBUTE_UNUSED) {
    xmlFuzzMemSetup();
    xmlInitParser();
#ifdef LIBXML_CATALOG_ENABLED
    xmlInitializeCatalog();
#endif
    xmlSetGenericErrorFunc(NULL, xmlFuzzErrorFunc);

    return 0;
}

int
LLVMFuzzerTestOneInput(const char *data, size_t size) {
    htmlDocPtr doc;
    const char *docBuffer;
    size_t maxAlloc, docSize;
    int opts;

    xmlFuzzDataInit(data, size);
    opts = (int) xmlFuzzReadInt(4);
    maxAlloc = xmlFuzzReadInt(4) % (size + 1);

    docBuffer = xmlFuzzReadRemaining(&docSize);
    if (docBuffer == NULL) {
        xmlFuzzDataCleanup();
        return(0);
    }

    /* Pull parser */

    xmlFuzzMemSetLimit(maxAlloc);
    doc = htmlReadMemory(docBuffer, docSize, NULL, NULL, opts);

#ifdef LIBXML_OUTPUT_ENABLED
    {
        xmlOutputBufferPtr out;

        /*
         * Also test the serializer. Call htmlDocContentDumpOutput with our
         * own buffer to avoid encoding the output. The HTML encoding is
         * excruciatingly slow (see htmlEntityValueLookup).
         */
        out = xmlAllocOutputBuffer(NULL);
        htmlDocContentDumpOutput(out, doc, NULL);
        xmlOutputBufferClose(out);
    }
#endif

    xmlFreeDoc(doc);

    /* Push parser */

#ifdef LIBXML_PUSH_ENABLED
    {
        static const size_t maxChunkSize = 128;
        xmlParserCtxtPtr ctxt;
        size_t consumed, chunkSize;

        xmlFuzzMemSetLimit(maxAlloc);
        ctxt = htmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL,
                                        XML_CHAR_ENCODING_NONE);

        if (ctxt != NULL) {
            htmlCtxtUseOptions(ctxt, opts);

            for (consumed = 0; consumed < docSize; consumed += chunkSize) {
                chunkSize = docSize - consumed;
                if (chunkSize > maxChunkSize)
                    chunkSize = maxChunkSize;
                htmlParseChunk(ctxt, docBuffer + consumed, chunkSize, 0);
            }

            htmlParseChunk(ctxt, NULL, 0, 1);
            xmlFreeDoc(ctxt->myDoc);
            htmlFreeParserCtxt(ctxt);
        }
    }
#endif

    /* Cleanup */

    xmlFuzzMemSetLimit(0);
    xmlFuzzDataCleanup();
    xmlResetLastError();

    return(0);
}

