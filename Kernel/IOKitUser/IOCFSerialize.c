/*
 * Copyright (c) 1999-2007 Apple Inc. All rights reserved.
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


#include <device/device_types.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFSerialize.h>
#include <IOKit/IOCFUnserialize.h>

#if IOKIT_SERVER_VERSION >= 20140421
#include <System/libkern/OSSerializeBinary.h>
#endif /* IOKIT_SERVER_VERSION >= 20140421 */

#include <assert.h>
#include <syslog.h>

typedef struct {
    CFMutableDataRef   data;

    int                idrefNumRefs;

/* For each CFType, we track whether a given plist value hasRefs,
 * and what the ids used for those refs are. 'hasRefs' is set to
 * kCFBooleanFalse the first time we see a given value, so we know
 * we saw it, and then kCFBooleanTrue if we see it again, so we know
 * we need an id for it. On writing out the XML, we generate ids as
 * we encounter the need.
 */
    CFMutableDictionaryRef stringIDRefDictionary;
    CFMutableDictionaryRef numberIDRefDictionary;
    CFMutableDictionaryRef dataIDRefDictionary;
    CFMutableDictionaryRef dictionaryIDRefDictionary;
    CFMutableDictionaryRef arrayIDRefDictionary;
    CFMutableDictionaryRef setIDRefDictionary;

} IOCFSerializeState;


static Boolean
DoCFSerialize(CFTypeRef object, IOCFSerializeState * state);

static CFDataRef
IOCFSerializeBinary(CFTypeRef object, CFOptionFlags options);

static Boolean
addChar(char chr, IOCFSerializeState * state)
{
	CFDataAppendBytes(state->data, (UInt8 *) &chr, 1);

	return true;
}

static Boolean
addString(const char * str, IOCFSerializeState * state)
{
	CFIndex	len = strlen(str);
	CFDataAppendBytes(state->data, (UInt8 *) str, len);

	return true;
}

static const char *
getTagString(CFTypeRef object)
{
	CFTypeID type;

	assert(object);

	type = CFGetTypeID(object);

	if (type == CFStringGetTypeID()) return "string";
	if (type == CFNumberGetTypeID()) return "integer";
	if (type == CFDataGetTypeID()) return "data";
	if (type == CFDictionaryGetTypeID()) return "dict";
	if (type == CFArrayGetTypeID()) return "array";
	if (type == CFSetGetTypeID()) return "set";

	return "internal error";
}

static CFMutableDictionaryRef
idRefDictionaryForObject(
    CFTypeRef              object,
    IOCFSerializeState   * state)
{
    CFMutableDictionaryRef result     = NULL;
    CFTypeID               objectType = CFNullGetTypeID();  // do not release

    objectType = CFGetTypeID(object);

   /* Sorted by rough order of % occurence in big plists.
    */
	if (objectType == CFDictionaryGetTypeID()) {
        result = state->dictionaryIDRefDictionary;
    } else if (objectType == CFStringGetTypeID()) {
        result = state->stringIDRefDictionary;
    } else if (objectType == CFArrayGetTypeID()) {
        result = state->arrayIDRefDictionary;
    } else if (objectType == CFNumberGetTypeID()) {
        result = state->numberIDRefDictionary;
    } else if (objectType == CFDataGetTypeID()) {
        result = state->dataIDRefDictionary;
    } else if (objectType == CFSetGetTypeID()) {
        result = state->setIDRefDictionary;
    }

	return result;
}

void
recordObjectInIDRefDictionary(
    CFTypeRef              object,
    IOCFSerializeState   * state)
{
    CFTypeRef              refEntry        = NULL;  // do not release
    CFMutableDictionaryRef idRefDictionary = NULL;  // do not release

    if (!object || !state) {
        goto finish;
    }

    idRefDictionary = idRefDictionaryForObject(object, state);
    if (!idRefDictionary) {
        goto finish;
    }

	refEntry = CFDictionaryGetValue(idRefDictionary, object);

   /* If we have never seen this object value, then add an entry
    * in the dictionary with value false, indicating we have seen
    * it once.
    *
    * If we have seen this object value, then set its entry to true
    * to indicate that we have now seen a second occurrence
    * of the object value, which means we will generate an ID and IDREFs
    * in the XML.
    */
    if (!refEntry) {
       /* Use AddValue here so as not to overwrite.
        */
        CFDictionaryAddValue(idRefDictionary, object, kCFBooleanFalse);
    } else {
        CFDictionarySetValue(idRefDictionary, object, kCFBooleanTrue);
    }

finish:
    return;
}

Boolean
previouslySerialized(
    CFTypeRef            object,
    IOCFSerializeState * state)
{
    Boolean                result     = FALSE;
    CFMutableDictionaryRef idRefDict  = NULL;  // do not release
    CFTypeRef              idRefEntry = NULL;  // do not release
    CFTypeID               idRefType  = CFNullGetTypeID();  // do not release
    char                   temp[64];
    int                    idInt      = -1;

    if (!object || !state) {
        goto finish;
    }

   /* If we don't get a lookup dict or an entry for the object,
    * then no ID or IDREF will be involved,
    * so treat is if never before serialized.
    */
    idRefDict = idRefDictionaryForObject(object, state);
    if (!idRefDict) {
        goto finish;
    }

    idRefEntry = (CFTypeRef)CFDictionaryGetValue(idRefDict, object);
    if (!idRefEntry) {
        goto finish;
    }

    idRefType = CFGetTypeID(idRefEntry);

   /* If the entry is of Boolean type, then an ID/IDREF may be involved,
    * but we haven't created one yet, so not yet serialized.
    */
    if (idRefType == CFBooleanGetTypeID()) {
        goto finish;
    }

   /* At this point the entry should be a CFNumber.
    */
    if (idRefType != CFNumberGetTypeID()) {
        goto finish;
    }

   /* Finally, get the IDREF value out of the idRef entry and write the
    * XML for it. Bail on extraction error.
    */
    if (!CFNumberGetValue((CFNumberRef)idRefEntry, kCFNumberIntType, &idInt)) {
        goto finish;
    }
    snprintf(temp, sizeof(temp), "<%s IDREF=\"%d\"/>", getTagString(object), idInt);
    result = addString(temp, state);

finish:
	return result;
}

static Boolean
addStartTag(
    CFTypeRef            object,
    const char         * additionalTags,
    IOCFSerializeState * state)
{
    CFMutableDictionaryRef idRefDict  = NULL;  // do not release
    CFTypeRef              idRefEntry = NULL;  // do not release
	CFNumberRef            idRef = NULL;  // must release
	char                   temp[128];

    idRefDict = idRefDictionaryForObject(object, state);
    if (idRefDict) {
        idRefEntry = (CFTypeRef)CFDictionaryGetValue(idRefDict, object);
    }

   /* If the IDRef entry is 'true', then we know we have an object value
    * with multiple references and need to emit an ID. So we create one
    * by incrementing the state's counter, making a CFNumber, and *replacing*
    * the boolean value in the IDRef dictionary with that number.
    */
	if (idRefEntry == kCFBooleanTrue) {
        int idInt = state->idrefNumRefs++;

        idRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &idInt);
        assert(idRef);

        CFDictionarySetValue(idRefDict, object, idRef);
        CFRelease(idRef);

		if (additionalTags) {
			snprintf(temp, sizeof(temp) * sizeof(char),
                        	"<%s ID=\"%d\" %s>", getTagString(object), idInt, additionalTags);
		} else {
			snprintf(temp, sizeof(temp) * sizeof(char),
                        	"<%s ID=\"%d\">", getTagString(object), idInt);
		}
	} else {
		if (additionalTags) {
			snprintf(temp, sizeof(temp) * sizeof(char),
                        	"<%s %s>", getTagString(object), additionalTags);
		} else {
			snprintf(temp, sizeof(temp) * sizeof(char),
                        	"<%s>", getTagString(object));
		}
	}

	return addString(temp, state);
}


static Boolean
addEndTag(CFTypeRef object,
	  IOCFSerializeState * state)
{
	char temp[128];

	snprintf(temp, sizeof(temp) * sizeof(char), "</%s>", getTagString(object));

	return addString(temp, state);
}

static Boolean
DoCFSerializeNumber(CFNumberRef object, IOCFSerializeState * state)
{
	char		temp[32];
	long long	value;
	int		size;

	if (previouslySerialized(object, state)) return true;

	if (!CFNumberGetValue(object, kCFNumberLongLongType, &value)) {
		return(false);
    }

	switch(CFNumberGetType(object)) {
	case kCFNumberSInt8Type:
	case kCFNumberCharType:
		size = 8;
		break;

	case kCFNumberSInt16Type:
	case kCFNumberShortType:
		size = 16;
		break;

	case kCFNumberSInt32Type:
	case kCFNumberIntType:
		size = 32;
		break;

	case kCFNumberLongType:
#if __LP64__
		size = 64;
#else
		size = 32;
#endif
		break;

	case kCFNumberSInt64Type:
	case kCFNumberLongLongType:
	default:
		size = 64;
		break;
	}

	snprintf(temp, sizeof(temp), "size=\"%d\"", size);
	if (!addStartTag(object, temp, state)) return false;

	if (size <= 32)
		snprintf(temp, sizeof(temp), "0x%lx", (unsigned long int) value);
	else
		snprintf(temp, sizeof(temp), "0x%qx", value);

	if (!addString(temp, state)) return false;

	return addEndTag(object, state);
}

static Boolean
DoCFSerializeBoolean(CFBooleanRef object, IOCFSerializeState * state)
{
	return addString(CFBooleanGetValue(object) ? "<true/>" : "<false/>", state);
}

static Boolean
DoCFSerializeString(CFStringRef object, IOCFSerializeState * state)
{
	CFDataRef   dataBuffer = 0;
	const char  *buffer = "";
	CFIndex     length = 0;
	char        c;
	int         i;
    Boolean     succeeded = true;

	if (previouslySerialized(object, state)) return true;

	if (!addStartTag(object, 0, state)) return false;

	dataBuffer = CFStringCreateExternalRepresentation(kCFAllocatorDefault, object, kCFStringEncodingUTF8, '?');

	if (dataBuffer) {
		length = CFDataGetLength(dataBuffer);
		buffer = (char *) CFDataGetBytePtr(dataBuffer);
	}

	// this works because all bytes in a multi-byte utf-8 character have the high order bit set
	for (i = 0; (i < length) && succeeded; i++) {
		c = buffer[i];
        switch (c) {
            case '<':
                succeeded = addString("&lt;", state);
                break;
            case '>':
                succeeded = addString("&gt;", state);
                break;
            case '&':
                succeeded = addString("&amp;", state);
                break;
            default:
                succeeded = addChar(c, state);
                break;
        }
	}

	if (dataBuffer) CFRelease(dataBuffer);

    if (succeeded)
        return addEndTag(object, state);
    else
        return false;
}

static Boolean
DoCFSerializeKey(CFStringRef object, IOCFSerializeState * state)
{
	CFDataRef   dataBuffer = 0;
	const char  *buffer = "";
	CFIndex     length = 0;
	char        c;
	int         i;
	Boolean     succeeded = true;

	const char *getOffMyXMLawn = "<!-- \xf0\x9f\xa4\xa6 -->";
	const char *classNames[] = { "AppleLSIFusionFC", "AppleLSIFusionSAS", "AppleLSIFusionSCSI",
				     "AppleUSBXHCI", "ICH8 ATA/100", "Physical Interconnect Location",
				     "IOPCITunnelCompatible" };
	int         numClasses   = (sizeof(classNames) / sizeof(classNames[0]));
	bool        matches      = false;

	if (!addString("<key>", state)) return false;

	dataBuffer = CFStringCreateExternalRepresentation(kCFAllocatorDefault, object, kCFStringEncodingUTF8, '?');

	if (dataBuffer) {
		length = CFDataGetLength(dataBuffer);
		buffer = (char *) CFDataGetBytePtr(dataBuffer);
	}

	for (i = 0; i < numClasses; i++) {
		if (!strncmp(buffer, classNames[i], length)) {
			matches = true;
			break;
		}
	}

	// this works because all bytes in a multi-byte utf-8 character have the high order bit set
	for (i = 0; (i < length) && succeeded; i++) {
		c = buffer[i];

		switch (c) {
			case '<':
				succeeded = addString("&lt;", state);
				break;
			case '>':
				succeeded = addString("&gt;", state);
				break;
			case '&':
				succeeded = addString("&amp;", state);
				break;
			default:
				succeeded = addChar(c, state);
			    break;
		}
	}

	if (dataBuffer) CFRelease(dataBuffer);

    if (succeeded) {
	    succeeded = addString("</key>", state);
	    if (matches && succeeded) {
		    return addString(getOffMyXMLawn, state);
	    } else {
		    return succeeded;
	    }
    } else {
	    return false;
    }
}

//this was taken from CFPropertyList.c
static const char __CFPLDataEncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static Boolean
DoCFSerializeData(CFDataRef object, IOCFSerializeState * state)
{
	CFIndex	  length;
	const UInt8 * bytes;
        CFIndex i;
        const unsigned char *p;
        unsigned char c = 0;

	if (previouslySerialized(object, state)) return true;

	length = CFDataGetLength(object);
	bytes = CFDataGetBytePtr(object);

        if (!addStartTag(object, 0, state)) return false;

        for (i = 0, p = (unsigned char *)bytes; i < length; i++, p++) {
		/* 3 bytes are encoded as 4 */
		switch (i % 3) {
                case 0:
			c = __CFPLDataEncodeTable [ ((p[0] >> 2) & 0x3f)];
			if (!addChar(c, state)) return false;
			break;
                case 1:
			c = __CFPLDataEncodeTable [ ((((p[-1] << 8) | p[0]) >> 4) & 0x3f)];
			if (!addChar(c, state)) return false;
			break;
                case 2:
			c = __CFPLDataEncodeTable [ ((((p[-1] << 8) | p[0]) >> 6) & 0x3f)];
			if (!addChar(c, state)) return false;
			c = __CFPLDataEncodeTable [ (p[0] & 0x3f)];
			if (!addChar(c, state)) return false;
			break;
		}
        }
        switch (i % 3) {
	case 0:
                break;
	case 1:
                c = __CFPLDataEncodeTable [ ((p[-1] << 4) & 0x30)];
                if (!addChar(c, state)) return false;
                if (!addChar('=', state)) return false;
                if (!addChar('=', state)) return false;
                break;
	case 2:
                c = __CFPLDataEncodeTable [ ((p[-1] << 2) & 0x3c)];
                if (!addChar(c, state)) return false;
                if (!addChar('=', state)) return false;
                break;
        }

	return addEndTag(object, state);
}

static Boolean
DoCFSerializeArray(CFArrayRef object, IOCFSerializeState * state)
{
	CFIndex	count, i;
	Boolean	ok = true;

	if (previouslySerialized(object, state)) return true;

        if (!addStartTag(object, 0, state)) return false;

	count = CFArrayGetCount(object);
	if (count) {
		for (i = 0; ok && (i < count); i++) {
			ok = DoCFSerialize((CFTypeRef) CFArrayGetValueAtIndex(object, i),
					   state);
		}
	}

	return ok && addEndTag(object, state);
}

static Boolean
DoCFSerializeSet(CFSetRef object, IOCFSerializeState * state)
{
	CFIndex	  count, i;
	const void ** values;
	Boolean	  ok = true;

	if (previouslySerialized(object, state)) return true;

        if (!addStartTag(object, 0, state)) return false;

	count = CFSetGetCount(object);

	if (count) {
		values = (const void **) malloc(count * sizeof(void *));
		if (!values)
			return false;
		CFSetGetValues(object, values);
	} else {
		values = 0;
	}

	for (i = 0; ok && (i < count); i++) {
		ok = DoCFSerialize((CFTypeRef) values[i], state);
	}

	if (values)
		free(values);

	return ok && addEndTag(object, state);
}

static Boolean
DoCFSerializeDictionary(CFDictionaryRef object,
				IOCFSerializeState * state)
{
	CFIndex	  count, i;
	const void ** values;
	const void ** keys;
	Boolean	  ok = true;

	if (previouslySerialized(object, state)) return true;

        if (!addStartTag(object, 0, state)) return false;

	count = CFDictionaryGetCount(object);

	if (count) {
		values = (const void **) malloc(2 * count * sizeof(void *));
		if (!values)
			return false;
		keys = values + count;
		CFDictionaryGetKeysAndValues(object, keys, values);
	} else {
		values = keys = 0;
	}

	for (i = 0; ok && (i < count); i++) {
		ok = DoCFSerializeKey((CFTypeRef) keys[i], state);
                if (!ok)
			break;
		if (!(ok = DoCFSerialize((CFTypeRef) values[i], state)))
			break;
	}

	if (values)
		free(values);

	return ok && addEndTag(object, state);
}

static Boolean
DoIdrefScan(CFTypeRef object, IOCFSerializeState * state)
{
	CFTypeID type;
	Boolean	ok = true;  // such an optimist
	CFIndex count, i;
	const void ** keys;
	const void ** values;

	assert(object);

	recordObjectInIDRefDictionary(object, state);

	type = CFGetTypeID(object);

	if (type == CFDictionaryGetTypeID()) {
		CFDictionaryRef dict = (CFDictionaryRef)object;
		count = CFDictionaryGetCount(dict);
		if (count) {
			keys = (const void **)malloc(count * sizeof(void *));
			values = (const void **)malloc(count * sizeof(void *));
			if (!keys || !values) {
				return false;
			}
			CFDictionaryGetKeysAndValues(dict, keys, values);
			for (i = 0; ok && (i < count); i++) {
				// don't record dictionary keys
				ok &= DoIdrefScan((CFTypeRef)values[i], state);
			}

			free(keys);
			free(values);
			if (!ok) {
				return ok;
			}
		}
    } else if (type == CFArrayGetTypeID()) {
		CFArrayRef array = (CFArrayRef)object;
		count = CFArrayGetCount(array);
		for (i = 0; i < count; i++) {
			CFTypeRef element = CFArrayGetValueAtIndex(array, i);
			ok = DoIdrefScan(element, state);
			if (!ok) {
				return ok;
			}
		}
	} else if (type == CFSetGetTypeID()) {
		CFSetRef set = (CFSetRef)object;
		count = CFSetGetCount(set);
		if (count) {
			values = (const void **)malloc(count * sizeof(void *));
			if (!values) {
				return false;
			}
			CFSetGetValues(set, values);
			for (i = 0; ok && (i < count); i++) {
				ok = DoIdrefScan((CFTypeRef)values[i], state);
			}

			free(values);
			if (!ok) {
				return ok;
			}
        }

	}

	return ok;
}

static Boolean
DoCFSerialize(CFTypeRef object, IOCFSerializeState * state)
{
    CFTypeID	type;
    Boolean	ok;

    assert(object);

    type = CFGetTypeID(object);

   /* Sorted by rough order of % occurrence in big plists.
    */
    if (type == CFDictionaryGetTypeID()) {
        ok = DoCFSerializeDictionary((CFDictionaryRef) object, state);
    } else if (type == CFStringGetTypeID()) {
        ok = DoCFSerializeString((CFStringRef) object, state);
    } else if (type == CFArrayGetTypeID()) {
        ok = DoCFSerializeArray((CFArrayRef) object, state);
    } else if (type == CFNumberGetTypeID()) {
        ok = DoCFSerializeNumber((CFNumberRef) object, state);
    } else if (type == CFDataGetTypeID()) {
        ok = DoCFSerializeData((CFDataRef) object, state);
    } else if (type == CFBooleanGetTypeID()) {
        ok = DoCFSerializeBoolean((CFBooleanRef) object, state);
    } else if (type == CFSetGetTypeID()) {
        ok = DoCFSerializeSet((CFSetRef) object, state);
    } else {
        CFStringRef temp = NULL;
        temp = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
				CFSTR("<string>typeID 0x%x not serializable</string>"), (int) type);
        if (temp) {
            ok = DoCFSerializeString(temp, state);
            CFRelease(temp);
        } else {
            ok = false;
        }

    }

    return ok;
}

CFDataRef
IOCFSerialize(CFTypeRef object, CFOptionFlags options)
{
    IOCFSerializeState       state;
    Boolean			         ok   = FALSE;
    CFDictionaryKeyCallBacks idrefKeyCallbacks;

    if (!object) return 0;
#if IOKIT_SERVER_VERSION >= 20140421
    if (kIOCFSerializeToBinary & options) return IOCFSerializeBinary(object, options);
#endif /* IOKIT_SERVER_VERSION >= 20140421 */
    if (options) return 0;

    state.data = CFDataCreateMutable(kCFAllocatorDefault, 0);
    assert(state.data);

    state.idrefNumRefs = 0;

    idrefKeyCallbacks = kCFTypeDictionaryKeyCallBacks;
    // only use pointer equality for these keys
    idrefKeyCallbacks.equal = NULL;

    state.stringIDRefDictionary = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &idrefKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
    assert(state.stringIDRefDictionary);

    state.numberIDRefDictionary = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &idrefKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
    assert(state.numberIDRefDictionary);

    state.dataIDRefDictionary = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &idrefKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
    assert(state.dataIDRefDictionary);

    state.dictionaryIDRefDictionary = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &idrefKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
    assert(state.dictionaryIDRefDictionary);

    state.arrayIDRefDictionary = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &idrefKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
    assert(state.arrayIDRefDictionary);

    state.setIDRefDictionary = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &idrefKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
    assert(state.setIDRefDictionary);

    ok = DoIdrefScan(object, &state);
    if (!ok) {
        goto finish;
    }

    ok = DoCFSerialize(object, &state);

    if (ok) {
        ok = addChar(0, &state);
    }
    if (!ok) {
        goto finish;
    }

finish:
    if (!ok && state.data) {
        CFRelease(state.data);
        state.data = NULL;  // it's returned
    }

    if (state.stringIDRefDictionary)     CFRelease(state.stringIDRefDictionary);
    if (state.numberIDRefDictionary)     CFRelease(state.numberIDRefDictionary);
    if (state.dataIDRefDictionary)       CFRelease(state.dataIDRefDictionary);
    if (state.dictionaryIDRefDictionary) CFRelease(state.dictionaryIDRefDictionary);
    if (state.arrayIDRefDictionary)      CFRelease(state.arrayIDRefDictionary);
    if (state.setIDRefDictionary)        CFRelease(state.setIDRefDictionary);

    return state.data;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if IOKIT_SERVER_VERSION >= 20140421

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#if 0
#define DEBG(fmt, args...)  { printf(fmt, args); }
#else
#define DEBG(fmt, args...)	{}
#endif

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

struct IOCFSerializeBinaryState
{
    CFMutableDataRef       data;
	CFMutableDictionaryRef tags;
    Boolean                endCollection;
    uintptr_t              tag;
};
typedef struct IOCFSerializeBinaryState IOCFSerializeBinaryState;

static Boolean
IOCFSerializeBinaryAdd(IOCFSerializeBinaryState * state, const void * bits, size_t size)
{
	CFDataAppendBytes(state->data, bits, size);
    if (3 & size) CFDataIncreaseLength(state->data, 4 - (3 & size));
	return true;
}

static Boolean
IOCFSerializeBinaryAddObject(IOCFSerializeBinaryState * state,
								  CFTypeRef o, uint32_t key,
								  const void * bits, size_t size, size_t zero)
{
    // add to tag dictionary
	CFDictionarySetValue(state->tags, o, (const void *)state->tag);
	state->tag++;

    if (state->endCollection)
    {
         state->endCollection = false;
         key |= kOSSerializeEndCollecton;
    }

	CFDataAppendBytes(state->data, (const UInt8 *) &key, sizeof(key));
	CFDataAppendBytes(state->data, bits, size - zero);
    if (zero) CFDataIncreaseLength(state->data, zero);
    if (3 & size) CFDataIncreaseLength(state->data, 4 - (3 & size));

	return (true);
}

struct ApplierState
{
    IOCFSerializeBinaryState * state;
    CFIndex                    index;
    CFIndex                    count;
    Boolean					   ok;
};
typedef struct ApplierState ApplierState;

static Boolean
DoCFSerializeBinary(IOCFSerializeBinaryState * state, CFTypeRef o, Boolean isKey);

static void
IOCFSerializeBinaryCFDictionaryFunction(const void *key, const void *value, void *context)
{
    ApplierState * ctx = (typeof(ctx)) context;

    ctx->index++;
	ctx->ok &= DoCFSerializeBinary(ctx->state, key, true);
	ctx->state->endCollection = (ctx->index == ctx->count);
	ctx->ok &= DoCFSerializeBinary(ctx->state, value, false);
}

static void
IOCFSerializeBinaryCFArraySetFunction(const void *value, void *context)
{
    ApplierState * ctx = (typeof(ctx)) context;

    ctx->index++;
	ctx->state->endCollection = (ctx->index == ctx->count);
	ctx->ok &= DoCFSerializeBinary(ctx->state, value, false);
}

static Boolean
DoCFSerializeBinary(IOCFSerializeBinaryState * state, CFTypeRef o, Boolean isKey)
{
    ApplierState applierState;
    Boolean	     ok;
    CFTypeID	 type;
	CFIndex      count;
    uint32_t     key;
    size_t       len;
    uintptr_t    tag;

	// look it up
	tag = (uintptr_t) CFDictionaryGetValue(state->tags, o);
	// does it exist?
	if (tag)
	{
		key = (kOSSerializeObject | (tag & kOSSerializeDataMask));
		if (state->endCollection)
		{
			 state->endCollection = false;
			 key |= kOSSerializeEndCollecton;
		}
		ok = IOCFSerializeBinaryAdd(state, &key, sizeof(key));
		return (ok);
	}

    type = CFGetTypeID(o);
	applierState.state = state;

    if (type == CFDictionaryGetTypeID())
	{
		count = CFDictionaryGetCount(o);
		key = (kOSSerializeDictionary | count);
		ok = IOCFSerializeBinaryAddObject(state, o, key, NULL, 0, 0);
		if (ok)
		{
			applierState.ok    = true;
			applierState.index = 0;
			applierState.count = count;
			CFDictionaryApplyFunction(o, &IOCFSerializeBinaryCFDictionaryFunction, &applierState);
			ok = applierState.ok;
		}
	}
    else if (type == CFArrayGetTypeID())
	{
		count = CFArrayGetCount(o);
		key = (kOSSerializeArray | count);
		ok = IOCFSerializeBinaryAddObject(state, o, key, NULL, 0, 0);
		if (ok)
		{
			applierState.ok    = true;
			applierState.index = 0;
			applierState.count = count;
			CFArrayApplyFunction(o, CFRangeMake(0, count), &IOCFSerializeBinaryCFArraySetFunction, &applierState);
			ok = applierState.ok;
		}
	}
    else if (type == CFSetGetTypeID())
	{
		count = CFArrayGetCount(o);
		key = (kOSSerializeSet | count);
		ok = IOCFSerializeBinaryAddObject(state, o, key, NULL, 0, 0);
		if (ok)
		{
			applierState.ok    = true;
			applierState.index = 0;
			applierState.count = count;
			CFSetApplyFunction(o, &IOCFSerializeBinaryCFArraySetFunction, &applierState);
			ok = applierState.ok;
		}
	}
    else if (type == CFNumberGetTypeID())
	{
		long long value;
		int		  size;

		ok = CFNumberGetValue(o, kCFNumberLongLongType, &value);
		if (ok)
		{
			switch(CFNumberGetType(o))
			{
				case kCFNumberSInt8Type:
				case kCFNumberCharType:
					size = sizeof(SInt8);
					break;

				case kCFNumberSInt16Type:
				case kCFNumberShortType:
					size = sizeof(SInt16);
					break;

				case kCFNumberSInt32Type:
				case kCFNumberIntType:
					size = sizeof(SInt32);
					break;

				case kCFNumberLongType:
					size = sizeof(long);
					break;

				case kCFNumberSInt64Type:
				case kCFNumberLongLongType:
				default:
					size = sizeof(SInt64);
					break;
			}
			key = (kOSSerializeNumber | (size * 8));
			ok = IOCFSerializeBinaryAddObject(state, o, key, &value, sizeof(value), 0);
		}
	}
    else if (type == CFBooleanGetTypeID())
	{
		key = (kOSSerializeBoolean | (kCFBooleanTrue == o));
		ok = IOCFSerializeBinaryAddObject(state, o, key, NULL, 0, 0);
	}
    else if (type == CFStringGetTypeID())
	{
		CFDataRef dataBuffer = 0;
		const char * buffer;
		bool conversionFailed = false;

		if ((buffer = CFStringGetCStringPtr(o, kCFStringEncodingUTF8))) len = CFStringGetLength(o);
		else
		{
			dataBuffer = CFStringCreateExternalRepresentation(kCFAllocatorDefault, o, kCFStringEncodingUTF8, 0);
			if (!dataBuffer)
			{
				dataBuffer = CFStringCreateExternalRepresentation(kCFAllocatorDefault, o, kCFStringEncodingUTF8, (UInt8)'?');
				conversionFailed = true;
			}

			if (dataBuffer)
			{
				len = CFDataGetLength(dataBuffer);
				buffer = (char *) CFDataGetBytePtr(dataBuffer);
			}
			else
			{
				len = 0;
				buffer = "";
				conversionFailed = true;
			}
		}

		if (conversionFailed)
		{
			char * tempBuffer;
			if (buffer && (tempBuffer = malloc(len + 1)))
			{
				bcopy(buffer, tempBuffer, len);
				tempBuffer[len] = 0;
				syslog(LOG_ERR, "FIXME: IOCFSerialize has detected a string that can not be converted to UTF-8, \"%s\"", tempBuffer);
				free(tempBuffer);
			}
		}

		if (isKey)
		{
			len++;
			key = (kOSSerializeSymbol | len);
			ok  = IOCFSerializeBinaryAddObject(state, o, key, buffer, len, 1);
		}
		else
		{
			key = (kOSSerializeString | len);
			ok  = IOCFSerializeBinaryAddObject(state, o, key, buffer, len, 0);
		}

		if (dataBuffer) CFRelease(dataBuffer);
	}
    else if (type == CFDataGetTypeID())
	{
		len = CFDataGetLength(o);
		key = (kOSSerializeData | len);
		ok  = IOCFSerializeBinaryAddObject(state, o, key, CFDataGetBytePtr(o), len, 0);
	}
	else
    {
        CFStringRef temp;
        temp = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
				CFSTR("<string>typeID 0x%x not serializable</string>"), (int) type);
        if ((ok = (NULL != temp)))
        {
            ok = DoCFSerializeBinary(state, temp, false);
            CFRelease(temp);
        }
    }

    return (ok);
}

CFDataRef
IOCFSerializeBinary(CFTypeRef object, CFOptionFlags options __unused)
{
    Boolean ok;
    IOCFSerializeBinaryState state;

    bzero(&state, sizeof(state));

	state.endCollection = true;
    state.data = CFDataCreateMutable(kCFAllocatorDefault, 0);
    assert(state.data);

    CFDictionaryKeyCallBacks keyCallbacks;
    keyCallbacks = kCFTypeDictionaryKeyCallBacks;
    // only use pointer equality for these keys
    keyCallbacks.equal = NULL;

    state.tags = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &keyCallbacks,
        (CFDictionaryValueCallBacks *) NULL);

    assert(state.tags);

	IOCFSerializeBinaryAdd(&state, kOSSerializeBinarySignature, sizeof(kOSSerializeBinarySignature));

	ok = DoCFSerializeBinary(&state, object, false);

    if (!ok && state.data)
    {
        CFRelease(state.data);
        state.data = NULL;  // it's returned
    }
    if (state.tags) CFRelease(state.tags);

    return (state.data);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define setAtIndex(v, idx, o)													    \
	if (idx >= v##Capacity)														    \
	{																			    \
        if (v##Capacity >= v##CapacityMax) ok = false;                              \
        else																		\
        {																			\
            uint32_t ncap = v##Capacity + 64;										\
            typeof(v##Array) nbuf = (typeof(v##Array)) malloc(ncap * sizeof(o));	\
            if (!nbuf) ok = false;													\
            else																	\
            {																		\
                if (v##Array)														\
                {																	\
                    bcopy(v##Array, nbuf, v##Capacity * sizeof(o));					\
                    free(v##Array);							                        \
                }																	\
                v##Array    = nbuf;													\
                v##Capacity = ncap;													\
            }																		\
	    }																			\
	}		    																	\
	if (ok) v##Array[idx] = o;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CFTypeRef
IOCFUnserializeBinary(const char	* buffer,
					  size_t          bufferSize,
					  CFAllocatorRef  allocator,
					  CFOptionFlags	  options __unused,
					  CFStringRef	* errorString)
{
	CFTypeRef * objsArray;
	uint32_t    objsCapacity;
	enum      { objsCapacityMax = 16*1024*1024 };
	uint32_t    objsIdx;

	CFTypeRef * stackArray;
	uint32_t    stackCapacity;
	enum      { stackCapacityMax = 64*1024 };
	uint32_t    stackIdx;

    CFTypeRef              result;
    CFTypeRef              parent;
    CFMutableDictionaryRef dict;
    CFMutableArrayRef      array;
    CFMutableSetRef        set;
    CFMutableDictionaryRef newDict;
    CFMutableArrayRef      newArray;
    CFMutableSetRef        newSet;
    CFTypeRef              o;
    CFStringRef            sym;

    size_t           bufferPos;
    const uint32_t * next;
    uint32_t         key, len, wordLen;
    bool             ok, end, newCollect, isRef;

	CFNumberType 	numType;
    CFTypeID	    type;
	const UInt8 *	bytes;

	if (errorString) *errorString = NULL;
	if (0 != strcmp(kOSSerializeBinarySignature, buffer)) return (NULL);
	if (3 & ((uintptr_t) buffer)) return (NULL);
	if (bufferSize < sizeof(kOSSerializeBinarySignature)) return (NULL);
	bufferPos = sizeof(kOSSerializeBinarySignature);
	next = (typeof(next)) (((uintptr_t) buffer) + sizeof(kOSSerializeBinarySignature));

	DEBG("---------OSUnserializeBinary(%p)\n", buffer);

	objsArray = stackArray    = NULL;
	objsIdx   = objsCapacity  = 0;
	stackIdx  = stackCapacity = 0;

    result   = 0;
    parent   = 0;
	dict     = 0;
	array    = 0;
	set      = 0;
	sym      = 0;

	ok       = true;
	while (ok)
	{
		bufferPos += sizeof(*next);
		if (!(ok = (bufferPos <= bufferSize))) break;
		key = *next++;

        len = (key & kOSSerializeDataMask);
        wordLen = (len + 3) >> 2;
		end = (0 != (kOSSerializeEndCollecton & key));
        DEBG("key 0x%08x: 0x%04x, %d\n", key, len, end);

        newCollect = isRef = false;
		o = 0; newDict = 0; newArray = 0; newSet = 0;

		switch (kOSSerializeTypeMask & key)
		{
		    case kOSSerializeDictionary:
				o = newDict = CFDictionaryCreateMutable(allocator, len,
														&kCFTypeDictionaryKeyCallBacks,
														&kCFTypeDictionaryValueCallBacks);
				newCollect = (len != 0);
		        break;
		    case kOSSerializeArray:
				o = newArray = CFArrayCreateMutable(allocator, len, &kCFTypeArrayCallBacks);
				newCollect = (len != 0);
		        break;
		    case kOSSerializeSet:
				o = newSet = CFSetCreateMutable(allocator, len, &kCFTypeSetCallBacks);
				newCollect = (len != 0);
		        break;

		    case kOSSerializeObject:
				if (len >= objsIdx) break;
				o = objsArray[len];
				isRef = true;
				break;

		    case kOSSerializeNumber:
				bufferPos += sizeof(long long);
				if (bufferPos > bufferSize) break;
				bytes = (const UInt8 *) &next[0];
				if (len <= 32) {
					numType = kCFNumberSInt32Type;
				} else {
					numType = kCFNumberSInt64Type;
				}
				o = CFNumberCreate(allocator, numType, (const void *) bytes);
		    	next += 2;
		        break;

		    case kOSSerializeSymbol:
		    	len--;
		    	/* fall thru */
		    case kOSSerializeString:
				bufferPos += (wordLen * sizeof(uint32_t));
				if (bufferPos > bufferSize) break;
				o = CFStringCreateWithBytes(allocator, (const UInt8 *) next, len, kCFStringEncodingUTF8, false);
				if (!o)
				{
					o = CFStringCreateWithBytes(allocator, (const UInt8 *) next, len, kCFStringEncodingMacRoman, false);
					syslog(LOG_ERR, "FIXME: IOUnserialize has detected a string that is not valid UTF-8, \"%s\".",
									CFStringGetCStringPtr(o, kCFStringEncodingMacRoman));
				}
		        next += wordLen;
		        break;

    	    case kOSSerializeData:
				bufferPos += (wordLen * sizeof(uint32_t));
				if (bufferPos > bufferSize) break;
				o = CFDataCreate(allocator, (const UInt8 *) next, len);
		        next += wordLen;
		        break;

    	    case kOSSerializeBoolean:
				o = (len ? kCFBooleanTrue : kCFBooleanFalse);
				CFRetain(o);
		        break;

		    default:
		        break;
		}

		if (!(ok = (o != 0))) break;

		if (!isRef)
		{
			setAtIndex(objs, objsIdx, o);
			if (!ok)
			{
			     CFRelease(o);
			     break;
            }
			objsIdx++;
		}
		if (dict)
		{
			if (sym)
			{
				if (o != dict) CFDictionarySetValue(dict, sym, o);
				sym = 0;
			}
			else
			{
				assert(CFStringGetTypeID() == CFGetTypeID(o));
				sym = o;
			}
		}
		else if (array)  CFArrayAppendValue(array, o);
		else if (set)    CFSetAddValue(set, o);
		else if (result) ok = false;
		else
		{
		    assert(!parent);
		    result = o;
		}

		if (!ok) break;

		if (newCollect)
		{
			if (!end)
			{
				stackIdx++;
				setAtIndex(stack, stackIdx, parent);
				if (!ok) break;
			}
			DEBG("++stack[%d] %p\n", stackIdx, parent);
			parent = o;
			dict   = newDict;
			array  = newArray;
			set    = newSet;
			end    = false;
		}

		if (end)
		{
			if (!stackIdx) break;
			parent = stackArray[stackIdx];
			DEBG("--stack[%d] %p\n", stackIdx, parent);
			stackIdx--;

			type = CFGetTypeID(parent);
			set   = NULL;
			dict  = NULL;
			array = NULL;
			if (type == CFDictionaryGetTypeID()) dict  = (CFMutableDictionaryRef) parent;
			else if (type == CFArrayGetTypeID()) array = (CFMutableArrayRef)      parent;
			else if (type == CFSetGetTypeID())   set   = (CFMutableSetRef)        parent;
			else                                 ok    = false;
		}
	}

	if (!ok) result = 0;

	if (objsCapacity)
	{
        for (len = (result != 0); len < objsIdx; len++) CFRelease(objsArray[len]);
	    free(objsArray);
    }
	if (stackCapacity) free(stackArray);

	DEBG("ret %p\n", result);

	return (result);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#endif /* IOKIT_SERVER_VERSION >= 20140421 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

CFTypeRef
IOCFUnserializeWithSize(const char	  * buffer,
						size_t          bufferSize,
						CFAllocatorRef	allocator,
						CFOptionFlags	options,
						CFStringRef	  * errorString)
{
 	if (errorString) *errorString = NULL;
	if (!buffer) return 0;

#if IOKIT_SERVER_VERSION >= 20140421
    if (bufferSize < sizeof(kOSSerializeBinarySignature)) return (0);
	if ((kIOCFSerializeToBinary & options)
		|| (!strcmp(kOSSerializeBinarySignature, buffer))) return (IOCFUnserializeBinary(buffer, bufferSize, allocator, options, errorString));
#else
    if (!bufferSize) return (0);
#endif /* IOKIT_SERVER_VERSION >= 20140421 */

	return (IOCFUnserialize(buffer, allocator, options, errorString));
}
