/*
 * Copyright (c) 1999-2007 Apple Inc.  All Rights Reserved.
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

#if __OBJC2__

#include "objc-private.h"
#include "objc-file.h"


// Look for a __DATA or __DATA_CONST or __DATA_DIRTY section 
// with the given name that stores an array of T.
template <typename T>
T* getDataSection(const headerType *mhdr, const char *sectname, 
                  size_t *outBytes, size_t *outCount)
{
    unsigned long byteCount = 0;
    T* data = (T*)getsectiondata(mhdr, "__DATA", sectname, &byteCount);
    if (!data) {
        data = (T*)getsectiondata(mhdr, "__DATA_CONST", sectname, &byteCount);
    }
    if (!data) {
        data = (T*)getsectiondata(mhdr, "__DATA_DIRTY", sectname, &byteCount);
    }
    if (outBytes) *outBytes = byteCount;
    if (outCount) *outCount = byteCount / sizeof(T);
    return data;
}

#define GETSECT(name, type, sectname)                                   \
    type *name(const headerType *mhdr, size_t *outCount) {              \
        return getDataSection<type>(mhdr, sectname, nil, outCount);     \
    }                                                                   \
    type *name(const header_info *hi, size_t *outCount) {               \
        return getDataSection<type>(hi->mhdr(), sectname, nil, outCount); \
    }

//      function name                 content type     section name
GETSECT(_getObjc2SelectorRefs,        SEL,             "__objc_selrefs"); 
GETSECT(_getObjc2MessageRefs,         message_ref_t,   "__objc_msgrefs"); 
GETSECT(_getObjc2ClassRefs,           Class,           "__objc_classrefs");
GETSECT(_getObjc2SuperRefs,           Class,           "__objc_superrefs");
GETSECT(_getObjc2ClassList,           classref_t const,      "__objc_classlist");
GETSECT(_getObjc2NonlazyClassList,    classref_t const,      "__objc_nlclslist");
GETSECT(_getObjc2CategoryList,        category_t * const,    "__objc_catlist");
GETSECT(_getObjc2CategoryList2,       category_t * const,    "__objc_catlist2");
GETSECT(_getObjc2NonlazyCategoryList, category_t * const,    "__objc_nlcatlist");
GETSECT(_getObjc2ProtocolList,        protocol_t * const,    "__objc_protolist");
GETSECT(_getObjc2ProtocolRefs,        protocol_t *,    "__objc_protorefs");
GETSECT(getLibobjcInitializers,       UnsignedInitializer, "__objc_init_func");


objc_image_info *
_getObjcImageInfo(const headerType *mhdr, size_t *outBytes)
{
    return getDataSection<objc_image_info>(mhdr, "__objc_imageinfo", 
                                           outBytes, nil);
}

// Look for an __objc* section other than __objc_imageinfo
static bool segmentHasObjcContents(const segmentType *seg)
{
    for (uint32_t i = 0; i < seg->nsects; i++) {
        const sectionType *sect = ((const sectionType *)(seg+1))+i;
        if (sectnameStartsWith(sect->sectname, "__objc_")  &&
            !sectnameEquals(sect->sectname, "__objc_imageinfo"))
        {
            return true;
        }
    }

    return false;
}

// Look for an __objc* section other than __objc_imageinfo
bool
_hasObjcContents(const header_info *hi)
{
    bool foundObjC = false;

    foreach_data_segment(hi->mhdr(), [&](const segmentType *seg, intptr_t slide)
    {
        if (segmentHasObjcContents(seg)) foundObjC = true;
    });

    return foundObjC;
    
}


// OBJC2
#endif
