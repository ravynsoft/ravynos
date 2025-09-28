/* libxml2 - Library for parsing XML documents
 * Copyright (C) 2006-2019 Free Software Foundation, Inc.
 *
 * This file is not part of the GNU gettext program, but is used with
 * GNU gettext.
 *
 * The original copyright notice is as follows:
 */

/*
 * Copyright (C) 1998-2012 Daniel Veillard.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is fur-
 * nished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
 * NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * daniel@veillard.com
 */

/*
 * DOCBparser.c : an attempt to parse SGML Docbook documents
 *
 * This is deprecated !!!
 * Code removed with release 2.6.0 it was broken.
 * The doc are expect to be migrated to XML DocBook
 */

#define IN_LIBXML
#include "libxml.h"
#ifdef LIBXML_DOCB_ENABLED

#include <libxml/xmlerror.h>
#include <libxml/DOCBparser.h>

/**
 * docbEncodeEntities:
 * @out:  a pointer to an array of bytes to store the result
 * @outlen:  the length of @out
 * @in:  a pointer to an array of UTF-8 chars
 * @inlen:  the length of @in
 * @quoteChar: the quote character to escape (' or ") or zero.
 *
 * Take a block of UTF-8 chars in and try to convert it to an ASCII
 * plus SGML entities block of chars out.
 *
 * Returns 0 if success, -2 if the transcoding fails, or -1 otherwise
 * The value of @inlen after return is the number of octets consumed
 *     as the return value is positive, else unpredictable.
 * The value of @outlen after return is the number of octets consumed.
 */
int
docbEncodeEntities(unsigned char *out ATTRIBUTE_UNUSED,
                   int *outlen ATTRIBUTE_UNUSED,
                   const unsigned char *in ATTRIBUTE_UNUSED,
                   int *inlen ATTRIBUTE_UNUSED,
                   int quoteChar ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbEncodeEntities() deprecated function reached\n");
        deprecated = 1;
    }
    return(-1);
}

/**
 * docbParseDocument:
 * @ctxt:  an SGML parser context
 *
 * parse an SGML document (and build a tree if using the standard SAX
 * interface).
 *
 * Returns 0, -1 in case of error. the parser context is augmented
 *                as a result of the parsing.
 */

int
docbParseDocument(docbParserCtxtPtr ctxt ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbParseDocument() deprecated function reached\n");
        deprecated = 1;
    }
    return (xmlParseDocument(ctxt));
}

/**
 * docbFreeParserCtxt:
 * @ctxt:  an SGML parser context
 *
 * Free all the memory used by a parser context. However the parsed
 * document in ctxt->myDoc is not freed.
 */

void
docbFreeParserCtxt(docbParserCtxtPtr ctxt ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbFreeParserCtxt() deprecated function reached\n");
        deprecated = 1;
    }
    xmlFreeParserCtxt(ctxt);
}

/**
 * docbParseChunk:
 * @ctxt:  an XML parser context
 * @chunk:  an char array
 * @size:  the size in byte of the chunk
 * @terminate:  last chunk indicator
 *
 * Parse a Chunk of memory
 *
 * Returns zero if no error, the xmlParserErrors otherwise.
 */
int
docbParseChunk(docbParserCtxtPtr ctxt ATTRIBUTE_UNUSED,
               const char *chunk ATTRIBUTE_UNUSED,
	       int size ATTRIBUTE_UNUSED,
               int terminate ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbParseChunk() deprecated function reached\n");
        deprecated = 1;
    }

    return (xmlParseChunk(ctxt, chunk, size, terminate));
}

/**
 * docbCreatePushParserCtxt:
 * @sax:  a SAX handler
 * @user_data:  The user data returned on SAX callbacks
 * @chunk:  a pointer to an array of chars
 * @size:  number of chars in the array
 * @filename:  an optional file name or URI
 * @enc:  an optional encoding
 *
 * Create a parser context for using the DocBook SGML parser in push mode
 * To allow content encoding detection, @size should be >= 4
 * The value of @filename is used for fetching external entities
 * and error/warning reports.
 *
 * Returns the new parser context or NULL
 */
docbParserCtxtPtr
docbCreatePushParserCtxt(docbSAXHandlerPtr sax ATTRIBUTE_UNUSED,
                         void *user_data ATTRIBUTE_UNUSED,
                         const char *chunk ATTRIBUTE_UNUSED,
			 int size ATTRIBUTE_UNUSED,
			 const char *filename ATTRIBUTE_UNUSED,
                         xmlCharEncoding enc ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbParseChunk() deprecated function reached\n");
        deprecated = 1;
    }

    return(xmlCreatePushParserCtxt(sax, user_data, chunk, size, filename));
}

/**
 * docbSAXParseDoc:
 * @cur:  a pointer to an array of xmlChar
 * @encoding:  a free form C string describing the SGML document encoding, or NULL
 * @sax:  the SAX handler block
 * @userData: if using SAX, this pointer will be provided on callbacks.
 *
 * parse an SGML in-memory document and build a tree.
 * It use the given SAX function block to handle the parsing callback.
 * If sax is NULL, fallback to the default DOM tree building routines.
 *
 * Returns the resulting document tree
 */

docbDocPtr
docbSAXParseDoc(xmlChar * cur ATTRIBUTE_UNUSED,
                const char *encoding ATTRIBUTE_UNUSED,
		docbSAXHandlerPtr sax ATTRIBUTE_UNUSED,
                void *userData ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbParseChunk() deprecated function reached\n");
        deprecated = 1;
    }

    return (xmlSAXParseMemoryWithData(sax, (const char *)cur,
			  xmlStrlen((const xmlChar *) cur), 0,  userData));
}

/**
 * docbParseDoc:
 * @cur:  a pointer to an array of xmlChar
 * @encoding:  a free form C string describing the SGML document encoding, or NULL
 *
 * parse an SGML in-memory document and build a tree.
 *
 * Returns the resulting document tree
 */

docbDocPtr
docbParseDoc(xmlChar * cur ATTRIBUTE_UNUSED,
             const char *encoding ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbParseChunk() deprecated function reached\n");
        deprecated = 1;
    }

    return (xmlParseDoc(cur));
}


/**
 * docbCreateFileParserCtxt:
 * @filename:  the filename
 * @encoding:  the SGML document encoding, or NULL
 *
 * Create a parser context for a file content.
 * Automatic support for ZLIB/Compress compressed document is provided
 * by default if found at compile-time.
 *
 * Returns the new parser context or NULL
 */
docbParserCtxtPtr
docbCreateFileParserCtxt(const char *filename ATTRIBUTE_UNUSED,
                         const char *encoding ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbCreateFileParserCtxt() deprecated function reached\n");
        deprecated = 1;
    }

    return (xmlCreateFileParserCtxt(filename));
}

/**
 * docbSAXParseFile:
 * @filename:  the filename
 * @encoding:  a free form C string describing the SGML document encoding, or NULL
 * @sax:  the SAX handler block
 * @userData: if using SAX, this pointer will be provided on callbacks.
 *
 * parse an SGML file and build a tree. Automatic support for ZLIB/Compress
 * compressed document is provided by default if found at compile-time.
 * It use the given SAX function block to handle the parsing callback.
 * If sax is NULL, fallback to the default DOM tree building routines.
 *
 * Returns the resulting document tree
 */

docbDocPtr
docbSAXParseFile(const char *filename ATTRIBUTE_UNUSED,
                 const char *encoding ATTRIBUTE_UNUSED,
                 docbSAXHandlerPtr sax ATTRIBUTE_UNUSED,
		 void *userData ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbSAXParseFile() deprecated function reached\n");
        deprecated = 1;
    }

    return (xmlSAXParseFileWithData(sax, filename, 0, userData));
}

/**
 * docbParseFile:
 * @filename:  the filename
 * @encoding:  a free form C string describing document encoding, or NULL
 *
 * parse a Docbook SGML file and build a tree. Automatic support for
 * ZLIB/Compress compressed document is provided by default if found
 * at compile-time.
 *
 * Returns the resulting document tree
 */

docbDocPtr
docbParseFile(const char *filename ATTRIBUTE_UNUSED,
              const char *encoding ATTRIBUTE_UNUSED)
{
    static int deprecated = 0;

    if (!deprecated) {
        xmlGenericError(xmlGenericErrorContext,
                        "docbParseFile() deprecated function reached\n");
        deprecated = 1;
    }

    return (xmlParseFile(filename));
}
#define bottom_DOCBparser
#include "elfgcchack.h"
#endif /* LIBXML_DOCB_ENABLED */
