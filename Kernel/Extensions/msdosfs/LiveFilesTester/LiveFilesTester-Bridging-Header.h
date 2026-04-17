//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//
#import <CommonCrypto/CommonCrypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <TargetConditionals.h>
#include <sys/ioctl.h>
#include <sys/disk.h>

#if TARGET_OS_IPHONE
#include <UserFS/UserVFS.h>
#else
#include "UserVFS.h"
#include "UserVFS_types.h"
#endif
#include <errno.h>

#define SUCCESS 0

// The next definitions are taken from the hfs code, if defined HFS_CRASH_TEST:
typedef enum {
    CRASH_ABORT_NONE,
    CRASH_ABORT_MAKE_DIR,
    CRASH_ABORT_JOURNAL_BEFORE_FINISH,        // Crash driver before journal update starts
    CRASH_ABORT_JOURNAL_AFTER_JOURNAL_DATA,   // Crash driver after the journal data has been written but before the journal header has been updated
    CRASH_ABORT_JOURNAL_AFTER_JOURNAL_HEADER, // Crash driver after the journal header has been updated but before blocks were written to destination
    CRASH_ABORT_JOURNAL_IN_BLOCK_DATA,        // Crash driver while writing data blocks
    CRASH_ABORT_JOURNAL_AFTER_BLOCK_DATA,     // Crash the driver after the data blocks were written
    CRASH_ABORT_ON_UNMOUNT,                   // Crash on unmount
    CRASH_ABORT_LAST
} CrashAbort_E;


double elapsed_time;

/******************************************************************************
 (De)Init functions
 ******************************************************************************/
int brdg_openFileDescriptor(const char* devName, int* fd);
int brdg_closeFileDescriptor(int* fd);
int brdg_loadDylib(UVFSFSOps* testerFsOps, const char* dylibPath, const char* fsOps_Symbol);
int brdg_unloadDylib(const char* dylibPath);
int brdg_ioctl(int fildes, unsigned long request, int *val);
int brdg_get_physical_block_size(int fildes, int *val);

/******************************************************************************
 Wrapping functions
 ******************************************************************************/

int brdg_fsops_init(UVFSFSOps* testerFsOps);
void brdg_fsops_fini(UVFSFSOps* testerFsOps);
int brdg_fsops_taste(UVFSFSOps* testerFsOps, int* devFd);
int brdg_fsops_scanvols(UVFSFSOps* testerFsOps, int* diskFd, UVFSScanVolsRequest *request, UVFSScanVolsReply *reply);
int brdg_fsops_mount(UVFSFSOps* testerFsOps, int* diskFd, UVFSFileNode *outRootFileNode);
int brdg_fsops_unmount(UVFSFSOps* testerFsOps, UVFSFileNode* rootFileNode, UVFSUnmountHint hint);
int brdg_fsops_getattr(UVFSFSOps* testerFsOps, UVFSFileNode* pfileNode, UVFSFileAttributes *attrs);
int brdg_fsops_setattr(UVFSFSOps* testerFsOps, UVFSFileNode *Node, const UVFSFileAttributes *attrs, UVFSFileAttributes *outAttrs);
int brdg_fsops_lookup(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, UVFSFileNode* outNode);
int brdg_fsops_reclaim(UVFSFSOps* testerFsOps, UVFSFileNode* Node);
int brdg_fsops_readlink(UVFSFSOps* testerFsOps, UVFSFileNode *Node, void *outBuf, size_t bufsize, size_t *actuallyRead, UVFSFileAttributes *outAttrs);
int brdg_fsops_write(UVFSFSOps* testerFsOps, UVFSFileNode* Node, uint64_t offset, size_t length, const void *buf, size_t *actuallyWritten);
int brdg_fsops_read(UVFSFSOps* testerFsOps, UVFSFileNode *Node, uint64_t offset, size_t length, void *outBuf, size_t *actuallyRead);
int brdg_fsops_create(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);
int brdg_fsops_mkdir(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);
int brdg_fsops_symlink(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, const char *contents, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);
int brdg_fsops_hardlink(UVFSFSOps* testerFsOps, UVFSFileNode* fromNode, UVFSFileNode* dirNode, const char *name, UVFSFileAttributes* outFileAttrs, UVFSFileAttributes* outDirAttrs);
int brdg_fsops_remove(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name);
int brdg_fsops_rmdir(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name);
int brdg_fsops_rename(UVFSFSOps* testerFsOps, UVFSFileNode* fromDirNode, UVFSFileNode fromNode, const char *fromName, UVFSFileNode* toDirNode, UVFSFileNode toNode, const char *toName, uint32_t flags);
int brdg_fsops_sync(UVFSFSOps* testerFsOps, UVFSFileNode* node);
int brdg_fsops_readdir(UVFSFSOps* testerFsOps, UVFSFileNode *dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytes_read,  uint64_t *verifier);
int brdg_fsops_readdirattr(UVFSFSOps* testerFsOps, UVFSFileNode *dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytes_read,  uint64_t *verifier);
int brdg_fsops_getfsattr(UVFSFSOps* testerFsOps, UVFSFileNode *Node, const char *attr, UVFSFSAttributeValue *val, size_t len, size_t *retlen);
int brdg_fsops_setfsattr(UVFSFSOps* testerFsOps, UVFSFileNode *Node, const char *attr, const UVFSFSAttributeValue *val, size_t len, UVFSFSAttributeValue *out_val, size_t out_len);
uint64_t brdg_fsops_version(UVFSFSOps* testerFsOps);
int brdg_fsops_check(UVFSFSOps* testerFsOps,int disk_fd, check_flags_t how);

//int brdg_fsops_statfs(UVFSFileNode dirNode, statfsResult *result);
//int brdg_fsops_pathconf(UVFSFileNode dirNode, pathconfResult *result);


/******************************************************************************
 Useful functions
 ******************************************************************************/
int 	brdg_createNewFile	(const char* fileName, UVFSFileNode* rootFileHandle);
void 	print_attrs			(UVFSFileAttributes *attrs);
void 	brdg_srand			(int seed);
int 	brdg_rand			(void);
char*	brdg_strerror		(int errnum);
char*	get_de_name			(void *streamBuf, size_t offset);
size_t 	get_direntry_reclen	(uint32_t namelen);
char*	get_fs_string		(UVFSFSAttributeValue *fsattr);
uint8_t *get_fs_opaque		(UVFSFSAttributeValue *fsattr);
char*   getBuildTime        (void);
char*   getBuildDate        (void);
int brdg_inject_error       (CrashAbort_E stepToFall);
int brdg_reject_error       (CrashAbort_E stepToFall);
size_t  get_dea_reclen(UVFSDirEntryAttr dea , uint32_t namelen);
char*   get_dea_name(void *streamBuf, size_t offset);
int     dirtyBlockTest_Create(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode);
int     dirtyBlockTest_Sync(UVFSFSOps* testerFsOps, UVFSFileNode* Node);
uint32_t  brdg_convertModeToUserBits (uint32_t);
double timespec_diff_in_ns(struct timespec *start, struct timespec *stop);



