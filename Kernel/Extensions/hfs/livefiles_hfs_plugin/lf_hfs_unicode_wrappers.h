//
//  lf_hfs_unicode_wrappers.h
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#ifndef lf_hfs_unicode_wrappers_h
#define lf_hfs_unicode_wrappers_h

#include <stdio.h>
#include "lf_hfs_defs.h"
#include "lf_hfs_file_mgr_internal.h"

int32_t FastUnicodeCompare      ( register ConstUniCharArrayPtr str1, register ItemCount len1, register ConstUniCharArrayPtr str2, register ItemCount len2);

int32_t UnicodeBinaryCompare    ( register ConstUniCharArrayPtr str1, register ItemCount len1, register ConstUniCharArrayPtr str2, register ItemCount len2 );

HFSCatalogNodeID GetEmbeddedFileID( ConstStr31Param filename, u_int32_t length, u_int32_t *prefixLength );

OSErr ConvertUnicodeToUTF8Mangled(ByteCount srcLen, ConstUniCharArrayPtr srcStr, ByteCount maxDstLen,
                                  ByteCount *actualDstLen, unsigned char* dstStr, HFSCatalogNodeID cnid);

u_int32_t
CountFilenameExtensionChars( const unsigned char * filename, u_int32_t length );


#endif /* lf_hfs_unicode_wrappers_h */
