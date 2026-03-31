/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#include "DirServices.h"
#include "DirServicesConst.h"
#include "DirServicesUtils.h"
#include "DirServicesTypes.h"
#include "DirServicesTypesPriv.h"
#include "DirServicesPriv.h"
#include "DirServicesUtilsPriv.h"
#include <dispatch/dispatch.h>
#include <OpenDirectory/OpenDirectory.h>
#include <OpenDirectory/OpenDirectoryPriv.h>
#include <syslog.h>
#include <sys/queue.h>
#include "internal.h"

typedef enum {
	eDirReference			= 1,
	eNodeReference,
	eRecordReference,
	eAttributeListReference,
	eAttributeValueListReference,
} eReferenceType;

typedef enum {
	eQueryContinue			= 10,
	eAuthContinue			= 11,

} eContinueType;

struct sQueryContinue {
	dispatch_queue_t			queue;
	ODQueryRef					query;
	CFMutableArrayRef			results;
	CFArrayRef					attributes;
	dispatch_semaphore_t		semaphore;
	bool						done;
	tDirStatus					status;
};

struct sAuthContinue {
	ODContextRef				context;
};

struct sContinue {
	TAILQ_ENTRY(sContinue)		tailq;
	tContextData				continue_id;
	eContinueType				type;

	union {
		struct sQueryContinue	query;
		struct sAuthContinue	auth;
	};
};

static UInt32					refCounter;
static TAILQ_HEAD(, sContinue)	query_continue	= TAILQ_HEAD_INITIALIZER(query_continue);

static dispatch_queue_t
_get_continue_queue(void)
{
	static dispatch_queue_t	queue;
	static dispatch_once_t once;

	dispatch_once(&once, ^(void) {
		queue = dispatch_queue_create("com.apple.DirectoryService.queryrefqueue", NULL);
	});

	return queue;
}

static struct sContinue *
_get_continue_data(tContextData context) {
	dispatch_queue_t			queue		= _get_continue_queue();
	__block struct sContinue 	*contdata	= NULL;

	// not super efficient, but shouldn't have that many inflight queries
	dispatch_sync(queue, ^(void) {
		TAILQ_FOREACH(contdata, &query_continue, tailq) {
			if (contdata->continue_id == context) {
				break;
			}
		};
	});

	return contdata;
}

static tDirStatus
_remove_continue_data(tContextData context)
{
	dispatch_queue_t	queue	= _get_continue_queue();
	__block tDirStatus	status	= eDSInvalidReference;

	// not super efficient, but shouldn't have that many inflight queries
	dispatch_sync(queue, ^(void) {
		struct sContinue 	*contdata	= NULL;

		TAILQ_FOREACH(contdata, &query_continue, tailq) {
			if (contdata->continue_id == context) {
				TAILQ_REMOVE(&query_continue, contdata, tailq);
				status = eDSNoErr;
				break;
			}
		};

		if (contdata != NULL) {
			switch (contdata->type) {
				case eQueryContinue:
					ODQuerySetDispatchQueue(contdata->query.query, NULL); // this cancels the callback
					dispatch_sync(contdata->query.queue, ^ {
						// need to sync in case we're in the middle of the callback
						safe_cfrelease_null(contdata->query.query);
						safe_cfrelease_null(contdata->query.results);
						safe_cfrelease_null(contdata->query.attributes);
						dispatch_release(contdata->query.semaphore);
					});
					dispatch_release(contdata->query.queue);
					free(contdata);
					break;

				case eAuthContinue:
					safe_cfrelease_null(contdata->auth.context);
					free(contdata);
					break;
			}
		}
	});

	return status;
}

static struct sContinue *
_create_continue_data(eContinueType type) {
	static uint32_t				lastid		= 0;
	dispatch_queue_t			queue		= _get_continue_queue();
	__block struct sContinue 	*contdata	= NULL;

	// not super efficient, but shouldn't have that many inflight queries
	dispatch_sync(queue, ^(void) {
		uint32_t	newid	= 0;

		while (newid == 0) {
			lastid++;

			TAILQ_FOREACH(contdata, &query_continue, tailq) {
				if (contdata->continue_id == lastid) {
					break;
				}
			};

			if (contdata == NULL) {
				newid = lastid;
				break;
			}
		}

		contdata = calloc(1, sizeof(struct sContinue));
		contdata->continue_id = newid;
		contdata->type = type;

		switch (type) {
			case eQueryContinue:
				contdata->query.queue = dispatch_queue_create("com.apple.DirectoryService.query_continuedata", NULL);
				contdata->query.results = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
				contdata->query.semaphore = dispatch_semaphore_create(0);
				break;

			case eAuthContinue:
				break;
		}

		TAILQ_INSERT_HEAD(&query_continue, contdata, tailq);
	});

	return contdata;
}

static dispatch_queue_t
_get_queue(void)
{
	static dispatch_queue_t	queue;
	static dispatch_once_t once;

	dispatch_once(&once, ^(void) {
		queue = dispatch_queue_create("com.apple.DirectoryService.refqueue", NULL);
	});

	return queue;
}

static CFMutableDictionaryRef
_get_container(void)
{
	static CFMutableDictionaryRef refContainer;
	static dispatch_once_t once;

	dispatch_once(&once, ^(void) {
		refContainer = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, &kCFTypeDictionaryValueCallBacks);
	});

	return refContainer;
}

static UInt32
_add_reference(CFTypeRef object)
{
	__block UInt32 reference = 0;

	if (object != NULL) {
		dispatch_sync(_get_queue(), ^(void) {
			CFMutableDictionaryRef container = _get_container();

			do {
				refCounter++;
				if (refCounter == 0) {
					refCounter++;
				}

				if (CFDictionaryContainsKey(container, (void *)(long)refCounter) == FALSE) {
					CFDictionarySetValue(container, (void *)(long)refCounter, object);
					reference = refCounter;
					break;
				}
			} while (1);
		});
	}

	return reference;
}

static tDirStatus
_remove_reference(eReferenceType type, UInt32 reference)
{
	__block tDirStatus	status = eDSInvalidRefType;

	dispatch_sync(_get_queue(), ^(void) {
		CFMutableDictionaryRef container = _get_container();
		CFTypeRef object = CFDictionaryGetValue(container, (void *)(long) reference);

		if (object != NULL) {
			switch (type) {
				case eDirReference:
					if (CFGetTypeID(object) != ODSessionGetTypeID()) {
						object = NULL;
					}
					break;

				case eNodeReference:
					if (CFGetTypeID(object) != ODNodeGetTypeID()) {
						object = NULL;
					}
					break;

				case eRecordReference:
					if (CFGetTypeID(object) != ODRecordGetTypeID()) {
						object = NULL;
					}
					break;

				default:
					break;
			}

			if (object != NULL) {
				CFDictionaryRemoveValue(container, (void *)(long)reference);
				status = eDSNoErr;
			}
		} else {
			status = eDSInvalidReference;
		}
	});

	return status;
}

static CFTypeRef
_get_reference(eReferenceType type, UInt32 reference)
{
	__block CFTypeRef object;

	dispatch_sync(_get_queue(), ^(void) {
		object = CFDictionaryGetValue(_get_container(), (void *)(long)reference);
	});

	if (object != NULL) {
		switch (type) {
			case eDirReference:
				if (CFGetTypeID(object) != ODSessionGetTypeID()) {
					object = NULL;
				}
				break;

			case eNodeReference:
				if (CFGetTypeID(object) != ODNodeGetTypeID()) {
					object = NULL;
				}
				break;

			case eRecordReference:
				if (CFGetTypeID(object) != ODRecordGetTypeID()) {
					object = NULL;
				}
				break;

			default:
				break;
		}
	}

	return object;
}

static tDirStatus
_put_propertylist_in_buffer(CFPropertyListRef propertyList, tDataBufferPtr outDataBuff)
{
	CFDataRef	data	= CFPropertyListCreateData(kCFAllocatorDefault, propertyList, kCFPropertyListBinaryFormat_v1_0, 0, NULL);
	UInt32		length	= (UInt32) CFDataGetLength(data);
	tDirStatus	status	= eDSBufferTooSmall;

	if (length < outDataBuff->fBufferSize) {
		CFDataGetBytes(data, CFRangeMake(0, length), (UInt8 *)outDataBuff->fBufferData);
		outDataBuff->fBufferLength = length;
		status = eDSNoErr;
	}

	safe_cfrelease_null(data);

	return status;
}

CF_RETURNS_RETAINED
static CFPropertyListRef
_create_propertylist_from_buffer(tDataBufferPtr outDataBuff)
{
	CFPropertyListRef	plist = NULL;

	if (outDataBuff != NULL) {
		CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (UInt8 *) outDataBuff->fBufferData, outDataBuff->fBufferLength, kCFAllocatorNull);

		plist = CFPropertyListCreateWithData(kCFAllocatorDefault, data, kCFPropertyListImmutable, NULL, NULL);
		safe_cfrelease_null(data);
	}

	return plist;
}

static tDirStatus
_pack_array_in_databuffer(CFArrayRef array, tDataBufferPtr outDataBuff)
{
	CFIndex		count	= (array != NULL ? CFArrayGetCount(array) : 0);
	UInt32		maxLen	= outDataBuff->fBufferSize;
	char 		*buffer	= outDataBuff->fBufferData;
	tDirStatus	status	= eDSNoErr;

	if (count == 0) {
		return eDSNoErr;
	} else if (maxLen < (count * sizeof(UInt32))) {
		return eDSBufferTooSmall;
	}

	for (CFIndex ii = 0; ii < count; ii++) {
		CFTypeRef   cfRef		= CFArrayGetValueAtIndex(array, ii);
		UInt32 	*len_ptr		= (UInt32 *)buffer;
		char 		*buff_off	= &buffer[sizeof(UInt32)];

		if (CFGetTypeID(cfRef) == CFStringGetTypeID()) {
			UInt32	length;

			if (CFStringGetCString(cfRef, buff_off, maxLen - sizeof(UInt32), kCFStringEncodingUTF8) == FALSE) {
				status = eDSBufferTooSmall;
				break;
			}

			length = strlen(buff_off) + 1;
			(*len_ptr) = length;

			buffer += sizeof(UInt32) + length;
			maxLen -= sizeof(UInt32) + length;
		} else if (CFGetTypeID(cfRef) == CFDataGetTypeID()) {
			char 	*temp	= (char *) CFDataGetBytePtr(cfRef);
			UInt32	tempLen	= CFDataGetLength(cfRef);

			if (tempLen + sizeof(UInt32) > maxLen) {
				status = eDSBufferTooSmall;
				break;
			}

			(*len_ptr) = tempLen;
			bcopy(temp, buff_off, tempLen);

			buffer += sizeof(UInt32) + tempLen;
			maxLen -= sizeof(UInt32) + tempLen;
		}
	}

	outDataBuff->fBufferLength = buffer - outDataBuff->fBufferData;

	return status;
}

CF_RETURNS_RETAINED
static CFTypeRef
_create_cftype_from_datanode(tDataNodePtr node)
{
	CFTypeRef	value = NULL;

	if (node != NULL) {
		value = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault, (UInt8 *)node->fBufferData, node->fBufferLength, kCFStringEncodingUTF8,
		                                      false, kCFAllocatorNull);

		if (value == NULL) {
			value = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (UInt8 *)node->fBufferData, node->fBufferLength, kCFAllocatorNull);
		}
	}

	return value;
}

static void
_query_callback(ODQueryRef query, CFArrayRef results, CFErrorRef error, void *context)
{
	struct sContinue 	*contdata	= (struct sContinue *) context;
	CFIndex				count		= (results != NULL ? CFArrayGetCount(results) : 0);

	if (count > 0) {
		assert(contdata->query.results != NULL);
		CFArrayAppendArray(contdata->query.results, results, CFRangeMake(0, count));

		// we signal for the total amount of results
		for (int ii = 0; ii < count; ii++) {
			dispatch_semaphore_signal(contdata->query.semaphore);
		}
	} else {
		if (error != NULL) {
			contdata->query.status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		}

		contdata->query.done = true;
		dispatch_semaphore_signal(contdata->query.semaphore);
	}
}

static tDirStatus
_continue_query(struct sContinue *context, tDataBufferPtr inOutDataBuff, UInt32 *outRecEntryCount, bool inAttrInfoOnly, bool *is_done)
{
	__block tDirStatus	status	= eDSNoErr;

	// TODO: do we care about inAttribInfoOnly?
	if (context->query.done == false) {
		dispatch_semaphore_wait(context->query.semaphore, DISPATCH_TIME_FOREVER);
	}

	dispatch_sync(context->query.queue, ^(void) {
		CFIndex	count = CFArrayGetCount(context->query.results);

		if (context->query.status != 0) {
			(*outRecEntryCount) = 0;
			(*is_done) = context->query.done;
			status = context->query.status;
		}

		if (count > 0) {
			ODRecordRef				record	= (ODRecordRef) CFArrayGetValueAtIndex(context->query.results, 0);
			CFMutableDictionaryRef	result	= CFDictionaryCreateMutable(kCFAllocatorDefault, 3, &kCFTypeDictionaryKeyCallBacks,
			                                                            &kCFTypeDictionaryValueCallBacks);
			CFStringRef				recName	= ODRecordGetRecordName(record);
			CFStringRef				recType	= ODRecordGetRecordType(record);
			CFDictionaryRef			attribs = ODRecordCopyDetails(record, context->query.attributes, NULL);

			assert(recName != NULL);
			assert(recType != NULL);
			assert(attribs != NULL);

			CFDictionarySetValue(result, kODAttributeTypeRecordName, recName);
			CFDictionarySetValue(result, kODAttributeTypeRecordType, recType);
			CFDictionarySetValue(result, CFSTR("attributes"), attribs);

			status = _put_propertylist_in_buffer(result, inOutDataBuff);
			if (status == eDSNoErr) {
				CFArrayRemoveValueAtIndex(context->query.results, 0);
				(*outRecEntryCount) = 1;
				(*is_done) = context->query.done && (count == 1);
			} else {
				(*outRecEntryCount) = 0;
			}

			safe_cfrelease_null(attribs);
			safe_cfrelease_null(result);
		} else {
			(*outRecEntryCount) = 0;
			(*is_done) = context->query.done;
		}
	});

	return status;
}

static UInt32
_calculate_CRC32(CFTypeRef value)
{
	static const SInt32 cr3tab[] = { /* CRC polynomial 0xedb88320 */
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
		0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
		0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
		0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
		0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
		0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
		0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
		0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
		0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
		0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
		0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
		0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
		0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
		0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
		0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
		0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
		0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
		0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};

	UInt32	(^calcCRC)(const unsigned char *, size_t) = ^(const unsigned char *str, size_t len) {
		UInt32	uiCRC	= 0xFFFFFFFF;

		for (UInt32 i = 0; i < len; ++i) {
			uiCRC = (cr3tab[(uiCRC ^ (*str)) & 0xff] ^ ((uiCRC >> 8) & 0x00FFFFFF));
			str++;
		}

		return uiCRC;
	};

	if (CFGetTypeID(value) == CFStringGetTypeID()) {
		CFIndex size		= CFStringGetMaximumSizeForEncoding(CFStringGetLength(value), kCFStringEncodingUTF8) + 1;
		char	*tempStr	= calloc(1, size);
		UInt32	uiCRC;

		CFStringGetCString(value, tempStr, size, kCFStringEncodingUTF8);

		uiCRC = calcCRC((unsigned char *)tempStr, strlen(tempStr));

		free(tempStr);

		return uiCRC;
	} else if (CFGetTypeID(value) == CFDataGetTypeID()) {
		return calcCRC(CFDataGetBytePtr(value), CFDataGetLength(value));
	}

	return 0xFFFFFFFF;
}

CF_RETURNS_RETAINED
static CFTypeRef
_find_value_in_record(ODRecordRef record, CFStringRef attribute, UInt32 inAttrValueID)
{
	CFArrayRef	values	= ODRecordCopyValues(record, attribute, NULL);
	CFTypeRef	value	= NULL;

	if (values != NULL) {
		CFIndex		count	= CFArrayGetCount(values);

		for (CFIndex ii = 0; ii < count; ii++) {
			CFTypeRef tempValue = CFArrayGetValueAtIndex(values, ii);
			if (_calculate_CRC32(tempValue) == inAttrValueID) {
				value = CFRetain(tempValue);
				break;
			}
		}

		safe_cfrelease_null(values);
	}

	return value;
}

static tAttributeValueEntryPtr
_create_attribute_value_entry(CFTypeRef value)
{
	tAttributeValueEntryPtr	valueEntry	= NULL;
	CFIndex					length		= 0;
	CFIndex					size		= 0;

	if (CFGetTypeID(value) == CFDataGetTypeID()) {
		length = size = CFDataGetLength(value);
		valueEntry = calloc(1, sizeof(tAttributeValueEntry) + size);

		CFDataGetBytes(value, CFRangeMake(0, length), (UInt8 *) valueEntry->fAttributeValueData.fBufferData);
	} else {
		size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(value), kCFStringEncodingUTF8) + 1;
		valueEntry = calloc(1, sizeof(tAttributeValueEntry) + size);

		CFStringGetCString(value, valueEntry->fAttributeValueData.fBufferData, size, kCFStringEncodingUTF8);
		length = strlen(valueEntry->fAttributeValueData.fBufferData);
	}

	valueEntry->fAttributeValueData.fBufferSize = size;
	valueEntry->fAttributeValueData.fBufferLength = length;
	valueEntry->fAttributeValueID = _calculate_CRC32(value);

	return valueEntry;
}

CF_RETURNS_RETAINED
static CFArrayRef
_convert_datalist_to_cfarray(tDataList *inDataList, Boolean allowEmpty)
{
	CFMutableArrayRef   cfArray		= CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	tDataBufferPriv     *dsDataNode	= (tDataBufferPriv *)(NULL != inDataList ? inDataList->fDataListHead : NULL);

	while (NULL != dsDataNode) {
		if (NULL != dsDataNode->fBufferData && (allowEmpty || dsDataNode->fBufferLength != 0)) {
			CFTypeRef value = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *) dsDataNode->fBufferData, dsDataNode->fBufferLength,
			                                          kCFStringEncodingUTF8, FALSE);
			if (value == NULL) {
				value = CFDataCreate(kCFAllocatorDefault, (const UInt8 *) dsDataNode->fBufferData, dsDataNode->fBufferLength);
			}

			if (NULL != value) {
				CFArrayAppendValue(cfArray, value);
				safe_cfrelease_null(value);
			}
		}

		dsDataNode = (tDataBufferPriv *) dsDataNode->fNextPtr;
	}

	return cfArray;
}

CF_RETURNS_RETAINED
static CFArrayRef
_convert_authbuffer_to_cfarray(CFStringRef authMethod, tDataBufferPtr inAuthBuff)
{
	CFArrayRef itemArray = NULL;

	if (CFEqual(authMethod, kODAuthenticationType2WayRandom) == TRUE) {
		CFTypeRef data = _create_cftype_from_datanode(inAuthBuff);
		itemArray = CFArrayCreate(kCFAllocatorDefault, &data, 1, &kCFTypeArrayCallBacks);
		safe_cfrelease_null(data);
	} else if (CFEqual(authMethod, CFSTR("dsAuthMethodStandard:GSSAPI")) == TRUE) {
		/* GSS auth buffer is slightly different, first value has no length, it's a fixed 4 bytes
		 * 4 bytes  = export security context back to calling application
		 * 4 bytes  = length of service principal string
		 * string   = service principal string
		 * 4 bytes  = length of incoming key block
		 * r        = incoming key block
		 */

		if (inAuthBuff->fBufferLength > (4 + 4 + 4)) {
			CFMutableArrayRef	workingArray	= CFArrayCreateMutable(kCFAllocatorDefault, 3, &kCFTypeArrayCallBacks);
			const char			*buffer			= inAuthBuff->fBufferData;
			const char			*end_buffer		= inAuthBuff->fBufferData + inAuthBuff->fBufferLength;
			CFDataRef			cfData;
			uint32_t			len;

			// add the export security context flag
			cfData = CFDataCreate(kCFAllocatorDefault, (UInt8 *) buffer, 4);
			CFArrayAppendValue(workingArray, cfData);
			safe_cfrelease_null(cfData);
			buffer += 4;

			// add the service principal as a string
			if (buffer < end_buffer) {
				len = *((uint32_t *) buffer);
				buffer += 4;

				if ((buffer + len) <= end_buffer) {
					CFStringRef cfString = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *) buffer, len, kCFStringEncodingUTF8, FALSE);
					CFArrayAppendValue(workingArray, cfString);
					safe_cfrelease_null(cfString);
				}

				buffer += len;
			}

			// add the key block as binary data
			if (buffer < end_buffer) {
				len = *((uint32_t *) buffer);
				buffer += 4;

				if ((buffer + len) <= end_buffer) {
					cfData = CFDataCreate(kCFAllocatorDefault, (UInt8 *) buffer, len);
					CFArrayAppendValue(workingArray, cfData);
					safe_cfrelease_null(cfData);
				}
			}

			itemArray = workingArray;
		}
	} else if (CFStringHasPrefix(authMethod, CFSTR("dsAuthMethodNative:")) == TRUE || CFEqual(authMethod, kODAuthenticationTypeWithAuthorizationRef) == TRUE) {
		CFDataRef	cfData = CFDataCreate(kCFAllocatorDefault, (UInt8 *) inAuthBuff->fBufferData, inAuthBuff->fBufferLength);

		itemArray = CFArrayCreate(kCFAllocatorDefault, (const void **) &cfData, 1, &kCFTypeArrayCallBacks);
		safe_cfrelease_null(cfData);
	} else {
		CFMutableArrayRef	workingArray	= CFArrayCreateMutable(kCFAllocatorDefault, 3, &kCFTypeArrayCallBacks);
		const char			*buffer			= inAuthBuff->fBufferData;
		const char			*end_buffer		= inAuthBuff->fBufferData + inAuthBuff->fBufferLength;
		CFStringRef			cfString;
		CFDataRef			cfData;
		uint32_t			len;

		while (buffer < end_buffer) {
			len = *((uint32_t *) buffer);
			buffer += sizeof(uint32_t);

			if ((buffer + len) <= end_buffer) {
				cfString = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *) buffer, len, kCFStringEncodingUTF8, FALSE);
				if (cfString != NULL) {
					CFArrayAppendValue(workingArray, cfString);
					safe_cfrelease_null(cfString);
				} else {
					cfData = CFDataCreate(kCFAllocatorDefault, (UInt8 *) buffer, len);
					CFArrayAppendValue(workingArray, cfData);
					safe_cfrelease_null(cfData);
				}
			}

			buffer += len;
		}

		itemArray = workingArray;
	}

	return itemArray;
}

/*
 * _copy_filtered_node_details
 * support for dsGetDirNodeInfo, see 7629676
 */
CF_RETURNS_RETAINED
static CFDictionaryRef
_copy_filtered_node_details(CFDictionaryRef details, CFArrayRef keys)
{
	CFRange range;
	Boolean standard, native;
	CFIndex i, count;
	CFTypeRef *dk, *dv;
	CFMutableDictionaryRef result = NULL;

	if (keys == NULL) {
		return CFRetain(details);
	}

	range = CFRangeMake(0, CFArrayGetCount(keys));

	if (CFArrayContainsValue(keys, range, kODAttributeTypeAllAttributes)) {
		return CFRetain(details);
	}

	standard = CFArrayContainsValue(keys, range, kODAttributeTypeStandardOnly);
	native = CFArrayContainsValue(keys, range, kODAttributeTypeNativeOnly);

	if (standard && native) {
		return CFRetain(details);
	}

	count = CFDictionaryGetCount(details);
	dk = alloca(sizeof(CFTypeRef) * count);
	dv = alloca(sizeof(CFTypeRef) * count);
	CFDictionaryGetKeysAndValues(details, dk, dv);

	result = CFDictionaryCreateMutable(NULL, count, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	for (i = 0; i < count; i++) {
		if (CFArrayContainsValue(keys, range, dk[i])
			|| (standard && CFStringHasPrefix(dk[i], CFSTR(kDSStdAttrTypePrefix)))
			|| (native && CFStringHasPrefix(dk[i], CFSTR(kDSNativeAttrTypePrefix)))) {
			CFDictionarySetValue(result, dk[i], dv[i]);
		}
	}

	return result;
}

#pragma mark -
#pragma mark SPIs

CF_RETURNS_RETAINED
CFTypeRef
dsCopyDataFromNodeRef(tDirNodeReference inNodeRef)
{
	ODNodeRef nodeRef = (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	if (nodeRef != NULL) {
		CFRetain(nodeRef);
	}

	return nodeRef;
}

CF_RETURNS_RETAINED
CFTypeRef
dsCopyDataFromDirRef(tDirReference inDirRef)
{
	ODSessionRef sessionRef = (ODSessionRef) _get_reference(eDirReference, inDirRef);
	if (sessionRef != NULL) {
		CFRetain(sessionRef);
	}

	return sessionRef;
}

tDirNodeReference
dsCreateNodeRefData(CFTypeRef data)
{
	return _add_reference(data);
}

tDirReference
dsCreateDataDirRefData(CFTypeRef data)
{
	return _add_reference(data);
}

#pragma mark -
#pragma mark Directory APIs

tDirStatus
dsOpenDirService(tDirReference *outDirRef)
{
	tDirStatus		status	= eDSNoErr;
	CFErrorRef		error	= NULL;
	ODSessionRef	session = ODSessionCreate(kCFAllocatorDefault, NULL, &error);

	if (session != NULL) {
		(*outDirRef) = _add_reference(session);
		CFRelease(session);
	} else {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	}

	return status;
}

tDirStatus
dsOpenDirServiceProxy(tDirReference *outDirRef, const char *inHostOrIPAddress, UInt32 inIPPort, tDataNodePtr inAuthMethod,
                      tDataBufferPtr inAuthStepData, tDataBufferPtr outAuthStepDataResponse, tContextData *ioContinueData)
{
	tDirStatus				status	= eDSNoErr;
	CFMutableDictionaryRef	options	= CFDictionaryCreateMutable(kCFAllocatorDefault, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFErrorRef				error	= NULL;
	ODSessionRef			session	= NULL;
	CFTypeRef				temp	= NULL;

	if (inHostOrIPAddress != NULL) {
		temp = CFStringCreateWithCString(kCFAllocatorDefault, inHostOrIPAddress, kCFStringEncodingUTF8);
		CFDictionarySetValue(options, kODSessionProxyAddress, temp);
		safe_cfrelease_null(temp);
	} else {
		status = eDSNullParameter;
		goto bail;
	}

	temp = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inIPPort);
	CFDictionarySetValue(options, kODSessionProxyPort, temp);
	safe_cfrelease_null(temp);

	if (inAuthStepData != NULL) {
		CFArrayRef array = _convert_authbuffer_to_cfarray(kODAuthenticationTypeClearText, inAuthStepData);

		if (CFArrayGetCount(array) == 2) {
			CFDictionarySetValue(options, kODSessionProxyUsername, CFArrayGetValueAtIndex(array, 0));
			CFDictionarySetValue(options, kODSessionProxyPassword, CFArrayGetValueAtIndex(array, 1));
		} else {
			status = eDSInvalidBuffFormat;
			safe_cfrelease_null(array);
			goto bail;
		}
		safe_cfrelease_null(array);
	} else {
		status = eDSInvalidBuffFormat;
		goto bail;
	}

	session = ODSessionCreate(kCFAllocatorDefault, options, &error);
	if (session != NULL) {
		(*outDirRef) = _add_reference(session);
		CFRelease(session);
		status = eDSNoErr;
	} else {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	}

bail:
	safe_cfrelease_null(options);
	if (ioContinueData != NULL) {
		(*ioContinueData) = 0; // we don't use continue data
	}

	return status;
}

tDirStatus
dsOpenDirServiceLocal(tDirReference *outDirRef, const char *inFilePath)
{
	tDirStatus				status	= eDSNoErr;
	CFMutableDictionaryRef	options	= CFDictionaryCreateMutable(kCFAllocatorDefault, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFStringRef				path	= CFStringCreateWithCString(kCFAllocatorDefault, inFilePath, kCFStringEncodingUTF8);
	CFErrorRef				error	= NULL;
	ODSessionRef			session	= NULL;

	CFDictionarySetValue(options, kODSessionLocalPath, path);
	safe_cfrelease_null(path);

	session = ODSessionCreate(kCFAllocatorDefault, options, &error);
	if (session != NULL) {
		(*outDirRef) = _add_reference(session);
		CFRelease(session);
		status = eDSNoErr;
	} else {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	}

	safe_cfrelease_null(options);

	return status;
}

tDirStatus
dsCloseDirService(tDirReference inDirRef)
{
	return _remove_reference(eDirReference, inDirRef);
}

tDirStatus
dsAddChildPIDToReference(tDirReference inDirRef, SInt32 inValidChildPID, UInt32 inValidAPIReferenceToGrantChild)
{
	return eNoLongerSupported;
}

tDirStatus
dsVerifyDirRefNum(tDirReference inDirRef)
{
	if (_get_reference(0, inDirRef) != NULL) {
		return eDSNoErr;
	}

	return eDSInvalidReference;
}

tDirStatus
dsReleaseContinueData(tDirReference inDirReference, tContextData inContinueData)
{
	return _remove_continue_data(inContinueData);
}

#pragma mark -
#pragma mark Node APIs

tDirStatus
dsGetDirNodeCount(tDirReference inDirRef, UInt32 *outNodeCount)
{
	tDirStatus		status	= eDSInvalidReference;
	ODSessionRef	session = (ODSessionRef) _get_reference(eDirReference, inDirRef);
	CFErrorRef		error	= NULL;

	if (outNodeCount == NULL) {
		return eDSNullParameter;
	}

	if (session != NULL) {
		CFArrayRef nodes = ODSessionCopyNodeNames(kCFAllocatorDefault, session, &error);
		if (nodes != NULL) {
			(*outNodeCount) = CFArrayGetCount(nodes);
			safe_cfrelease_null(nodes);
			status = eDSNoErr;
		} else if (error != NULL) {
			status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
			safe_cfrelease_null(error);
		} else {
			status = eDSNoErr;
		}
	}

	return status;
}

tDirStatus
dsGetDirNodeCountWithInfo(tDirReference inDirRef, UInt32 *outNodeCount, UInt32 *outDirectoryNodeChangeToken)
{
	static UInt32	token;
	tDirStatus		status = dsGetDirNodeCount(inDirRef, outNodeCount);

	if (status == eDSNoErr) {
		// TODO: watch for node changes so we can up an internal value
		(*outDirectoryNodeChangeToken) = ++token;
	}

	return status;
}

tDirStatus
dsGetDirNodeList(tDirReference inDirRef, tDataBufferPtr outDataBuff, UInt32 *outNodeCount, tContextData *ioContinueData)
{
	tDirStatus		status	= eDSInvalidReference;
	ODSessionRef	session = (ODSessionRef) _get_reference(eDirReference, inDirRef);
	CFErrorRef		error	= NULL;
	CFArrayRef		nodes	= NULL;

	if (session == NULL) {
		return eDSInvalidReference;
	} else if (outDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (outNodeCount == NULL) {
		return eDSNullParameter;
	}

	nodes = ODSessionCopyNodeNames(kCFAllocatorDefault, session, &error);
	if (nodes != NULL) {
		status = _put_propertylist_in_buffer(nodes, outDataBuff);
		if (status == eDSNoErr) {
			(*outNodeCount) = CFArrayGetCount(nodes);
		}

		safe_cfrelease_null(nodes);
	} else if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	} else {
		status = eDSNoErr;
	}

	if (ioContinueData != NULL) {
		(*ioContinueData) = 0; // we don't use continue data
	}

	return status;
}

tDirStatus
dsFindDirNodes(tDirReference inDirRef, tDataBufferPtr outDataBuff, tDataListPtr inNodeNamePattern, tDirPatternMatch inPatternMatchType,
               UInt32 *outDirNodeCount, tContextData *ioContinueData)
{
	ODSessionRef		session	= (ODSessionRef) _get_reference(eDirReference, inDirRef);
	CFStringRef			name	= NULL;
	tDirStatus			status	= eDSNoErr;
	ODNodeRef			node	= NULL;
	CFArrayRef			temp	= NULL;
	CFArrayRef			array	= NULL;
	CFMutableArrayRef	nodes	= NULL;
	CFIndex				count;

	if (session == NULL) {
		return eDSInvalidReference;
	} else if (outDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (outDirNodeCount == NULL) {
		return eDSNullParameter;
	}

	array	= _convert_datalist_to_cfarray(inNodeNamePattern, FALSE);
	if (array != NULL) {
		CFStringRef tempName = CFStringCreateByCombiningStrings(kCFAllocatorDefault, array, CFSTR("/"));
		name = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("/%@"), tempName);
		safe_cfrelease_null(tempName);
	}

	nodes = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	switch (inPatternMatchType) {
		case eDSConfigNodeName:
			CFArrayAppendValue(nodes, CFSTR("/Configure"));
			break;

		case eDSLocalNodeNames:
		case eDSLocalHostedNodes:
			CFArrayAppendValue(nodes, CFSTR("/Local/Default"));
			break;

		case eDSAuthenticationSearchNodeName:
		case eDSNetworkSearchNodeName:
			CFArrayAppendValue(nodes, CFSTR("/Search"));
			break;

		case eDSContactsSearchNodeName:
			CFArrayAppendValue(nodes, CFSTR("/Contacts"));
			break;

		case eDSDefaultNetworkNodes:
			// get subnodes of "/Search"
			node = ODNodeCreateWithName(kCFAllocatorDefault, session, CFSTR("/Search"), NULL);
			temp = ODNodeCopySubnodeNames(node, NULL);
			CFArrayAppendArray(nodes, temp, CFRangeMake(0, CFArrayGetCount(temp)));
			safe_cfrelease_null(node);
			break;

		case eDSCacheNodeName:
			CFArrayAppendValue(nodes, CFSTR("/Cache"));
			break;

		case eDSiExact:
			temp = ODSessionCopyNodeNames(kCFAllocatorDefault, session, NULL);
			count = CFArrayGetCount(temp);
			for (CFIndex ii = 0; ii < count; ii++) {
				CFStringRef tempName = CFArrayGetValueAtIndex(temp, ii);
				if (CFStringGetTypeID() == CFGetTypeID(tempName) && CFStringCompare(tempName, name, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
					CFArrayAppendValue(nodes, tempName);
					break;
				}
			}
			break;

		case eDSExact:
			temp = ODSessionCopyNodeNames(kCFAllocatorDefault, session, NULL);
			if (CFArrayContainsValue(temp, CFRangeMake(0, CFArrayGetCount(temp)), name) == TRUE) {
				CFArrayAppendValue(nodes, name);
			}
			break;

		case eDSiStartsWith:
			temp = ODSessionCopyNodeNames(kCFAllocatorDefault, session, NULL);
			count = CFArrayGetCount(temp);
			for (CFIndex ii = 0; ii < count; ii++) {
				CFStringRef tempName = CFArrayGetValueAtIndex(temp, ii);
				if (CFStringGetTypeID() == CFGetTypeID(tempName)) {
					CFRange range = CFStringFind(tempName, name, kCFCompareCaseInsensitive | kCFCompareAnchored);
					if (range.location == 0) {
						CFArrayAppendValue(nodes, tempName);
					}
				}
			}
			break;

		case eDSStartsWith:
			temp = ODSessionCopyNodeNames(kCFAllocatorDefault, session, NULL);
			count = CFArrayGetCount(temp);
			for (CFIndex ii = 0; ii < count; ii++) {
				CFStringRef tempName = CFArrayGetValueAtIndex(temp, ii);
				if (CFStringGetTypeID() == CFGetTypeID(tempName) && CFStringHasPrefix(tempName, name) == TRUE) {
					CFArrayAppendValue(nodes, tempName);
				}
			}
			break;

		case eDSiContains:
			temp = ODSessionCopyNodeNames(kCFAllocatorDefault, session, NULL);
			count = CFArrayGetCount(temp);
			for (CFIndex ii = 0; ii < count; ii++) {
				CFStringRef tempName = CFArrayGetValueAtIndex(temp, ii);
				if (CFStringGetTypeID() == CFGetTypeID(tempName)) {
					CFRange range = CFStringFind(tempName, name, kCFCompareCaseInsensitive);
					if (range.location != kCFNotFound) {
						CFArrayAppendValue(nodes, tempName);
					}
				}
			}
			break;

		case eDSContains:
			temp = ODSessionCopyNodeNames(kCFAllocatorDefault, session, NULL);
			count = CFArrayGetCount(temp);
			for (CFIndex ii = 0; ii < count; ii++) {
				CFStringRef tempName = CFArrayGetValueAtIndex(temp, ii);
				if (CFStringGetTypeID() == CFGetTypeID(tempName)) {
					CFRange range = CFStringFind(tempName, name, 0);
					if (range.location != kCFNotFound) {
						CFArrayAppendValue(nodes, tempName);
					}
				}
			}
			break;

		case eDSiEndsWith:
		case eDSEndsWith:
			temp = ODSessionCopyNodeNames(kCFAllocatorDefault, session, NULL);
			count = CFArrayGetCount(temp);
			for (CFIndex ii = 0; ii < count; ii++) {
				CFStringRef tempName = CFArrayGetValueAtIndex(temp, ii);
				if (CFStringGetTypeID() == CFGetTypeID(tempName) && CFStringHasSuffix(tempName, name) == TRUE) {
					CFArrayAppendValue(nodes, tempName);
				}
			}
			break;

			// no one would actually use this search and we should cut it off now
		default:
			status = eDSInvalidPatternMatchType;
			break;
	}

	if (status == eDSNoErr) {
		status = _put_propertylist_in_buffer(nodes, outDataBuff);
		if (status == eDSNoErr) {
			(*outDirNodeCount) = CFArrayGetCount(nodes);
		}
	}

	safe_cfrelease_null(temp);
	safe_cfrelease_null(nodes);

	if (ioContinueData != NULL) {
		(*ioContinueData) = 0; // we don't use continue data
	}

	safe_cfrelease_null(array);
	safe_cfrelease_null(name);

	return status;
}

tDirStatus
dsGetDirNodeName(tDirReference inDirRef, tDataBufferPtr inDataBuff, UInt32 inDirNodeIndex, tDataList **outDataList)
{
	ODSessionRef	session	= (ODSessionRef) _get_reference(eDirReference, inDirRef);
	tDirStatus		status	= eDSInvalidBuffFormat;
	CFDataRef		data	= NULL;
	CFArrayRef		nodes	= NULL;

	if (session == NULL) {
		return eDSInvalidReference;
	} else if (inDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (outDataList == NULL) {
		return eDSNullParameter;
	}

	data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (UInt8 *)inDataBuff->fBufferData, inDataBuff->fBufferLength, kCFAllocatorNull);
	nodes = CFPropertyListCreateWithData(kCFAllocatorDefault, data, kCFPropertyListImmutable, NULL, NULL);
	if (nodes != NULL && CFArrayGetTypeID() == CFGetTypeID(nodes)) {
		inDirNodeIndex--;
		if (inDirNodeIndex < (UInt32) CFArrayGetCount(nodes)) {
			CFStringRef	node	= CFArrayGetValueAtIndex(nodes, inDirNodeIndex);
			char		buffer[1024];	// path should never be this big

			CFStringGetCString(node, buffer, sizeof(buffer), kCFStringEncodingUTF8);

			(*outDataList) = dsBuildFromPath(0, buffer, "/");
			status = eDSNoErr;
		} else {
			status = eDSIndexOutOfRange;
		}
	}

	safe_cfrelease_null(data);
	safe_cfrelease_null(nodes);

	return status;
}

tDirStatus
dsOpenDirNode(tDirReference inDirRef, tDataListPtr inDirNodeName, tDirNodeReference *outDirNodeRef)
{
	ODSessionRef	session	= NULL;
	ODNodeRef		node	= NULL;
	tDirStatus		status	= eDSNodeNotFound;
	CFErrorRef		error	= NULL;
	CFStringRef		name	= NULL;
	char 			*nodeStr	= NULL;

	if (inDirNodeName == NULL) {
		return eDSNullNodeName;
	} else if (inDirNodeName->fDataNodeCount == 0) {
		return eDSEmptyNodeName;
	} else if (outDirNodeRef == NULL) {
		return eDSNullParameter;
	}

	nodeStr	= dsGetPathFromList(0, inDirNodeName, "/");
	if (nodeStr != NULL) {
		name = CFStringCreateWithCString(kCFAllocatorDefault, nodeStr, kCFStringEncodingUTF8);
		session = (ODSessionRef) _get_reference(eDirReference, inDirRef);
		if (session == NULL) {
			status = eDSInvalidReference;
			goto fail;
		}

		node = ODNodeCreateWithName(kCFAllocatorDefault, session, name, &error);
		if (node != NULL) {
			(*outDirNodeRef) = _add_reference(node);
			CFRelease(node);
			status = eDSNoErr;
		} else {
			status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		}
	}

fail:
	free(nodeStr);
	safe_cfrelease_null(error);
	safe_cfrelease_null(name);

	return status;
}

tDirStatus
dsCloseDirNode(tDirNodeReference inNodeRef)
{
	return _remove_reference(eNodeReference, inNodeRef);
}

tDirStatus
dsGetDirNodeInfo(tDirNodeReference inNodeRef, tDataListPtr inDirNodeInfoTypeList, tDataBufferPtr outDataBuff, dsBool inAttrInfoOnly,
                 UInt32 *outAttrInfoCount, tAttributeListRef *outAttrListRef, tContextData *ioContinueData)
{
	ODNodeRef		node	= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	CFArrayRef		keys	= NULL;
	CFDictionaryRef details	= NULL;
	CFDictionaryRef	filtered_details = NULL;

	if (node == NULL) {
		return eDSInvalidReference;
	}
	if (outDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (outDataBuff->fBufferSize < 4) {
		return eDSBufferTooSmall;
	} else if (inDirNodeInfoTypeList == NULL) {
		return eDSNullNodeInfoTypeList;
	} else if (inDirNodeInfoTypeList->fDataNodeCount == 0) {
		return eDSEmptyNodeInfoTypeList;
	} else if (outAttrInfoCount == NULL || outAttrListRef == NULL) {
		return eDSNullParameter;
	}

	keys = _convert_datalist_to_cfarray(inDirNodeInfoTypeList, FALSE);
	details = ODNodeCopyDetails(node, keys, NULL);

	if (details != NULL) {
		filtered_details = _copy_filtered_node_details(details, keys);
		CFRelease(details);
	}

	if (filtered_details != NULL) {
		(*outAttrInfoCount) = CFDictionaryGetCount(filtered_details);
		(*outAttrListRef) = _add_reference(filtered_details);
	}

	safe_cfrelease_null(filtered_details);
	safe_cfrelease_null(keys);

	if (ioContinueData != NULL) {
		(*ioContinueData) = 0; // we don't use continue data
	}

	return eDSNoErr;
}

tDirStatus
dsGetRecordList(tDirNodeReference inNodeRef, tDataBufferPtr inOutDataBuff, tDataListPtr inRecNameList, tDirPatternMatch inPatternMatch,
                tDataListPtr inRecTypeList, tDataListPtr inAttribTypeList, dsBool inAttrInfoOnly, UInt32 *inOutRecEntryCount,
                tContextData *ioContinueData)
{
	ODNodeRef			node		= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	tDirStatus			status		= eDSInvalidReference;
	ODQueryRef			query		= NULL;
	CFErrorRef			error		= NULL;
	struct sContinue 	*contdata	= NULL;

	if (node == NULL) {
		return eDSInvalidReference;
	} else if (inOutDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (inRecNameList == NULL) {
		return eDSNullRecNameList;
	} else if (inRecNameList->fDataNodeCount == 0) {
		return eDSEmptyRecordNameList;
	} else if (inRecTypeList == NULL) {
		return eDSNullRecTypeList;
	} else if (inRecTypeList->fDataNodeCount == 0) {
		return eDSEmptyRecordTypeList;
	} else if (inAttribTypeList == NULL) {
		return eDSNullAttributeTypeList;
	} else if (inAttribTypeList->fDataNodeCount == 0) {
		return eDSEmptyAttributeTypeList;
	}

	if (ioContinueData != NULL) {
		contdata = _get_continue_data(*ioContinueData);
		if (contdata != NULL && contdata->type != eQueryContinue) {
			return eDSInvalidContinueData;
		}
	}

	if (contdata == NULL && node != NULL) {
		CFArrayRef	records		= _convert_datalist_to_cfarray(inRecTypeList, FALSE);
		CFArrayRef	values		= _convert_datalist_to_cfarray(inRecNameList, FALSE);
		CFArrayRef	retAttribs	= _convert_datalist_to_cfarray(inAttribTypeList, FALSE);

		query = ODQueryCreateWithNode(kCFAllocatorDefault, node, records, kODAttributeTypeRecordName, inPatternMatch, values, retAttribs,
		                              (*inOutRecEntryCount), &error);

		contdata = _create_continue_data(eQueryContinue);
		contdata->query.query = (ODQueryRef) CFRetain(query);
		contdata->query.attributes = CFRetain(retAttribs);

		if (ioContinueData != NULL) {
			(*ioContinueData) = contdata->continue_id;
		}

		ODQuerySetCallback(query, _query_callback, contdata);
		ODQuerySetDispatchQueue(query, contdata->query.queue);

		safe_cfrelease_null(records);
		safe_cfrelease_null(values);
		safe_cfrelease_null(retAttribs);
		safe_cfrelease_null(query);
	}

	if (contdata != NULL) {
		bool	done = false;

		status = _continue_query(contdata, inOutDataBuff, inOutRecEntryCount, inAttrInfoOnly, &done);
		if (done == true) {
			_remove_continue_data(contdata->continue_id);
			if (ioContinueData != NULL) {
				(*ioContinueData) = 0;
			}
		}
	}

	return status;
}

tDirStatus
dsDoAttributeValueSearchWithData(tDirNodeReference inNodeRef, tDataBufferPtr outDataBuff, tDataListPtr inRecTypeList,
                                 tDataNodePtr inAttrType, tDirPatternMatch inPattMatchType, tDataNodePtr inPatt2Match,
                                 tDataListPtr inAttrTypeRequestList, dsBool inAttrInfoOnly, UInt32 *inOutMatchRecordCount,
                                 tContextData *ioContinueData)
{
	ODNodeRef			node		= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	tDirStatus			status		= eDSInvalidReference;
	ODQueryRef			query		= NULL;
	CFErrorRef			error		= NULL;
	struct sContinue 	*contdata	= NULL;

	if (node == NULL) {
		return eDSInvalidReference;
	} else if (outDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (inRecTypeList == NULL) {
		return eDSNullRecTypeList;
	} else if (inRecTypeList->fDataNodeCount == 0) {
		return eDSEmptyRecordTypeList;
	} else if (inAttrType == NULL) {
		return eDSNullAttributeType;
	} else if (inAttrTypeRequestList == NULL) {
		return eDSNullAttributeRequestList;
	} else if (inAttrTypeRequestList->fDataNodeCount == 0) {
		return eDSEmptyAttributeRequestList;
	} else if (inPatt2Match == NULL) {
		return eDSNullPatternMatch;
	} else if (inOutMatchRecordCount == NULL || ioContinueData == NULL) {
		return eDSNullParameter;
	}

	contdata = _get_continue_data(*ioContinueData);
	if (contdata != NULL && contdata->type != eQueryContinue) {
		return eDSInvalidContinueData;
	}

	if (contdata == NULL && node != NULL) {
		CFArrayRef	records		= _convert_datalist_to_cfarray(inRecTypeList, FALSE);
		CFTypeRef	pattern		= _create_cftype_from_datanode(inPatt2Match);
		CFTypeRef	attribute	= _create_cftype_from_datanode(inAttrType);
		CFTypeRef	retAttribs	= _convert_datalist_to_cfarray(inAttrTypeRequestList, FALSE);

		query = ODQueryCreateWithNode(kCFAllocatorDefault, node, records, attribute, inPattMatchType, pattern, retAttribs,
		                              (*inOutMatchRecordCount), &error);

		contdata = _create_continue_data(eQueryContinue);
		contdata->query.query = (ODQueryRef) CFRetain(query);
		contdata->query.attributes = CFRetain(retAttribs);

		ODQuerySetCallback(query, _query_callback, contdata);
		ODQuerySetDispatchQueue(query, contdata->query.queue);

		(*ioContinueData) = contdata->continue_id;

		safe_cfrelease_null(retAttribs);
		safe_cfrelease_null(attribute);
		safe_cfrelease_null(pattern);
		safe_cfrelease_null(records);
		safe_cfrelease_null(query);
	}

	if (contdata != NULL) {
		bool	done = false;

		status = _continue_query(contdata, outDataBuff, inOutMatchRecordCount, inAttrInfoOnly, &done);
		if (done == true) {
			_remove_continue_data(contdata->continue_id);
			(*ioContinueData) = 0;
		}
	}

	return status;
}

tDirStatus
dsDoAttributeValueSearch(tDirNodeReference inNodeRef, tDataBufferPtr outDataBuff, tDataListPtr inRecTypeList,
                         tDataNodePtr inAttrType, tDirPatternMatch inPattMatchType, tDataNodePtr inPatt2Match, UInt32 *inOutMatchRecordCount,
                         tContextData *ioContinueData)
{
	tDataListPtr	attributes = dsBuildListFromStrings(0, kDSAttributesAll, NULL);
	tDirStatus		status;

	status = dsDoAttributeValueSearchWithData(inNodeRef, outDataBuff, inRecTypeList, inAttrType, inPattMatchType, inPatt2Match, attributes, true,
	                                          inOutMatchRecordCount, ioContinueData);
	dsDataListDeallocate(0, attributes);
	free(attributes);

	return status;
}

tDirStatus
dsDoMultipleAttributeValueSearchWithData(tDirNodeReference inNodeRef, tDataBufferPtr outDataBuff, tDataListPtr inRecTypeList,
                                         tDataNodePtr inAttrType, tDirPatternMatch inPattMatchType, tDataListPtr inPatterns2Match,
                                         tDataListPtr inAttrTypeRequestList, dsBool inAttrInfoOnly, UInt32 *inOutMatchRecordCount,
                                         tContextData *ioContinueData)
{
	ODNodeRef			node		= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	tDirStatus			status		= eDSInvalidReference;
	ODQueryRef			query		= NULL;
	CFErrorRef			error		= NULL;
	struct sContinue 	*contdata	= NULL;

	if (node == NULL) {
		return eDSInvalidReference;
	} else if (outDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (inRecTypeList == NULL) {
		return eDSNullRecTypeList;
	} else if (inRecTypeList->fDataNodeCount == 0) {
		return eDSEmptyRecordTypeList;
	} else if (inAttrType == NULL) {
		return eDSNullAttributeType;
	} else if (inAttrTypeRequestList == NULL) {
		return eDSNullAttributeRequestList;
	} else if (inAttrTypeRequestList->fDataNodeCount == 0) {
		return eDSEmptyAttributeRequestList;
	} else if (inPatterns2Match == NULL) {
		return eDSNullPatternMatch;
	} else if (inPatterns2Match->fDataNodeCount == 0) {
		return eDSEmptyPattern2Match;
	} else if (inOutMatchRecordCount == NULL || ioContinueData == NULL) {
		return eDSNullParameter;
	}

	contdata = _get_continue_data(*ioContinueData);
	if (contdata != NULL && contdata->type != eQueryContinue) {
		return eDSInvalidContinueData;
	}

	if (contdata == NULL && node != NULL) {
		CFArrayRef	records		= _convert_datalist_to_cfarray(inRecTypeList, FALSE);
		CFArrayRef	patterns	= _convert_datalist_to_cfarray(inPatterns2Match, FALSE);
		CFTypeRef	attribute	= _create_cftype_from_datanode(inAttrType);
		CFTypeRef	retAttribs	= _convert_datalist_to_cfarray(inAttrTypeRequestList, FALSE);

		query = ODQueryCreateWithNode(kCFAllocatorDefault, node, records, attribute, inPattMatchType, patterns, retAttribs,
		                              (*inOutMatchRecordCount), &error);

		contdata = _create_continue_data(eQueryContinue);
		contdata->query.query = (ODQueryRef) CFRetain(query);
		contdata->query.attributes = CFRetain(retAttribs);

		ODQuerySetCallback(query, _query_callback, contdata);
		ODQuerySetDispatchQueue(query, contdata->query.queue);

		(*ioContinueData) = contdata->continue_id;

		safe_cfrelease_null(retAttribs);
		safe_cfrelease_null(attribute);
		safe_cfrelease_null(patterns);
		safe_cfrelease_null(records);
		safe_cfrelease_null(query);
	}

	if (contdata != NULL) {
		bool	done = false;

		status = _continue_query(contdata, outDataBuff, inOutMatchRecordCount, inAttrInfoOnly, &done);
		if (done == true) {
			_remove_continue_data(contdata->continue_id);
			(*ioContinueData) = 0;
		}
	}

	return status;
}

tDirStatus
dsDoMultipleAttributeValueSearch(tDirNodeReference inNodeRef, tDataBufferPtr outDataBuff, tDataListPtr inRecTypeList,
                                 tDataNodePtr inAttrType, tDirPatternMatch inPattMatchType, tDataListPtr inPatterns2Match,
                                 UInt32 *inOutMatchRecordCount, tContextData *ioContinueData)
{
	tDataListPtr	attributes = dsBuildListFromStrings(0, kDSNAttrRecordName, kDSNAttrRecordType, kDSNAttrMetaNodeLocation, NULL);
	tDirStatus		status;

	status =  dsDoMultipleAttributeValueSearchWithData(inNodeRef, outDataBuff, inRecTypeList, inAttrType, inPattMatchType, inPatterns2Match, attributes,
	                                                   true, inOutMatchRecordCount, ioContinueData);

	dsDataListDeallocate(0, attributes);
	free(attributes);
	return status;
}

tDirStatus
dsGetRecordEntry(tDirNodeReference inNodeRef, tDataBufferPtr inOutDataBuff, UInt32 inRecordEntryIndex, tAttributeListRef *outAttrListRef,
                 tRecordEntryPtr *outRecEntryPtr)
{
	tDirStatus	status	= eDSIndexOutOfRange;
	ODNodeRef	node	= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);

	if (node == NULL) {
		return eDSInvalidReference;
	} else if (inOutDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (inRecordEntryIndex == 0) {
		return eDSInvalidIndex;
	} else if (outAttrListRef == NULL || outRecEntryPtr == NULL) {
		return eDSNullParameter;
	}

	// we'll always have 1 record in buffer since it doesn't require a dispatch to the daemon
	if (inRecordEntryIndex == 1) {
		CFDictionaryRef	record = _create_propertylist_from_buffer(inOutDataBuff);
		if (record != NULL) {
			CFStringRef		recName		= CFDictionaryGetValue(record, kODAttributeTypeRecordName);
			CFIndex			recNameLen	= (recName ? CFStringGetLength(recName) : 0);
			CFStringRef		recType		= CFDictionaryGetValue(record, kODAttributeTypeRecordType);
			CFIndex			recTypeLen	= (recType ? CFStringGetLength(recType) : 0);
			size_t			length		= (sizeof(UInt16) + CFStringGetMaximumSizeForEncoding(recNameLen, kCFStringEncodingUTF8) +
			                               sizeof(UInt16) + CFStringGetMaximumSizeForEncoding(recTypeLen, kCFStringEncodingUTF8));
			tRecordEntryPtr	entry		= (tRecordEntryPtr) calloc(1, sizeof(tRecordEntry) + length);
			char 			*buffer		= entry->fRecordNameAndType.fBufferData;
			CFDictionaryRef	attribs		= CFDictionaryGetValue(record, CFSTR("attributes"));
			size_t			bufferLen	= 0;

			(*outRecEntryPtr) = entry;
			entry->fRecordAttributeCount = CFDictionaryGetCount(attribs);
			entry->fRecordNameAndType.fBufferSize = length;

			if (recName != NULL) {
				CFStringGetCString(recName, &buffer[sizeof(UInt16)], length, kCFStringEncodingUTF8);
				bufferLen = strlen(&buffer[sizeof(UInt16)]) + 1;
			} else {
				bufferLen = 0;
			}
			*((UInt16 *) buffer) = bufferLen;
			buffer += sizeof(UInt16) + bufferLen;
			entry->fRecordNameAndType.fBufferLength += sizeof(UInt16) + bufferLen;

			if (recType != NULL) {
				CFStringGetCString(recType, &buffer[sizeof(UInt16)], length, kCFStringEncodingUTF8);
				bufferLen = strlen(&buffer[sizeof(UInt16)]) + 1;
			} else {
				bufferLen = 0;
			}
			*((UInt16 *) buffer) = bufferLen;
			entry->fRecordNameAndType.fBufferLength += sizeof(UInt16) + bufferLen;

			(*outAttrListRef) = _add_reference(attribs);

			status = eDSNoErr;
			safe_cfrelease_null(record);
		} else {
			status = eDSInvalidBuffFormat;
		}
	}

	return status;
}

tDirStatus
dsGetAttributeEntry(tDirNodeReference inNodeRef, tDataBufferPtr inOutDataBuff, tAttributeListRef inAttrListRef,
                    UInt32 inAttrInfoIndex, tAttributeValueListRef *outAttrValueListRef, tAttributeEntryPtr *outAttrInfoPtr)
{
	ODNodeRef		node	= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	CFDictionaryRef record	= _get_reference(eAttributeListReference, inAttrListRef);
	tDirStatus		status	= eDSInvalidReference;
	UInt32			count;

	if (node == NULL || record == NULL) {
		return eDSInvalidReference;
	} else if (inOutDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (inAttrInfoIndex == 0) {
		return eDSInvalidIndex;
	} else if (outAttrValueListRef == NULL || outAttrInfoPtr == NULL) {
		return eDSNullParameter;
	}

	count = (UInt32) CFDictionaryGetCount(record);
	inAttrInfoIndex--;
	if (inAttrInfoIndex < count) {
		CFTypeRef 			*keys	= alloca(sizeof(CFTypeRef) * count);
		CFTypeRef 			*values	= alloca(sizeof(CFTypeRef) * count);
		CFIndex				length;
		tAttributeEntryPtr	attrInfoPtr;

		CFDictionaryGetKeysAndValues(record, keys, values);

		length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(keys[inAttrInfoIndex]), kCFStringEncodingUTF8);
		attrInfoPtr = calloc(1, sizeof(tAttributeEntry) + length);

		attrInfoPtr->fAttributeValueCount = CFArrayGetCount(values[inAttrInfoIndex]);
		attrInfoPtr->fAttributeSignature.fBufferSize = length;

		CFStringGetCString(keys[inAttrInfoIndex], attrInfoPtr->fAttributeSignature.fBufferData, length, kCFStringEncodingUTF8);
		attrInfoPtr->fAttributeSignature.fBufferLength = strlen(attrInfoPtr->fAttributeSignature.fBufferData);

		(*outAttrInfoPtr) = attrInfoPtr;
		(*outAttrValueListRef) = _add_reference(values[inAttrInfoIndex]);

		status = eDSNoErr;
	} else {
		status = eDSIndexOutOfRange;
	}

	return status;
}

tDirStatus
dsGetNextAttributeEntry(tDirNodeReference inNodeRef, tDataBufferPtr inOutDataBuff, tAttributeListRef inAttrListRef,
                        UInt32 inAttrInfoIndex, SInt32 *inOutAttributeOffset, tAttributeValueListRef *outAttrValueListRef,
                        tAttributeEntryPtr *outAttrInfoPtr)
{
	(*inOutAttributeOffset) = 0;

	return dsGetAttributeEntry(inNodeRef, inOutDataBuff, inAttrListRef, inAttrInfoIndex, outAttrValueListRef, outAttrInfoPtr);
}

tDirStatus
dsGetAttributeValue(tDirNodeReference inNodeRef, tDataBufferPtr inOutDataBuff, UInt32 inAttrValueIndex,
                    tAttributeValueListRef inAttrValueListRef, tAttributeValueEntryPtr *outAttrValue)
{
	ODNodeRef	node	= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	CFArrayRef	values	= _get_reference(eAttributeValueListReference, inAttrValueListRef);
	tDirStatus	status	= eDSInvalidReference;
	UInt32		count;

	if (node == NULL || values == NULL) {
		return eDSInvalidReference;
	} else if (inOutDataBuff == NULL) {
		return eDSNullDataBuff;
	} else if (inAttrValueIndex == 0) {
		return eDSInvalidIndex;
	} else if (outAttrValue == NULL) {
		return eDSNullParameter;
	}

	count = (UInt32) CFArrayGetCount(values);

	inAttrValueIndex--;
	if (inAttrValueIndex < count) {
		(*outAttrValue) = _create_attribute_value_entry(CFArrayGetValueAtIndex(values, inAttrValueIndex));
		status = eDSNoErr;
	} else {
		status = eDSIndexOutOfRange;
	}

	return status;
}

tDirStatus
dsGetNextAttributeValue(tDirNodeReference inNodeRef, tDataBufferPtr inOutDataBuff, UInt32 inAttrValueIndex,
                        SInt32 *inOutAttributeValueOffset, tAttributeValueListRef inAttrValueListRef, tAttributeValueEntryPtr *outAttrValue)
{
	(*inOutAttributeValueOffset) = 0;

	return dsGetAttributeValue(inNodeRef, inOutDataBuff, inAttrValueIndex, inAttrValueListRef, outAttrValue);
}

tDirStatus
dsCloseAttributeList(tAttributeListRef inAttributeListRef)
{
	return _remove_reference(eAttributeListReference, inAttributeListRef);
}

tDirStatus
dsCloseAttributeValueList(tAttributeValueListRef inAttributeValueListRef)
{
	return _remove_reference(eAttributeValueListReference, inAttributeValueListRef);
}

tDirStatus
dsDoDirNodeAuth(tDirNodeReference inNodeRef, tDataNodePtr inAuthMethod, dsBool inDirNodeAuthOnlyFlag, tDataBufferPtr inAuthStepData,
                tDataBufferPtr outAuthStepDataResponse, tContextData *ioContinueData)
{
	tDataNodePtr	recordType = dsDataNodeAllocateString(0, kDSStdRecordTypeUsers);
	tDirStatus		status;

	status = dsDoDirNodeAuthOnRecordType(inNodeRef, inAuthMethod, inDirNodeAuthOnlyFlag, inAuthStepData, outAuthStepDataResponse,
	                                     ioContinueData, recordType);
	dsDataNodeDeAllocate(inNodeRef, recordType);

	return status;
}

tDirStatus
dsDoDirNodeAuthOnRecordType(tDirNodeReference inNodeRef, tDataNodePtr inAuthMethod, dsBool inDirNodeAuthOnlyFlag,
                            tDataBufferPtr inAuthStepData, tDataBufferPtr outAuthStepDataResponse, tContextData *ioContinueData,
                            tDataNodePtr inRecordType)
{
	ODNodeRef			node		= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	tDirStatus			status		= eDSAuthFailed;
	struct sContinue 	*contdata	= (ioContinueData != NULL ? _get_continue_data(*ioContinueData) : NULL);
	CFErrorRef			error		= NULL;
	CFArrayRef			outStep		= NULL;
	ODContextRef		context		= NULL;
	const char			*authMethod	= inAuthMethod->fBufferData;
	CFArrayRef			inStep		= NULL;
	CFTypeRef			method		= NULL;
	CFTypeRef			recType		= NULL;

	if (node == NULL) {
		return eDSInvalidReference;
	} else if (inAuthMethod == NULL) {
		return eDSNullAutMethod;
	} else if (inAuthStepData == NULL) {
		return eDSNullAuthStepData;
	} else if (outAuthStepDataResponse == NULL) {
		return eDSNullAuthStepDataResp;
	} else if (inRecordType == NULL) {
		return eDSNullRecType;
	}

	if (contdata != NULL) {
		if (contdata->type == eAuthContinue) {
			context = contdata->auth.context;
		} else {
			return eDSInvalidContinueData;
		}
	}

	method = _create_cftype_from_datanode(inAuthMethod);
	recType = _create_cftype_from_datanode(inRecordType);
	inStep = _convert_authbuffer_to_cfarray(method, inAuthStepData);

	// use change password method so that we get better feedback in logging of what is being done
	if (strcmp(authMethod, kDSStdAuthChangePasswd) == 0) {
		CFStringRef		recName		= CFArrayGetValueAtIndex(inStep, 0);
		CFStringRef		password	= CFArrayGetValueAtIndex(inStep, 1);
		CFStringRef		newPassword	= CFArrayGetValueAtIndex(inStep, 2);
		ODRecordRef		record		= ODNodeCopyRecord(node, recType, recName, NULL, &error);

		if (record != NULL) {
			if (ODRecordChangePassword(record, password, newPassword, &error) == TRUE) {
				status = eDSNoErr;
			}

			safe_cfrelease_null(record);
		}
	} else if (strcmp(authMethod, kDSStdAuthSetPasswdAsRoot) == 0) {
		CFStringRef		recName		= CFArrayGetValueAtIndex(inStep, 0);
		CFStringRef		password	= CFArrayGetValueAtIndex(inStep, 1);
		ODRecordRef		record		= ODNodeCopyRecord(node, recType, recName, NULL, &error);

		if (record != NULL) {
			if (ODRecordChangePassword(record, NULL, password, &error) == TRUE) {
				status = eDSNoErr;
			}

			safe_cfrelease_null(record);
		}
	} else {
		if (inDirNodeAuthOnlyFlag == true) {
			if (strcmp(authMethod, kDSStdAuthClearText) == 0 || strcmp(authMethod, kDSStdAuthNodeNativeClearTextOK) == 0
			        || strcmp(authMethod, kDSStdAuthNodeNativeNoClearText) == 0) {
				CFStringRef		recName		= CFArrayGetValueAtIndex(inStep, 0);
				CFStringRef		password	= (CFArrayGetCount(inStep) > 1) ? CFArrayGetValueAtIndex(inStep, 1) : NULL;
				ODRecordRef		record		= ODNodeCopyRecord(node, recType, recName, NULL, &error);

				if (record != NULL) {
					if (ODRecordVerifyPassword(record, password, &error) == TRUE) {
						status = eDSNoErr;
					}

					safe_cfrelease_null(record);
				}
			} else if (ODNodeVerifyCredentialsExtended(node, recType, method, inStep, &outStep, &context, &error) == TRUE) {
				status = eDSNoErr;
			}
		} else {
			if (strcmp(authMethod, kDSStdAuthClearText) == 0 || strcmp(authMethod, kDSStdAuthNodeNativeClearTextOK) == 0
			        || strcmp(authMethod, kDSStdAuthNodeNativeNoClearText) == 0) {
				CFStringRef		recName		= CFArrayGetValueAtIndex(inStep, 0);
				CFStringRef		password	= CFArrayGetValueAtIndex(inStep, 1);

				if (ODNodeSetCredentials(node, recType, recName, password, &error) == TRUE) {
					status = eDSNoErr;
				}
			} else if (ODNodeSetCredentialsExtended(node, recType, method, inStep, &outStep, &context, &error) == TRUE) {
				status = eDSNoErr;
			}
		}
	}

	if (ioContinueData != NULL) {
		if (context != NULL) {
			if ((*ioContinueData) == 0) {
				contdata = _create_continue_data(eAuthContinue);
				contdata->auth.context = (ODContextRef) CFRetain(context);
				(*ioContinueData) = contdata->continue_id;
			} else if (contdata != NULL) {
				// if the contexts are different lets throw it out and change our internal ref
				if (contdata->auth.context != context) {
					CFRelease(contdata->auth.context);
					contdata->auth.context = (ODContextRef) context;
				}
			}
		} else if ((*ioContinueData) != 0) {
			_remove_continue_data(*ioContinueData);
			(*ioContinueData) = 0;
		}
	}

	if (outStep != NULL) {
		status = _pack_array_in_databuffer(outStep, outAuthStepDataResponse);
	}

	if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	}

	safe_cfrelease_null(context);
	safe_cfrelease_null(outStep);
	safe_cfrelease_null(method);
	safe_cfrelease_null(recType);
	safe_cfrelease_null(inStep);

	return status;
}

tDirStatus
dsDoPlugInCustomCall(tDirNodeReference inNodeRef, UInt32 inRequestCode, tDataBuffer *inDataBuff, tDataBuffer *outDataBuff)
{
	tDirStatus	status	= eDSInvalidReference;
	ODNodeRef	node	= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	CFErrorRef	error	= NULL;
	CFDataRef	data	= NULL;

	if (node == NULL) {
		return eDSInvalidReference;
	} else if (inDataBuff == NULL || outDataBuff == NULL) {
		return eDSNullParameter;
	}

	data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (UInt8 *)inDataBuff->fBufferData, inDataBuff->fBufferLength, kCFAllocatorNull);
	if (node != NULL) {
		CFDataRef outData = ODNodeCustomCall(node, inRequestCode, data, &error);
		if (outData != NULL) {
			UInt32 length = (UInt32) CFDataGetLength(outData);

			if (length <= outDataBuff->fBufferSize) {
				CFDataGetBytes(outData, CFRangeMake(0, length), (UInt8 *)outDataBuff->fBufferData);
				outDataBuff->fBufferLength = length;
				status = eDSNoErr;
			} else {
				status = eDSBufferTooSmall;
			}

			safe_cfrelease_null(outData);
		} else {
			status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
			safe_cfrelease_null(error);
		}
	}

	safe_cfrelease_null(data);

	return status;
}

#pragma mark -
#pragma mark Record APIs

tDirStatus
dsOpenRecord(tDirNodeReference inNodeRef, tDataNodePtr inRecType, tDataNodePtr inRecName, tRecordReference *outRecRef)
{
	ODNodeRef	node	= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	tDirStatus	status	= eDSInvalidReference;
	CFErrorRef	error	= NULL;
	CFTypeRef	recType	= NULL;
	CFTypeRef	recName	= NULL;
	CFArrayRef	attribs	= NULL;
	ODRecordRef	record	= NULL;

	if (node == NULL) {
		return eDSInvalidReference;
	} else if (inRecType == NULL) {
		return eDSNullRecType;
	} else if (inRecType->fBufferLength == 0) {
		return eDSEmptyRecordType;
	} else if (inRecName == NULL) {
		return eDSNullRecName;
	} else if (inRecName->fBufferLength == 0) {
		return eDSEmptyRecordName;
	} else if (outRecRef == NULL) {
		return eDSNullParameter;
	}

	recType = _create_cftype_from_datanode(inRecType);
	recName = _create_cftype_from_datanode(inRecName);
	attribs	= CFArrayCreate(kCFAllocatorDefault, (const void **) &kODAttributeTypeAllAttributes, 1, &kCFTypeArrayCallBacks);
	record = ODNodeCopyRecord(node, recType, recName, attribs, &error);
	if (record != NULL) {
		(*outRecRef) = _add_reference(record);
		status = eDSNoErr;
	} else if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	} else {
		status = eDSRecordNotFound;
	}

	safe_cfrelease_null(attribs);
	safe_cfrelease_null(recType);
	safe_cfrelease_null(recName);
	safe_cfrelease_null(record);

	return status;
}

tDirStatus
dsGetRecordReferenceInfo(tRecordReference inRecRef, tRecordEntryPtr *outRecInfo)
{
	ODRecordRef		record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	CFStringRef		recName		= ODRecordGetRecordName(record);
	CFStringRef		recType		= ODRecordGetRecordType(record);
	CFDictionaryRef details		= NULL;
	tRecordEntryPtr	entry		= NULL;
	char 			*buffer		= NULL;
	CFIndex			recNameLen;
	CFIndex			recTypeLen;
	size_t			length;
	size_t			bufferLen;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (outRecInfo == NULL) {
		return eDSNullParameter;
	}

	recNameLen = CFStringGetLength(recName);
	recTypeLen = CFStringGetLength(recType);
	details = ODRecordCopyDetails(record, NULL, NULL);
	length = (sizeof(UInt16) + CFStringGetMaximumSizeForEncoding(recNameLen, kCFStringEncodingUTF8) +
	          sizeof(UInt16) + CFStringGetMaximumSizeForEncoding(recTypeLen, kCFStringEncodingUTF8));
	entry = (tRecordEntryPtr) calloc(1, sizeof(tRecordEntry) + length);
	buffer = entry->fRecordNameAndType.fBufferData;

	(*outRecInfo) = entry;
	entry->fRecordAttributeCount = CFDictionaryGetCount(details);
	entry->fRecordNameAndType.fBufferSize = length;

	CFStringGetCString(recName, &buffer[sizeof(UInt16)], length, kCFStringEncodingUTF8);
	bufferLen = strlen(&buffer[sizeof(UInt16)]) + 1;
	*((UInt16 *) buffer) = bufferLen;
	buffer += sizeof(UInt16) + bufferLen;
	entry->fRecordNameAndType.fBufferLength += sizeof(UInt16) + bufferLen;

	CFStringGetCString(recType, &buffer[sizeof(UInt16)], length, kCFStringEncodingUTF8);
	bufferLen = strlen(&buffer[sizeof(UInt16)]) + 1;
	*((UInt16 *) buffer) = bufferLen;
	entry->fRecordNameAndType.fBufferLength += sizeof(UInt16) + bufferLen;

	safe_cfrelease_null(details);

	return eDSNoErr;
}

tDirStatus
dsGetRecordAttributeInfo(tRecordReference inRecRef, tDataNodePtr inAttributeType, tAttributeEntryPtr *outAttrInfoPtr)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFErrorRef	error		= NULL;
	CFStringRef	attribute	= NULL;
	CFArrayRef	values		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inAttributeType == NULL) {
		return eDSNullAttributeType;
	} else if (inAttributeType->fBufferLength == 0) {
		return eDSEmptyAttributeType;
	} else if (outAttrInfoPtr == NULL) {
		return eDSNullParameter;
	}

	attribute = _create_cftype_from_datanode(inAttributeType);
	values = ODRecordCopyValues(record, attribute, &error);
	if (values != NULL) {
		tAttributeEntryPtr	attribInfo	= (tAttributeEntryPtr) calloc(1, sizeof(tAttributeEntry) + inAttributeType->fBufferSize);
		CFIndex				valueCount	= CFArrayGetCount(values);
		UInt32				uiValueSize = 0;

		for (CFIndex ii = 0; ii < valueCount; ii++) {
			CFTypeRef	cfValue = CFArrayGetValueAtIndex(values, ii);

			if (CFGetTypeID(cfValue) == CFStringGetTypeID()) {
				uiValueSize += CFStringGetLength(cfValue);
			} else {
				uiValueSize += CFDataGetLength(cfValue);
			}
		}

		attribInfo->fAttributeValueCount				= CFArrayGetCount(values);
		attribInfo->fAttributeDataSize					= uiValueSize;
		attribInfo->fAttributeSignature.fBufferSize		= inAttributeType->fBufferSize;
		attribInfo->fAttributeSignature.fBufferLength	= inAttributeType->fBufferLength;

		strlcpy(attribInfo->fAttributeSignature.fBufferData, inAttributeType->fBufferData, attribInfo->fAttributeSignature.fBufferSize);

		(*outAttrInfoPtr) = attribInfo;

		safe_cfrelease_null(values);
		status = eDSNoErr;
	} else if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	} else {
		status = eDSAttributeNotFound;
	}

	safe_cfrelease_null(attribute);

	return status;
}

tDirStatus
dsGetRecordAttributeValueByID(tRecordReference inRecRef, tDataNodePtr inAttributeType, UInt32 inValueID, tAttributeValueEntryPtr *outEntryPtr)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFStringRef	attribute	= NULL;
	CFTypeRef	value		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inAttributeType == NULL) {
		return eDSNullAttributeType;
	} else if (inAttributeType->fBufferLength == 0) {
		return eDSEmptyAttributeType;
	} else if (outEntryPtr == NULL) {
		return eDSNullParameter;
	}

	attribute = _create_cftype_from_datanode(inAttributeType);
	value = _find_value_in_record(record, attribute, inValueID);
	if (value != NULL) {
		(*outEntryPtr) = _create_attribute_value_entry(value);
		status = eDSNoErr;
	} else {
		status = eDSAttributeValueNotFound;
	}

	safe_cfrelease_null(value);
	safe_cfrelease_null(attribute);

	return status;
}

tDirStatus
dsGetRecordAttributeValueByIndex(tRecordReference inRecRef, tDataNodePtr inAttributeType, UInt32 inAttrValueIndex,
                                 tAttributeValueEntryPtr *outEntryPtr)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFErrorRef	error		= NULL;
	CFStringRef	attribute	= NULL;
	CFArrayRef	values		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inAttributeType == NULL) {
		return eDSNullAttributeType;
	} else if (inAttributeType->fBufferLength == 0) {
		return eDSEmptyAttributeType;
	} else if (inAttrValueIndex == 0) {
		return eDSInvalidIndex;
	} else if (outEntryPtr == NULL) {
		return eDSNullParameter;
	}

	attribute = _create_cftype_from_datanode(inAttributeType);
	values = ODRecordCopyValues(record, attribute, &error);
	if (values != NULL) {
		inAttrValueIndex--;
		if (inAttrValueIndex < (UInt32) CFArrayGetCount(values)) {
			(*outEntryPtr) = _create_attribute_value_entry(CFArrayGetValueAtIndex(values, inAttrValueIndex));
			status = eDSNoErr;
		} else {
			status = eDSIndexOutOfRange;
		}
		safe_cfrelease_null(values);
	} else if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	} else {
		status = eDSAttributeValueNotFound;
	}

	safe_cfrelease_null(attribute);

	return status;
}

tDirStatus
dsGetRecordAttributeValueByValue(tRecordReference inRecRef, tDataNodePtr inAttributeType, tDataNodePtr inAttributeValue,
                                 tAttributeValueEntryPtr *outEntryPtr)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFErrorRef	error		= NULL;
	CFStringRef	attribute	= NULL;
	CFStringRef attribValue	= NULL;
	CFArrayRef	values		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inAttributeType == NULL) {
		return eDSNullAttributeType;
	} else if (inAttributeType->fBufferLength == 0) {
		return eDSEmptyAttributeType;
	} else if (inAttributeValue == NULL) {
		return eDSNullAttributeValue;
	} else if (inAttributeValue->fBufferLength == 0) {
		return eDSEmptyAttributeValue;
	} else if (outEntryPtr == NULL) {
		return eDSNullParameter;
	}

	attribute = _create_cftype_from_datanode(inAttributeType);
	attribValue = _create_cftype_from_datanode(inAttributeValue);
	values = ODRecordCopyValues(record, attribute, &error);
	if (values != NULL) {
		if (CFArrayContainsValue(values, CFRangeMake(0, CFArrayGetCount(values)), attribValue) == TRUE) {
			(*outEntryPtr) = _create_attribute_value_entry(attribValue);
			status = eDSNoErr;
		} else {
			status = eDSAttributeValueNotFound;
		}

		safe_cfrelease_null(values);
	} else if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	} else {
		status = eDSAttributeValueNotFound;
	}

	safe_cfrelease_null(attribute);
	safe_cfrelease_null(attribValue);

	return status;
}

tDirStatus
dsFlushRecord(tRecordReference inRecRef)
{
	ODRecordRef	record	= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	if (record == NULL) {
		return eDSInvalidReference;
	}

	return eDSNoErr;
}

tDirStatus
dsCloseRecord(tRecordReference inRecRef)
{
	return _remove_reference(eRecordReference, inRecRef);
}

tDirStatus
dsSetRecordName(tRecordReference inRecRef, tDataNodePtr inNewRecordName)
{
	ODRecordRef			record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus			status		= eDSInvalidReference;
	CFErrorRef			error		= NULL;
	CFStringRef			newName		= NULL;
	CFArrayRef			values		= NULL;
	CFMutableArrayRef	newValues	= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inNewRecordName == NULL) {
		return eDSNullRecName;
	}

	newName	= _create_cftype_from_datanode(inNewRecordName);
	values = ODRecordCopyValues(record, kODAttributeTypeRecordName, NULL);
	if (values != NULL) {
		newValues = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, values);
		CFArraySetValueAtIndex(newValues, 0, newName);
	} else {
		newValues = (CFMutableArrayRef) CFArrayCreate(kCFAllocatorDefault, (const void **) &newName, 1, &kCFTypeArrayCallBacks);
	}

	if (ODRecordSetValue(record, kODAttributeTypeRecordName, newValues, &error) == TRUE) {
		status = eDSNoErr;
	} else if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	} else {
		status = eDSPermissionError;
	}

	safe_cfrelease_null(values);
	safe_cfrelease_null(newValues);
	safe_cfrelease_null(newName);

	return status;
}

tDirStatus
dsSetRecordType(tRecordReference inRecRef, tDataNodePtr inNewRecordType)
{
	return eNoLongerSupported;
}

tDirStatus
dsDeleteRecord(tRecordReference inRecRef)
{
	ODRecordRef	record	= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status	= eDSInvalidReference;
	CFErrorRef	error	= NULL;

	if (record != NULL) {
		if (ODRecordDelete(record, &error) == TRUE) {
			_remove_reference(eRecordReference, inRecRef);
			status = eDSNoErr;
		} else {
			status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
			safe_cfrelease_null(error);
		}
	}

	return status;
}

tDirStatus
dsCreateRecord(tDirNodeReference inNodeRef, tDataNodePtr inRecType, tDataNodePtr inRecName)
{
	return dsCreateRecordAndOpen(inNodeRef, inRecType, inRecName, NULL);
}

tDirStatus
dsCreateRecordAndOpen(tDirNodeReference inNodeRef, tDataNodePtr inRecType, tDataNodePtr inRecName, tRecordReference *outRecRef)
{
	ODNodeRef	node	= (ODNodeRef) _get_reference(eNodeReference, inNodeRef);
	tDirStatus	status	= eDSInvalidReference;
	CFErrorRef	error	= NULL;
	CFStringRef	recType	= NULL;
	CFStringRef	recName	= NULL;
	ODRecordRef	record	= NULL;

	if (node == NULL) {
		return eDSInvalidReference;
	} else if (inRecType == NULL) {
		return eDSNullRecType;
	} else if (inRecName == NULL) {
		return eDSNullRecName;
	}

	recType	= _create_cftype_from_datanode(inRecType);
	recName	= _create_cftype_from_datanode(inRecName);
	record = ODNodeCreateRecord(node, recType, recName, NULL, &error);
	if (record != NULL) {
		if (outRecRef != NULL) {
			(*outRecRef) = _add_reference(record);
		}
		safe_cfrelease_null(record);

		status = eDSNoErr;
	} else {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	}

	safe_cfrelease_null(recType);
	safe_cfrelease_null(recName);

	return status;
}

tDirStatus
dsAddAttribute(tRecordReference inRecRef, tDataNodePtr inNewAttr, tAccessControlEntryPtr inNewAttrAccess, tDataNodePtr inFirstAttrValue)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFErrorRef	error		= NULL;
	CFStringRef	attribute	= NULL;
	CFStringRef value		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inNewAttr == NULL) {
		return eDSNullAttributeType;
	} else if (inFirstAttrValue == NULL) {
		// we don't really do anything if there is no value
		return eDSNoErr;
	}

	attribute = _create_cftype_from_datanode(inNewAttr);
	value = _create_cftype_from_datanode(inFirstAttrValue);
	if (ODRecordSetValue(record, attribute, value, &error) == TRUE) {
		status = eDSNoErr;
	} else if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	} else {
		status = eDSPermissionError;
	}

	safe_cfrelease_null(attribute);
	safe_cfrelease_null(value);

	return status;
}

tDirStatus
dsRemoveAttribute(tRecordReference inRecRef, tDataNodePtr inAttribute)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFErrorRef	error		= NULL;
	CFStringRef	attribute	= NULL;
	CFArrayRef	empty		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inAttribute == NULL) {
		return eDSNullAttributeType;
	}

	attribute = _create_cftype_from_datanode(inAttribute);
	empty = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	if (ODRecordSetValue(record, attribute, empty, &error) == TRUE) {
		status = eDSNoErr;
	} else if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	} else {
		status = eDSPermissionError;
	}

	safe_cfrelease_null(empty);
	safe_cfrelease_null(attribute);

	return status;
}

tDirStatus
dsAddAttributeValue(tRecordReference inRecRef, tDataNodePtr inAttrType, tDataNodePtr inAttrValue)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFStringRef	attribute	= NULL;
	CFStringRef	value		= NULL;
	CFErrorRef	error		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inAttrType == NULL) {
		return eDSNullAttributeType;
	} else if (inAttrValue == NULL) {
		return eDSNullAttributeValue;
	}

	attribute = _create_cftype_from_datanode(inAttrType);
	value = _create_cftype_from_datanode(inAttrValue);
	if (ODRecordAddValue(record, attribute, value, &error) == TRUE) {
		status = eDSNoErr;
	} else {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	}

	safe_cfrelease_null(attribute);
	safe_cfrelease_null(value);

	return status;
}

tDirStatus
dsRemoveAttributeValue(tRecordReference inRecRef, tDataNodePtr inAttrType, UInt32 inAttrValueID)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFStringRef	attribute	= NULL;
	CFStringRef	value		= NULL;
	CFErrorRef	error		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inAttrType == NULL) {
		return eDSNullAttributeType;
	}

	attribute = _create_cftype_from_datanode(inAttrType);
	value = _find_value_in_record(record, attribute, inAttrValueID);

	if (value != NULL) {
		if (ODRecordRemoveValue(record, attribute, value, &error) == TRUE) {
			status = eDSNoErr;
		} else if (error != NULL) {
			status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
			safe_cfrelease_null(error);
		} else {
			status = eDSAttributeDoesNotExist;
		}
	}

	safe_cfrelease_null(value);
	safe_cfrelease_null(attribute);

	return status;
}

tDirStatus
dsSetAttributeValue(tRecordReference inRecRef, tDataNodePtr inAttrType, tAttributeValueEntryPtr inAttrValueEntry)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFArrayRef	values		= NULL;
	CFStringRef	attribute	= NULL;
	CFStringRef	newValue	= NULL;
	CFErrorRef	error		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inAttrType == NULL) {
		return eDSNullAttributeType;
	} else if (inAttrValueEntry == NULL) {
		return eDSNullParameter;
	}

	attribute = _create_cftype_from_datanode(inAttrType);
	newValue = _create_cftype_from_datanode(&inAttrValueEntry->fAttributeValueData);
	values = ODRecordCopyValues(record, attribute, NULL);
	if (values != NULL) {
		CFIndex	count	= CFArrayGetCount(values);
		CFIndex ii;

		for (ii = 0; ii < count; ii++) {
			CFTypeRef tempValue = CFArrayGetValueAtIndex(values, ii);
			if (_calculate_CRC32(tempValue) == inAttrValueEntry->fAttributeValueID) {
				break;
			}
		}

		if (ii < count) {
			CFMutableArrayRef newValues = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, values);

			CFArraySetValueAtIndex(newValues, ii, newValue);
			if (ODRecordSetValue(record, attribute, newValues, &error) == TRUE) {
				status = eDSNoErr;
			} else if (error != NULL) {
				status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
				safe_cfrelease_null(error);
			} else {
				status = eDSPermissionError;
			}

			safe_cfrelease_null(newValues);
		} else {
			status = eDSAttributeValueNotFound;
		}

		safe_cfrelease_null(values);
	} else {
		status = eDSAttributeNotFound;
	}

	safe_cfrelease_null(attribute);
	safe_cfrelease_null(newValue);

	return status;
}

tDirStatus
dsSetAttributeValues(tRecordReference inRecRef, tDataNodePtr inAttrType, tDataListPtr inAttributeValuesPtr)
{
	ODRecordRef	record		= (ODRecordRef) _get_reference(eRecordReference, inRecRef);
	tDirStatus	status		= eDSInvalidReference;
	CFStringRef	attribute	= NULL;
	CFArrayRef	values		= NULL;
	CFErrorRef	error		= NULL;

	if (record == NULL) {
		return eDSInvalidReference;
	} else if (inAttrType == NULL) {
		return eDSNullAttributeType;
	} else if (inAttributeValuesPtr == NULL) {
		return eDSNullParameter;
	}

	attribute = _create_cftype_from_datanode(inAttrType);
	values = _convert_datalist_to_cfarray(inAttributeValuesPtr, FALSE);

	if (ODRecordSetValue(record, attribute, values, &error) == TRUE) {
		status = eDSNoErr;
	} else if (error != NULL) {
		status = ODConvertToLegacyErrorCode(CFErrorGetCode(error));
		safe_cfrelease_null(error);
	} else {
		status = eDSPermissionError;
	}

	safe_cfrelease_null(values);
	safe_cfrelease_null(attribute);

	return status;
}
