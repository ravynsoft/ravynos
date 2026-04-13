//
//  msdosfs_tester_wraper.c
//  msdosfs_tester
//
//  Created by Liran Ritkop on 15/10/2017.
//

#include "LiveFilesTester-Bridging-Header.h"
#include <unistd.h>
#include <fcntl.h>
#import  <os/log.h>
#include <stdio.h>
#include <dispatch/dispatch.h>
#include <dlfcn.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/disk.h>


#define logf(...) \
    ({ \
        printf("[FATTester] %s (%d): ", __func__, __LINE__);   \
        printf(__VA_ARGS__);    \
    })

#define NOF_DYLIBS_SUPPORTED    3
#define MAX_DYLIB_PATH_LENGTH   80

// The next definitions are taken from the hfs code, if defined HFS_CRASH_TEST:
typedef int (*CrashAbortFunction_FP)(CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, pthread_t pSyncerThread);
int HFSTest_CrashAbort(CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, pthread_t pSyncerThread);

char dylibs_paths[NOF_DYLIBS_SUPPORTED][MAX_DYLIB_PATH_LENGTH];
void *plugin_handle[NOF_DYLIBS_SUPPORTED];
int dylibs_loaded_counter = 0;

struct timespec tm1;
struct timespec tm2;
/******************************************************************************
                             (De)Init functions
******************************************************************************/

// The function load dynamic library with the required filesysem handler for using
// the fsops. Currently its hard-coded for msdos.
int brdg_loadDylib(UVFSFSOps* testerFsOps, const char* dylibPath, const char* fsOps_Symbol) {
    logf("Loading %s\n", dylibPath);
    // Load plugin; get its function pointers
    if (dylibs_loaded_counter == NOF_DYLIBS_SUPPORTED ) {
        logf("Error, request loading for too many dylib. Check that there isn't dylib that loaded twice");
        return E2BIG;
    }
    
    plugin_handle[dylibs_loaded_counter] = dlopen(dylibPath, RTLD_NOW|RTLD_LOCAL|RTLD_FIRST);
    strcpy(dylibs_paths[dylibs_loaded_counter], dylibPath);
    
    if (plugin_handle[dylibs_loaded_counter] == NULL) {
        logf("Error on dlopen: %s\n", dlerror());
        return EINVAL;
    }
    
    
    void* plugin_init = dlsym(plugin_handle[dylibs_loaded_counter], fsOps_Symbol);
    
    if (plugin_init == NULL)
        assert(0);
    
    *testerFsOps = *(UVFSFSOps*)plugin_init;
    dylibs_loaded_counter++;
    return SUCCESS;
}



// The function unload the dylib that had been loaded on init.
int brdg_unloadDylib(const char* dylibPath) {
    int i;
    logf("Unloading live files dylib\n");
    for (i=0; i<NOF_DYLIBS_SUPPORTED; i++) {
        if (strcmp(dylibs_paths[i], dylibPath) == 0) {
            dlclose(plugin_handle[i]);
            dylibs_loaded_counter--;
            return SUCCESS;
        }
    }
    logf("dylib requested to unload is not familiar");
    return EINVAL;
}


// Thvoide function receive a device name (as string) and fileHandle.
// The function assigns the fileHandle a valid value.
// Return SUCCESS or any error.
int brdg_openFileDescriptor(const char* devName, int* fd){
    
    *fd = open(devName,O_RDWR);
    if (*fd < 0)
        return EBADF;
    return SUCCESS;
}

// The function deallocate all necessary resources for the filehandle.
// Return SUCCESS or any error.
int brdg_closeFileDescriptor(int* fd){

    if ( close(*fd) != 0 ) {
        return EBADF;
    }
    return SUCCESS;
    
}

int brdg_ioctl(int fildes, unsigned long request, int *val) {
    return ioctl(fildes, request, val);
}


int brdg_get_physical_block_size(int fildes, int *val){
    return ioctl(fildes, DKIOCGETPHYSICALBLOCKSIZE , val);
}

/******************************************************************************
                             Wrapping functions
 ******************************************************************************/

int brdg_fsops_setfsattr(UVFSFSOps* testerFsOps, UVFSFileNode *Node, const char *attr, const UVFSFSAttributeValue *val, size_t len, UVFSFSAttributeValue *out_val, size_t out_len)
{
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_setfsattr(*Node, attr, val, len, out_val, out_len);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_setfsattr return (%d) - %s\n", errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}

int brdg_fsops_getfsattr(UVFSFSOps* testerFsOps, UVFSFileNode *Node, const char *attr, UVFSFSAttributeValue *val, size_t len, size_t *retlen)
{
    
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_getfsattr(*Node, attr, val, len, retlen);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_getfsattr return (%d) - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
    
}

int brdg_fsops_init(UVFSFSOps* testerFsOps){
    return (*testerFsOps).fsops_init();
}

void brdg_fsops_fini(UVFSFSOps* testerFsOps){
    
    (*testerFsOps).fsops_fini();
}

int brdg_fsops_taste(UVFSFSOps* testerFsOps, int* devFd){
    
    int errnum;
    errnum = (*testerFsOps).fsops_taste(*devFd);
    logf("fsops_taste return (%d) - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}

int brdg_fsops_scanvols(UVFSFSOps* testerFsOps, int* diskFd, UVFSScanVolsRequest *request, UVFSScanVolsReply *reply){
    int errnum;
    errnum = (*testerFsOps).fsops_scanvols(*diskFd, request, reply);
    logf("fsops_scanvols return (%d) -%s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}

int brdg_fsops_mount(UVFSFSOps* testerFsOps, int* diskFd, UVFSFileNode *outRootFileNode){
    int errnum;
    errnum = (*testerFsOps).fsops_mount(*diskFd, 0, 0, NULL, outRootFileNode);
    logf("fsops_mount return (%d) - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}

int brdg_fsops_unmount(UVFSFSOps* testerFsOps, UVFSFileNode* rootFileNode, UVFSUnmountHint hint){
    
    return (*testerFsOps).fsops_unmount(*rootFileNode, hint);
}


int brdg_fsops_lookup(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, UVFSFileNode* outNode){
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_lookup(*dirNode, name, outNode);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    return errnum;
}

int brdg_fsops_reclaim(UVFSFSOps* testerFsOps, UVFSFileNode* Node){
    int error;
    clock_gettime(CLOCK_REALTIME, &tm1);
    error =  (*testerFsOps).fsops_reclaim(*Node);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);

    return error;
}



int brdg_fsops_getattr(UVFSFSOps* testerFsOps, UVFSFileNode* pfileNode, UVFSFileAttributes *pattrs)
{
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_getattr(*pfileNode, pattrs);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_getattr return %d - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}

int brdg_fsops_setattr(UVFSFSOps* testerFsOps, UVFSFileNode *Node, const UVFSFileAttributes *attrs, UVFSFileAttributes *outAttrs){
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_setattr(*Node, attrs, outAttrs);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_setattr return %d - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}

int brdg_fsops_write(UVFSFSOps* testerFsOps, UVFSFileNode* Node, uint64_t offset, size_t length, const void *buf, size_t *p_actuallyWrite)
{
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_write(*Node, offset, length, buf, p_actuallyWrite);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_write() Node=%p offset=%08llX length=%08zX Buf=%08X actuallyWrite=%08zX return %d (%s) \n",*Node,offset,length,(uint32_t)buf,*p_actuallyWrite,errnum, errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}


int brdg_fsops_create(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode){
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_create(*dirNode, name, attrs, outNode);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_create return %d - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
    
    
    logf("fsops_setfsattr return (%d) - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    
    
}

int brdg_fsops_mkdir(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode){
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_mkdir(*dirNode, name, attrs, outNode);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);

    return errnum;
}

int brdg_fsops_symlink(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, const char *contents, const UVFSFileAttributes *attrs, UVFSFileNode *outNode){
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_symlink(*dirNode, name, contents, attrs, outNode);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_symlink return %d - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}

int brdg_fsops_hardlink(UVFSFSOps* testerFsOps, UVFSFileNode* fromNode, UVFSFileNode* dirNode, const char *name, UVFSFileAttributes* outFileAttrs, UVFSFileAttributes* outDirAttrs) {
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_link(*fromNode, *dirNode, name, outFileAttrs, outDirAttrs);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_link return %d - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}

int brdg_fsops_remove(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name){
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_remove(*dirNode, name, NULL);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    
    return errnum;
}

int brdg_fsops_rmdir(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name){
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_rmdir(*dirNode, name);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    
    return errnum;
}


int brdg_fsops_readdir(UVFSFSOps* testerFsOps, UVFSFileNode *dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytes_read,  uint64_t *verifier)
{
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_readdir(*dirNode, buf, buflen, cookie, bytes_read, verifier);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_readdir return (%d) %s\n",errnum,errnum==0?"SUCCESS":errnum==UVFS_READDIR_EOF_REACHED?"UVFS_READDIR_EOF_REACHED":errnum==UVFS_READDIR_BAD_COOKIE?"UVFS_READDIR_BAD_COOKIE":errnum==UVFS_READDIR_VERIFIER_MISMATCHED?"UVFS_READDIR_VERIFIER_MISMATCHED":strerror(errnum));
    return errnum;
}

int brdg_fsops_readdirattr(UVFSFSOps* testerFsOps, UVFSFileNode *dirNode, void *buf, size_t buflen, uint64_t cookie, size_t *bytes_read,  uint64_t *verifier)
{
    int errnum;
    clock_gettime(CLOCK_REALTIME, &tm1);
    errnum = (*testerFsOps).fsops_readdirattr(*dirNode, buf, buflen, cookie, bytes_read, verifier);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    logf("fsops_readdirattr return (%d) %s\n",errnum,errnum==0?"SUCCESS":errnum==UVFS_READDIR_EOF_REACHED?"UVFS_READDIR_EOF_REACHED":errnum==UVFS_READDIR_BAD_COOKIE?"UVFS_READDIR_BAD_COOKIE":errnum==UVFS_READDIR_VERIFIER_MISMATCHED?"UVFS_READDIR_VERIFIER_MISMATCHED":strerror(errnum));
    return errnum;
}

int brdg_fsops_rename(UVFSFSOps* testerFsOps, UVFSFileNode* fromDirNode, UVFSFileNode fromNode, const char *fromName, UVFSFileNode* toDirNode, UVFSFileNode toNode, const char *toName, uint32_t flags){
    int errnum;

    clock_gettime(CLOCK_REALTIME, &tm1);
 
    errnum = (*testerFsOps).fsops_rename(*fromDirNode, fromNode, fromName, *toDirNode, toNode, toName, flags);
    clock_gettime(CLOCK_REALTIME, &tm2);
    elapsed_time = timespec_diff_in_ns(&tm1, &tm2);
    return errnum;
}

int brdg_fsops_readlink(UVFSFSOps* testerFsOps, UVFSFileNode *Node, void *outBuf, size_t bufsize, size_t *actuallyRead, UVFSFileAttributes *outAttrs)
{
    int errnum;
    errnum = (*testerFsOps).fsops_readlink(*Node, outBuf, bufsize, actuallyRead, outAttrs);
    logf("fsops_readlink return (%d) - %s\n",errnum,errnum==0?"SUCCESS":strerror(errnum));
    return errnum;
}

int brdg_fsops_read(UVFSFSOps* testerFsOps, UVFSFileNode *Node, uint64_t offset, size_t length, void *outBuf, size_t *p_actuallyRead)
{
    int errnum;
    assert(*Node);
    errnum = (*testerFsOps).fsops_read(*Node, offset, length, outBuf, p_actuallyRead);
    logf("fsops_read() Node=%p offset=%08llX length=%08zX outBuf=%08X actuallyRead=%08zX return %d (%s) \n",*Node,offset,length,(uint32_t)outBuf,*p_actuallyRead,errnum,errnum==0?"SUCCESS":strerror(errnum));
    
    return errnum;
}

int brdg_fsops_sync(UVFSFSOps* testerFsOps, UVFSFileNode* node){
    
    return (*testerFsOps).fsops_sync(*node);
}

uint64_t brdg_fsops_version(UVFSFSOps* testerFsOps){
    return (*testerFsOps).fsops_version;
}

const char *check_flag_str[] = {
    "INVALID"             ,
    "QUICK_CHECK"         ,   /* Perform quick check, returning 0 if filesystem was cleanly unmounted, or 1 otherwise */
    "CHECK"               ,   /* Perform full check but no repairs. Return 0 if the filesystem is consistent, or appropriate errno otherwise */
    "CHECK_AND_REPAIR"       /* Perform full check and carry out any necessary repairs to make filesystem consistent */
};

int brdg_fsops_check(UVFSFSOps* testerFsOps, int disk_fd, check_flags_t how) {
    int errnum;
    errnum =  (*testerFsOps).fsops_check( disk_fd, 0, NULL, how );
    logf("fsops_check fd=%d, how=%s return (%d) - %s\n",
         disk_fd,
         how <= CHECK_AND_REPAIR ? check_flag_str[how] : "UNDEFINED_INPUT",
         errnum,
         errnum==0 ? "SUCCESS" : strerror(errnum) );
    return errnum;
}


/******************************************************************************
 Useful functions
 ******************************************************************************/

int brdg_createNewFile(const char* fileName, UVFSFileNode* rootFileHandle){
    int err = SUCCESS;
    //    fileNode* psFolder = rootFileHandle;
    UVFSFileAttributes attrs;
    
    attrs.fa_type = UVFS_FA_TYPE_FILE;
    attrs.fa_mode = UVFS_FA_MODE_USR(UVFS_FA_MODE_RWX);
    
    return err;
}


void print_attrs(UVFSFileAttributes *attrs){
    logf("Attributes:\n");
    logf("fa_validmask = %llu\n",  (unsigned long long)attrs->fa_validmask);
    logf("fa_type = %d\n",         attrs->fa_type);
    logf("fa_mode = %d\n",         attrs->fa_mode);
    logf("fa_nlink = %d\n",        attrs->fa_nlink);
    logf("fa_uid = %d\n",          attrs->fa_uid);
    logf("fa_gid = %d\n",          attrs->fa_gid);
    logf("fa_bsd_flags = %d\n",    attrs->fa_bsd_flags);
    logf("fa_size = %llu\n",         attrs->fa_size);
    logf("fa_allocsize = %llu\n",    attrs->fa_allocsize);
    logf("fa_fileid = %llu\n",       attrs->fa_fileid);
    logf("fa_atime = %lld.%.9ld\n",     (long long)(attrs->fa_atime.tv_sec), attrs->fa_atime.tv_nsec);
    logf("fa_mtime = %lld.%.9ld\n",     (long long)(attrs->fa_mtime.tv_sec), attrs->fa_mtime.tv_nsec);
    logf("fa_ctime = %lld.%.9ld\n",     (long long)(attrs->fa_ctime.tv_sec), attrs->fa_ctime.tv_nsec);
}


char *get_de_name(void *streamBuf, size_t offset)
{
    UVFSDirEntry *dir_entry = (UVFSDirEntry*)(streamBuf+offset);
    return (char*)&(dir_entry->de_name[0]);
}

char *get_dea_name(void *streamBuf, size_t offset)
{
   UVFSDirEntryAttr *dir_attr_entry = (UVFSDirEntryAttr*)(streamBuf+offset);
    return (char*)UVFS_DIRENTRYATTR_NAMEPTR(dir_attr_entry);
}

// Couldn't find a good way to rand in swift, so let's do it in C:
void brdg_srand(int seed) {
    srand(seed);
}

int brdg_rand(void) {
    return rand();
}

char *brdg_strerror(int errnum)
{
    return strerror(errnum);
}

size_t get_direntry_reclen(uint32_t namelen)
{
    return UVFS_DIRENTRY_RECLEN(namelen);
}

size_t get_dea_reclen(UVFSDirEntryAttr dea , uint32_t namelen)
{
    return UVFS_DIRENTRYATTR_RECLEN(&dea,namelen);
}

uint32_t brdg_convertModeToUserBits (uint32_t mode) {
    return UVFS_FA_MODE_USR(mode);
}

char *get_fs_string(UVFSFSAttributeValue *fsattr)
{
    return &fsattr->fsa_string[0];
}


uint8_t *get_fs_opaque(UVFSFSAttributeValue *fsattr)
{
    return &fsattr->fsa_opaque[0];
}

double timespec_diff_in_ns(struct timespec *start, struct timespec *stop)
{
    double diff_sec, diff_nSec, diff;
    
    diff_sec = (double)(stop->tv_sec - start->tv_sec);
    diff_nSec = (double)(stop->tv_nsec - start->tv_nsec);
    diff = diff_sec * 1000000000 + diff_nSec;
    return diff;
}

char* getBuildTime(void) { return __TIME__;}

char* getBuildDate(void) { return __DATE__;}

// This function is used by the T_dirtyBitLockTest test in order to use semaphores as closed as possible to the
// plugin operations.
//
int rValue = -1;
dispatch_semaphore_t    sem;

int dirtyBlockTest_Create(UVFSFSOps* testerFsOps, UVFSFileNode* dirNode, const char *name, const UVFSFileAttributes *attrs, UVFSFileNode *outNode){
    int errnum;
    while (sem == NULL) {sleep(1);}
    dispatch_semaphore_signal(sem);
    logf("changing the rValue to %d\n", EWOULDBLOCK);
    rValue = EWOULDBLOCK;
    errnum = (*testerFsOps).fsops_create(*dirNode, name, attrs, outNode);
    logf("changing the rValue to %d\n", SUCCESS);
    rValue = SUCCESS;
    return rValue;
}

int dirtyBlockTest_Sync(UVFSFSOps* testerFsOps, UVFSFileNode* node) {
    sem = dispatch_semaphore_create(0);
    dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
    usleep(200);    // Just for letting the Create thread call the create fsop first.
    logf("Sleep in sync ended\n");
    if (rValue == SUCCESS) { // In case that the create thread finished and sync operation didn't even start.
        logf("changing the rValue to %d\n", EPERM);
        rValue = EPERM;
    }
    logf("Calling fsops_sync\n");
    (*testerFsOps).fsops_sync(*node);
    return rValue;
}
int brdg_inject_error   (CrashAbort_E stepToFall) {
    int i;
    printf("ejecting crash abort\n");
    for (i=0; i<NOF_DYLIBS_SUPPORTED; i++) {
        if (strstr(dylibs_paths[i], "hfs") != NULL) {
            void* CrashAbortFunctionArray = dlsym(plugin_handle[i], "gpsCrashAbortFunctionArray");
            if (CrashAbortFunctionArray == NULL)
                assert(0);
            CrashAbortFunction_FP *gpsCrashAbortFunctionArray = CrashAbortFunctionArray;
            gpsCrashAbortFunctionArray[stepToFall] = &HFSTest_CrashAbort;
            break;
        }
    }
    
    return 0;
}

int brdg_reject_error(CrashAbort_E stepToFall) {
    int i;
    printf("rejecting crash abort\n");
    for (i=0; i<NOF_DYLIBS_SUPPORTED; i++) {
        if (strstr(dylibs_paths[i], "hfs") != NULL) {
            void* CrashAbortFunctionArray = dlsym(plugin_handle[i], "gpsCrashAbortFunctionArray");
            if (CrashAbortFunctionArray == NULL)
            assert(0);
            CrashAbortFunction_FP *gpsCrashAbortFunctionArray = CrashAbortFunctionArray;
            gpsCrashAbortFunctionArray[stepToFall] = NULL;
            break;
        }
    }
    
    return 0;
}

int HFSTest_CrashAbort(CrashAbort_E eAbort, int iFD, UVFSFileNode psNode, pthread_t pSyncerThread) {
    printf("In CrashAbort callback (eAbort %u)\n", eAbort);
    printf("Closing file descriptor on crashAbort\n");
    close(iFD);
    
    return 0;
}

