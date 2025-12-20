/* include/config.h.  Generated from config.h.in by configure.  */
/* #undef HAVE_SYS_STATFS_H */
#define HAVE_SYS_XATTR_H 1
/* #undef HAVE_SYS_EXTATTR_H */
#define HAVE_SYS_PARAM_H 1
/* #undef HAVE_LGETXATTR */
/* #undef HAVE_LSETXATTR */
#define HAVE_GETXATTR 1
#define HAVE_SETXATTR 1
#define HAVE_GETATTRLIST 1
#define HAVE_SETATTRLIST 1
#ifdef __linux__
#undef HAVE_CHFLAGS
#else
#define HAVE_CHFLAGS 1
#endif
#define HAVE_STATVFS 1
#define HAVE_STATFS 1
/* #undef HAVE_EXT2FS_EXT2_FS_H */
#ifdef __linux__
#undef  HAVE_STRUCT_STAT_ST_FLAGS
#else
#define HAVE_STRUCT_STAT_ST_FLAGS 1
#endif
/* #undef HAVE_STRUCT_STATVFS_F_FSTYPENAME */
#define HAVE_STRUCT_STATFS_F_FSTYPENAME 1
#define HAVE_SYS_ACL_H 1
/* #undef HAVE_LIBUTIL_H */
#define HAVE_ASPRINTF 1
/* #undef HAVE_LIBBZ2 */
/* #undef HAVE_LIBLZMA */
#define HAVE_LCHOWN 1
#define HAVE_LCHMOD 1
#ifndef __linux__
#define HAVE_STRMODE 1
#endif
#define UID_STRING RId32
#define UID_CAST (uint32_t)
#define GID_STRING PRId32
#define GID_CAST (uint32_t)
#define INO_STRING PRId64
#define INO_HEXSTRING PRIx64
#define INO_CAST (uint64_t)
#define DEV_STRING PRId32
#define DEV_HEXSTRING PRIx32
#define DEV_CAST (uint32_t)
