/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFXMLParser.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

/*  CFXMLParser is deprecated as of Mac OS X 10.8. The suggested replacements are the Foundation classes NSXMLParser and NSXMLDocument, or the libxml2 library. */

#if !defined(__COREFOUNDATION_CFXMLPARSER__)
#define __COREFOUNDATION_CFXMLPARSER__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFTree.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFXMLNode.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct __CFXMLParser * CFXMLParserRef;

/* These are the various options you can configure the parser with.  These are
   chosen such that an option flag of 0 (kCFXMLParserNoOptions) leaves the XML
   as "intact" as possible (reports all structures; performs no replacements).
   Hence, to make the parser do the most work, returning only the pure element
   tree, set the option flag to kCFXMLParserAllOptions.

kCFXMLParserValidateDocument -
   validate the document against its grammar from the DTD, reporting any errors.
   Currently not supported.

kCFXMLParserSkipMetaData -
   silently skip over metadata constructs (the DTD and comments)

kCFXMLParserReplacePhysicalEntities -
   replace declared entities like &lt;.  Note that other than the 5 predefined
   entities (lt, gt, quot, amp, apos), these must be defined in the DTD.
   Currently not supported.

kCFXMLParserSkipWhitespace -
   skip over all whitespace that does not abut non-whitespace character data.
   In other words, given <foo>  <bar> blah </bar></foo>, the whitespace between
   foo's open tag and bar's open tag would be suppressed, but the whitespace
   around blah would be preserved.

kCFXMLParserAddImpliedAttributes -
   where the DTD specifies implied attribute-value pairs for a particular element,
   add those pairs to any occurances of the element in the element tree.
   Currently not supported.
*/

typedef CF_OPTIONS(CFOptionFlags, CFXMLParserOptions) {
    kCFXMLParserValidateDocument = (1UL << 0),
    kCFXMLParserSkipMetaData = (1UL << 1),
    kCFXMLParserReplacePhysicalEntities = (1UL << 2),
    kCFXMLParserSkipWhitespace = (1UL << 3),
    kCFXMLParserResolveExternalEntities = (1UL << 4),
    kCFXMLParserAddImpliedAttributes = (1UL << 5),
    kCFXMLParserAllOptions = 0x00FFFFFF,
    kCFXMLParserNoOptions = 0
};

/* This list is expected to grow */
typedef CF_OPTIONS(CFIndex, CFXMLParserStatusCode) {
    kCFXMLStatusParseNotBegun = -2,
    kCFXMLStatusParseInProgress = -1,
    kCFXMLStatusParseSuccessful = 0,
    kCFXMLErrorUnexpectedEOF = 1,
    kCFXMLErrorUnknownEncoding,
    kCFXMLErrorEncodingConversionFailure,
    kCFXMLErrorMalformedProcessingInstruction,
    kCFXMLErrorMalformedDTD,
    kCFXMLErrorMalformedName,
    kCFXMLErrorMalformedCDSect,
    kCFXMLErrorMalformedCloseTag,
    kCFXMLErrorMalformedStartTag,
    kCFXMLErrorMalformedDocument,
    kCFXMLErrorElementlessDocument,
    kCFXMLErrorMalformedComment,
    kCFXMLErrorMalformedCharacterReference,
    kCFXMLErrorMalformedParsedCharacterData,
    kCFXMLErrorNoData
};


/*  These functions are called as a parse progresses.

createXMLStructure -
  called as new XML structures are encountered by the parser.  May return NULL to indicate
  that the given structure should be skipped; if NULL is returned for a given structure,
  only minimal parsing is done for that structure (enough to correctly determine its end,
  and to extract any data necessary for the remainder of the parse, such as Entity definitions).
  createXMLStructure (or indeed, any of the tree-creation callbacks) will not be called for any
  children of the skipped structure.  The only exception is that the top-most element will always
  be reported even if NULL was returned for the document as a whole.  NOTE: for performance reasons,
  the node passed to createXMLStructure cannot be safely retained by the client; the node as
  a whole must be copied (via CFXMLNodeCreateCopy), or its contents must be extracted and copied.

addChild -
  called as children are parsed and are ready to be added to the tree.  If createXMLStructure
  returns NULL for a given structure, that structure is omitted entirely, and addChild will
  NOT be called for either a NULL child or parent.

endXMLStructure -
  called once a structure (and all its children) are completely parsed.  As elements are encountered,
  createXMLStructure is called for them first, then addChild to add the new structure to its parent,
  then addChild (potentially several times) to add the new structure's children to it, then finally 
  endXMLStructure to show that the structure has been fully parsed.

createXMLStructure, addChild, and endXMLStructure are all REQUIRED TO BE NON-NULL.

resolveExternalEntity -
  called when external entities are referenced (NOT when they are simply defined).  If the function
  pointer is NULL, the parser uses its internal routines to try and resolve the entity.  If the
  function pointer is set, and the function returns NULL, a place holder for the external entity
  is inserted into the tree.  In this manner, the parser's client can prevent any external network 
  or file accesses.

handleError - called as errors/warnings are encountered in the data stream.  At some point, we will
  have an enum of the expected errors, some of which will be fatal, others of which will not.  If
  the function pointer is NULL, the parser will silently attempt to recover.  The
  handleError function may always return false to force the parser to stop; if handleError returns
  true, the parser will attempt to recover (fatal errors will still cause the parse to abort
  immediately).
*/

typedef void *		(*CFXMLParserCreateXMLStructureCallBack)(CFXMLParserRef parser, CFXMLNodeRef nodeDesc, void *info);
typedef void		(*CFXMLParserAddChildCallBack)(CFXMLParserRef parser, void *parent, void *child, void *info);
typedef void		(*CFXMLParserEndXMLStructureCallBack)(CFXMLParserRef parser, void *xmlType, void *info);
typedef CFDataRef	(*CFXMLParserResolveExternalEntityCallBack)(CFXMLParserRef parser, CFXMLExternalID *extID, void *info);
typedef Boolean		(*CFXMLParserHandleErrorCallBack)(CFXMLParserRef parser, CFXMLParserStatusCode error, void *info);
typedef struct {
    CFIndex                                  version;
    CFXMLParserCreateXMLStructureCallBack    createXMLStructure;
    CFXMLParserAddChildCallBack              addChild;
    CFXMLParserEndXMLStructureCallBack       endXMLStructure;
    CFXMLParserResolveExternalEntityCallBack resolveExternalEntity;
    CFXMLParserHandleErrorCallBack           handleError;
} CFXMLParserCallBacks;

typedef const void *	(*CFXMLParserRetainCallBack)(const void *info);
typedef void		(*CFXMLParserReleaseCallBack)(const void *info);
typedef CFStringRef	(*CFXMLParserCopyDescriptionCallBack)(const void *info);
typedef struct {
    CFIndex				version;
    void *				info;
    CFXMLParserRetainCallBack		retain;
    CFXMLParserReleaseCallBack		release;
    CFXMLParserCopyDescriptionCallBack	copyDescription;
} CFXMLParserContext;

CF_EXPORT
CFTypeID CFXMLParserGetTypeID(void) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Creates a parser which will parse the given data with the given options.  xmlData may not be NULL. 
   dataSource should be the URL from which the data came, and may be NULL; it is used to resolve any
   relative references found in xmlData. versionOfNodes determines which version CFXMLNodes are produced
   by the parser; see CFXMLNode.h for more details.  callBacks are the callbacks called by the parser as
   the parse progresses; callBacks, callBacks->createXMLStructure, callBacks->addChild, and
   callBacks->endXMLStructure must all be non-NULL.  context determines what if any info pointer is
   passed to the callbacks as the parse progresses; context may be NULL.  */
CF_EXPORT
CFXMLParserRef CFXMLParserCreate(CFAllocatorRef allocator, CFDataRef xmlData, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes, CFXMLParserCallBacks *callBacks, CFXMLParserContext *context) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Arguments as above, except that the data to be parsed is loaded directly 
   from dataSource.  dataSource may not be NULL.  */
CF_EXPORT
CFXMLParserRef CFXMLParserCreateWithDataFromURL(CFAllocatorRef allocator, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes, CFXMLParserCallBacks *callBacks, CFXMLParserContext *context) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

CF_EXPORT
void CFXMLParserGetContext(CFXMLParserRef parser, CFXMLParserContext *context) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

CF_EXPORT
void CFXMLParserGetCallBacks(CFXMLParserRef parser, CFXMLParserCallBacks *callBacks) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

CF_EXPORT
CFURLRef CFXMLParserGetSourceURL(CFXMLParserRef parser) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Returns the character index of the current parse location */
CF_EXPORT
CFIndex CFXMLParserGetLocation(CFXMLParserRef parser) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Returns the line number of the current parse location */
CF_EXPORT
CFIndex CFXMLParserGetLineNumber(CFXMLParserRef parser) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Returns the top-most object returned by the createXMLStructure callback */
CF_EXPORT
void *CFXMLParserGetDocument(CFXMLParserRef parser) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Get the status code or a user-readable description of the last error that occurred in a parse.
   If no error has occurred, a null description string is returned.  See the enum above for
   possible status returns */
CF_EXPORT
CFXMLParserStatusCode CFXMLParserGetStatusCode(CFXMLParserRef parser) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

CF_EXPORT
CFStringRef CFXMLParserCopyErrorDescription(CFXMLParserRef parser) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Cause any in-progress parse to abort with the given error code and description.  errorCode
   must be positive, and errorDescription may not be NULL.  Cannot be called asynchronously
   (i.e. must be called from within a parser callback) */
CF_EXPORT
void CFXMLParserAbort(CFXMLParserRef parser, CFXMLParserStatusCode errorCode, CFStringRef errorDescription) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Starts a parse of the data the parser was created with; returns success or failure.
   Upon success, use CFXMLParserGetDocument() to get the product of the parse.  Upon
   failure, use CFXMLParserGetErrorCode() or CFXMLParserCopyErrorDescription() to get
   information about the error.  It is an error to call CFXMLParserParse() while a
   parse is already underway. */
CF_EXPORT
Boolean CFXMLParserParse(CFXMLParserRef parser) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* These functions provide a higher-level interface.  The XML data is parsed to a
   special CFTree (an CFXMLTree) with known contexts and callbacks.  See CFXMLNode.h
   for full details on using an CFXMLTree and the CFXMLNodes contained therein.
*/
/* Parse to an CFXMLTreeRef.  parseOptions are as above. versionOfNodes determines
   what version CFXMLNodes are used to populate the tree.  */
CF_EXPORT
CFXMLTreeRef CFXMLTreeCreateFromData(CFAllocatorRef allocator, CFDataRef xmlData, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* As above, with the additional by-reference pass of a CFDictionaryRef containing
   various error information (see below). The caller is responsible for releasing the
   returned dictionary. If the error dictionary is not desired, pass NULL. */
CF_EXPORT
CFXMLTreeRef CFXMLTreeCreateFromDataWithError(CFAllocatorRef allocator, CFDataRef xmlData, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes, CFDictionaryRef *errorDict) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Loads the data to be parsed directly from dataSource.  Arguments as above. */
CF_EXPORT
CFXMLTreeRef CFXMLTreeCreateWithDataFromURL(CFAllocatorRef allocator, CFURLRef dataSource, CFOptionFlags parseOptions, CFIndex versionOfNodes) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Generate the XMLData (ready to be written to whatever permanent storage is to be
   used) from an CFXMLTree.  Will NOT regenerate entity references (except those
   required for syntactic correctness) if they were replaced at the parse time;
   clients that wish this should walk the tree and re-insert any entity references
   that should appear in the final output file. */
CF_EXPORT
CFDataRef CFXMLTreeCreateXMLData(CFAllocatorRef allocator, CFXMLTreeRef xmlTree) CF_DEPRECATED(10_0, 10_8, 2_0, 6_0);

/* Escaping and unescaping XML entities in CFStrings. The standard XML entities
   are always replaced.  */
/* Creates a CFString by replacing entities that appear in the entities dictionary.
   Dictionary keys are the entities themselves, and the values should be CFStrings
   containing the expansion. Pass NULL for entitiesDictionary to indicate no entities
   other than the standard five. */
CF_EXPORT
CFStringRef CFXMLCreateStringByEscapingEntities(CFAllocatorRef allocator, CFStringRef string, CFDictionaryRef entitiesDictionary);

CF_EXPORT
CFStringRef CFXMLCreateStringByUnescapingEntities(CFAllocatorRef allocator, CFStringRef string, CFDictionaryRef entitiesDictionary);

/* CFXMLTreeCreateFromDataWithError error dictionary key constants. */
CF_EXPORT const CFStringRef kCFXMLTreeErrorDescription;
    /* value is a CFString containing the readable error string. */

CF_EXPORT const CFStringRef kCFXMLTreeErrorLineNumber;
    /* value is a CFNumber containing the line on which the error appears. */

CF_EXPORT const CFStringRef kCFXMLTreeErrorLocation;
    /* value is a CFNumber containing the byte location at which the error occurred. */

CF_EXPORT const CFStringRef kCFXMLTreeErrorStatusCode;
    /* value is a CFNumber containing the error status code. */

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFXMLPARSER__ */

