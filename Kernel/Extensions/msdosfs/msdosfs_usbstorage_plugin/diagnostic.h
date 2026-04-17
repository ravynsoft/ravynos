/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  diagnostic.h
 *  livefiles_msdos
 *
 *  Created by Or Haimovich on 31/1/18.
 *
 */
#ifndef diagnostic_h
#define diagnostic_h

#ifdef DIAGNOSTIC

#include "Common.h"

#define DIAGNOSTIC_INIT(A)   (DIAGNOSTIC_CacheInit(A))
#define DIAGNOSTIC_DEINIT(A) (DIAGNOSTIC_CacheDeInit(A))
#define DIAGNOSTIC_INSERT(A,B,C) (DIAGNOSTIC_InsertNewRecord(A,B,C))
#define DIAGNOSTIC_REMOVE(A) (DIAGNOSTIC_RemoveRecord(GET_FSRECORD(A),A->sRecordData.uParentCluster,&(A->sRecordData.sName)))

void DIAGNOSTIC_CacheInit(FileSystemRecord_s* psFSRecord);
int DIAGNOSTIC_CacheDeInit(FileSystemRecord_s* psFSRecord);
int DIAGNOSTIC_InsertNewRecord(NodeRecord_s* psFileRecord, uint64_t uParentCluster, const char * pcName);
int DIAGNOSTIC_RemoveRecord(FileSystemRecord_s* psFSRecord, uint64_t uParentCluster, struct unistr255* psName);

#else

#define DIAGNOSTIC_INIT(A)
#define DIAGNOSTIC_DEINIT(A) (0)
#define DIAGNOSTIC_INSERT(A,B,C) (0)
#define DIAGNOSTIC_REMOVE(A) (0)

#endif //DIAGNOSTIC


#endif /* diagnostic_h */
