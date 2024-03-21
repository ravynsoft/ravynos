/*
 * xml.c: a libFuzzer target to test several XML parser interfaces.
 *
 * See Copyright for the status of this software.
 */

#include <libxml/catalog.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlreader.h>
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
    xmlSetExternalEntityLoader(xmlFuzzEntityLoader);

    return 0;
}

int
LLVMFuzzerTestOneInput(const char *data, size_t size) {
    xmlDocPtr doc;
    const char *docBuffer, *docUrl;
    size_t maxAlloc, docSize;
    int opts;

    xmlFuzzDataInit(data, size);
    opts = (int) xmlFuzzReadInt(4);
    /*
     * Disable options that are known to cause timeouts
     */
    opts &= ~XML_PARSE_XINCLUDE &
            ~XML_PARSE_DTDVALID &
            ~XML_PARSE_SAX1;
    maxAlloc = xmlFuzzReadInt(4) % (size + 1);

    xmlFuzzReadEntities();
    docBuffer = xmlFuzzMainEntity(&docSize);
    docUrl = xmlFuzzMainUrl();
    if (docBuffer == NULL)
        goto exit;

    /* Pull parser */

    xmlFuzzMemSetLimit(maxAlloc);
    doc = xmlReadMemory(docBuffer, docSize, docUrl, NULL, opts);

#ifdef LIBXML_OUTPUT_ENABLED
    {
        xmlChar *out;
        int outSize;

        /* Also test the serializer. */
        xmlDocDumpMemory(doc, &out, &outSize);
        xmlFree(out);
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
        ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, docUrl);
        if (ctxt == NULL)
            goto exit;
        xmlCtxtUseOptions(ctxt, opts);

        for (consumed = 0; consumed < docSize; consumed += chunkSize) {
            chunkSize = docSize - consumed;
            if (chunkSize > maxChunkSize)
                chunkSize = maxChunkSize;
            xmlParseChunk(ctxt, docBuffer + consumed, chunkSize, 0);
        }

        xmlParseChunk(ctxt, NULL, 0, 1);
        xmlFreeDoc(ctxt->myDoc);
        xmlFreeParserCtxt(ctxt);
    }
#endif

    /* Reader */

#ifdef LIBXML_READER_ENABLED
    {
        xmlTextReaderPtr reader;
        int j;

        xmlFuzzMemSetLimit(maxAlloc);
        reader = xmlReaderForMemory(docBuffer, docSize, NULL, NULL, opts);
        if (reader == NULL)
            goto exit;
        while (xmlTextReaderRead(reader) == 1) {
            if (xmlTextReaderNodeType(reader) == XML_ELEMENT_NODE) {
                int i, n = xmlTextReaderAttributeCount(reader);
                for (i=0; i<n; i++) {
                    xmlTextReaderMoveToAttributeNo(reader, i);
                    while (xmlTextReaderReadAttributeValue(reader) == 1);
                }
            }
        }
        for (j = 0; j < 10; j++)
            xmlTextReaderRead(reader);
        xmlFreeTextReader(reader);
    }
#endif

exit:
    xmlFuzzMemSetLimit(0);
    xmlFuzzDataCleanup();
    xmlResetLastError();
    return(0);
}

