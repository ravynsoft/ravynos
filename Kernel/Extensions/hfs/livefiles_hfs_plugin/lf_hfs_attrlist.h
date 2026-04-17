/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_attrlist.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 25/3/18.
 */

#ifndef lf_hfs_attrlist_h
#define lf_hfs_attrlist_h

#include <sys/attr.h>

#include "lf_hfs_catalog.h"
#include "lf_hfs_vnode.h"
/*
 * The following define the attributes that HFS supports:
 */

typedef struct
{
    scandir_matching_request_t* psMatchingCriteria;
    scandir_matching_reply_t*   psMatchingResult;
} ScanDirRequest_s;

#define HFS_ATTR_CMN_VALID (ATTR_CMN_NAME | ATTR_CMN_DEVID    |        \
                            ATTR_CMN_FSID | ATTR_CMN_OBJTYPE |        \
                            ATTR_CMN_OBJTAG | ATTR_CMN_OBJID |        \
                            ATTR_CMN_OBJPERMANENTID | ATTR_CMN_PAROBJID |    \
                            ATTR_CMN_SCRIPT | ATTR_CMN_CRTIME |        \
                            ATTR_CMN_MODTIME | ATTR_CMN_CHGTIME |        \
                            ATTR_CMN_ACCTIME | ATTR_CMN_BKUPTIME |        \
                            ATTR_CMN_FNDRINFO |ATTR_CMN_OWNERID |        \
                            ATTR_CMN_GRPID | ATTR_CMN_ACCESSMASK |        \
                            ATTR_CMN_FLAGS | ATTR_CMN_USERACCESS |        \
                            ATTR_CMN_FILEID | ATTR_CMN_PARENTID )

#define HFS_ATTR_CMN_SEARCH_VALID  (ATTR_CMN_NAME | ATTR_CMN_OBJID |    \
                                    ATTR_CMN_PAROBJID | ATTR_CMN_CRTIME |    \
                                    ATTR_CMN_MODTIME | ATTR_CMN_CHGTIME |     \
                                    ATTR_CMN_ACCTIME | ATTR_CMN_BKUPTIME |    \
                                    ATTR_CMN_FNDRINFO | ATTR_CMN_OWNERID |    \
                                    ATTR_CMN_GRPID    | ATTR_CMN_ACCESSMASK | \
                                    ATTR_CMN_FILEID | ATTR_CMN_PARENTID )

#define HFS_ATTR_DIR_VALID (ATTR_DIR_LINKCOUNT | ATTR_DIR_ENTRYCOUNT | ATTR_DIR_MOUNTSTATUS)

#define HFS_ATTR_DIR_SEARCH_VALID (ATTR_DIR_ENTRYCOUNT)

#define HFS_ATTR_FILE_VALID (ATTR_FILE_LINKCOUNT |ATTR_FILE_TOTALSIZE |      \
                             ATTR_FILE_ALLOCSIZE | ATTR_FILE_IOBLOCKSIZE |      \
                             ATTR_FILE_CLUMPSIZE | ATTR_FILE_DEVTYPE |      \
                             ATTR_FILE_DATALENGTH | ATTR_FILE_DATAALLOCSIZE | \
                             ATTR_FILE_RSRCLENGTH | ATTR_FILE_RSRCALLOCSIZE)

#define HFS_ATTR_FILE_SEARCH_VALID (ATTR_FILE_DATALENGTH | ATTR_FILE_DATAALLOCSIZE |    \
                                    ATTR_FILE_RSRCLENGTH | ATTR_FILE_RSRCALLOCSIZE )

int hfs_readdirattr_internal(struct vnode *dvp, ReadDirBuff_s* psReadDirBuffer, int maxcount, uint32_t *newstate, int *eofflag, int *actualcount, uint64_t uCookie);
int hfs_scandir(struct vnode *dvp, ScanDirRequest_s* psScanDirRequest);
#endif /* lf_hfs_attrlist_h */
