/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _DATA_PCKTS_H
#define _DATA_PCKTS_H

/*
 * This file contains structure definitions for the binary file formats
 * used in the experiment.  It is implemented as C header file so that
 * it can be processed by both ANSI-C and C++.
 */

#include <pthread.h>
#include <stdint.h>

#include "gp-defs.h"
#include "gp-time.h"

#if WSIZE(64)
typedef uint64_t Vaddr_type;    /* process address for 64 bit apps */
typedef uint64_t Size_type;     /* size_t for 64 bit apps */
#else
typedef uint32_t Vaddr_type;    /* process address */
typedef uint32_t Size_type;     /* size_t for 32 bit apps */
#endif

/* marker to indicate dump of O7 register on stack (support for leaf routines) */
#define SP_LEAF_CHECK_MARKER    ((uint64_t)(-1))

/* marker to indicate truncated stack */
#define SP_TRUNC_STACK_MARKER   ((uint64_t)(-2))

/* marker to indicate failed stack unwind */
#define SP_FAILED_UNWIND_MARKER ((uint64_t)(-3))

#define PROFILE_BUFFER_CHUNK    16384

typedef enum
{
  MASTER_SMPL = 0,
  PROGRAM_SMPL,
  PERIOD_SMPL,
  MANUAL_SMPL
} Smpl_type;

typedef enum
{ /* values for "profpckt kind" stored in log.xml */
  EMPTY_PCKT = 0,
  PROF_PCKT,
  SYNC_PCKT,
  HW_PCKT,
  XHWC_PCKT,
  HEAP_PCKT,
  MPI_PCKT,
  MHWC_PCKT,
  OPROF_PCKT,
  OMP_PCKT,
  RACE_PCKT,
  FRAME_PCKT,
  OMP2_PCKT,
  DEADLOCK_PCKT,
  OMP3_PCKT,
  OMP4_PCKT,
  OMP5_PCKT,
  UID_PCKT,
  FRAME2_PCKT,
  IOTRACE_PCKT,
  LAST_PCKT,            /* last data packet type */
  CLOSED_PCKT = 65535   /*  -1, this packet closes a block */
} Pckt_type;

typedef enum
{
  EMPTY_INFO = 0,
  STACK_INFO,
  JAVA_INFO,
  OMP_INFO,
  MPI_INFO,
  OMP2_INFO,
  LAST_INFO             /* keep this one last */
} Info_type;

#define COMPRESSED_INFO 0x80000000

#define JAVA_PCKT       0x80
#define OMPS_PCKT       0x40  /* packet contains OMP state info */
#define PCKT_TYPE(x)    ((x) & 0x1f)

typedef struct CommonHead_packet
{
  unsigned int tsize : 16;
  unsigned int type : 16;
} CommonHead_packet;

// All collector modules record their packets as extensions of CM_Packet
typedef struct CM_Packet
{
  unsigned int tsize : 16;
  unsigned int type : 16;
} CM_Packet;

typedef struct Common_packet
{
  unsigned int tsize : 16; /* packet size  */
  unsigned int type : 16;
  pthread_t    lwp_id;
  pthread_t    thr_id;
  uint32_t     cpu_id;
  hrtime_t     tstamp;
  uint64_t     frinfo;
} Common_packet;

/* Definition of values stored in the experiment PROP_MSTATE field */
/* They include:
 *   LWP microstates (copied from msacct.h).  Also see PrUsage class.
 *   Linux's CPU time
 *   er_kernel time
 */
/*   Can be used with LMS_STATE_STRINGS (below) */
#define LMS_USER        0   /* running in user mode */
#define LMS_SYSTEM      1   /* running in sys call or page fault */
#define LMS_TRAP        2   /* running in other trap */
#define LMS_TFAULT      3   /* asleep in user text page fault */
#define LMS_DFAULT      4   /* asleep in user data page fault */
#define LMS_KFAULT      5   /* asleep in kernel page fault */
#define LMS_USER_LOCK   6   /* asleep waiting for user-mode lock */
#define LMS_SLEEP       7   /* asleep for any other reason */
#define LMS_WAIT_CPU    8   /* waiting for CPU (latency) */
#define LMS_STOPPED     9   /* stopped (/proc, jobcontrol, or lwp_stop) */
#define LMS_LINUX_CPU   10  /* LINUX timer_create(CLOCK_THREAD_CPUTIME_ID) */
#define LMS_KERNEL_CPU  11  /* LINUX timer_create(CLOCK_THREAD_CPUTIME_ID) */
#define LMS_NUM_STATES  12  /* total number of above states */
#define LMS_NUM_SOLARIS_MSTATES     10  /* LMS microstates thru LMS_STOPPED */

// Magic value stored in experiments that identifies which LMS states are valid
#define LMS_MAGIC_ID_SOLARIS        10  // Solaris: LMS_USER thru LMS_STOPPED
#define LMS_MAGIC_ID_ERKERNEL_USER   2  // er_kernel user: LMS_USER, LMS_SYSTEM
#define LMS_MAGIC_ID_ERKERNEL_KERNEL 3  // er_kernel kernel: LMS_KERNEL_CPU
#define LMS_MAGIC_ID_LINUX           1  // Linux: LMS_LINUX_CPU

#define LMS_STATE_STRINGS \
{ \
  NTXT("USER"),         /* LMS_USER */ \
  NTXT("SYSTEM"),       /* LMS_SYSTEM */ \
  NTXT("TRAP"),         /* LMS_TRAP */ \
  NTXT("TFAULT"),       /* LMS_TFAULT */ \
  NTXT("DFAULT"),       /* LMS_DFAULT */ \
  NTXT("KFAULT"),       /* LMS_KFAULT */ \
  NTXT("USER_LOCK"),    /* LMS_USER_LOCK */ \
  NTXT("SLEEP"),        /* LMS_SLEEP */ \
  NTXT("WAIT_CPU"),     /* LMS_WAIT_CPU */ \
  NTXT("STOPPED"),      /* LMS_STOPPED */ \
  NTXT("LINUX_CPU"),    /* LMS_LINUX_CPU */ \
  NTXT("KERNEL_CPU")    /* LMS_KERNEL_CPU */ \
}
#define LMS_STATE_USTRINGS \
{ \
  GTXT("User CPU"),             /* LMS_USER */ \
  GTXT("System CPU"),           /* LMS_SYSTEM */ \
  GTXT("Trap CPU"),             /* LMS_TRAP */ \
  GTXT("Text Page Fault"),      /* LMS_TFAULT */ \
  GTXT("Data Page Fault"),      /* LMS_DFAULT */ \
  GTXT("Kernel Page Fault"),    /* LMS_KFAULT */ \
  GTXT("User Lock"),            /* LMS_USER_LOCK */ \
  GTXT("Sleep"),                /* LMS_SLEEP */ \
  GTXT("Wait CPU"),             /* LMS_WAIT_CPU */ \
  GTXT("Stopped"),              /* LMS_STOPPED */ \
  GTXT("User+System CPU"),      /* LMS_LINUX_CPU */ \
  GTXT("Kernel CPU")            /* LMS_KERNEL_CPU */ \
}

typedef enum
{
  MALLOC_TRACE = 0,
  FREE_TRACE,
  REALLOC_TRACE,
  MMAP_TRACE,
  MUNMAP_TRACE,
  HEAPTYPE_LAST
} Heap_type;

#define HEAPTYPE_STATE_STRINGS \
{ \
    NTXT("MALLOC"), \
    NTXT("FREE"), \
    NTXT("REALLOC"), \
    NTXT("MMAP"), \
    NTXT("MUNMAP") \
}
#define HEAPTYPE_STATE_USTRINGS \
{ \
    GTXT("malloc"), \
    GTXT("free"), \
    GTXT("realloc"), \
    GTXT("mmap"), \
    GTXT("munmap") \
}

typedef enum
{
  ZFS_TYPE = 0,
  NFS_TYPE,
  UFS_TYPE,
  UDFS_TYPE,
  LOFS_TYPE,
  VXFS_TYPE,
  TMPFS_TYPE,
  PCFS_TYPE,
  HSFS_TYPE,
  PROCFS_TYPE,
  FIFOFS_TYPE,
  SWAPFS_TYPE,
  CACHEFS_TYPE,
  AUTOFS_TYPE,
  SPECFS_TYPE,
  SOCKFS_TYPE,
  FDFS_TYPE,
  MNTFS_TYPE,
  NAMEFS_TYPE,
  OBJFS_TYPE,
  SHAREFS_TYPE,
  EXT2FS_TYPE,
  EXT3FS_TYPE,
  EXT4FS_TYPE,
  UNKNOWNFS_TYPE,
  FSTYPE_LAST
} FileSystem_type;

typedef enum
{
  READ_TRACE = 0,
  WRITE_TRACE,
  OPEN_TRACE,
  CLOSE_TRACE,
  OTHERIO_TRACE,
  READ_TRACE_ERROR,
  WRITE_TRACE_ERROR,
  OPEN_TRACE_ERROR,
  CLOSE_TRACE_ERROR,
  OTHERIO_TRACE_ERROR,
  IOTRACETYPE_LAST
} IOTrace_type;

#define IOTRACETYPE_STATE_STRINGS \
{ \
  NTXT("READ"), \
  NTXT("WRITE"), \
  NTXT("OPEN"), \
  NTXT("CLOSE"), \
  NTXT("OTHERIO"), \
  NTXT("READERROR"), \
  NTXT("WRITEERROR"), \
  NTXT("OPENERROR"), \
  NTXT("CLOSEERROR"), \
  NTXT("OTHERIOERROR") \
}
#define IOTRACETYPE_STATE_USTRINGS \
{ \
  GTXT("Read"), \
  GTXT("Write"), \
  GTXT("Open"), \
  GTXT("Close"), \
  GTXT("Other I/O"), \
  GTXT("Read error"), \
  GTXT("Write error"), \
  GTXT("Open error"), \
  GTXT("Close error"), \
  GTXT("Other I/O error") \
}

// the type of racing memory access with redundance flag
typedef enum
{
  WRITE_RACE = 0,
  WRITE_RACE_RED,
  READ_RACE,
  READ_RACE_RED,
  RACETYPE_LAST
} Race_type;

typedef struct Frame_packet
{
  unsigned int tsize : 16; /* packet size */
  unsigned int type : 16;
  uint32_t hsize;           /* header size */
  uint64_t uid;             /* unique id (experiment wide) */
} Frame_packet;

typedef struct Uid_packet
{
  unsigned int tsize : 16;  /* packet size */
  unsigned int type : 16;
  uint32_t flags;
  uint64_t uid;             /* unique id (experiment wide) */
} Uid_packet;

/*
 * Components of the variable part of Frame_packet
 */
typedef struct Common_info
{
  unsigned int hsize;   /* size of this info */
  unsigned int kind;
  uint64_t uid;         /* unique id of this info if any */
} Common_info;

typedef struct Stack_info
{ /* Native call stack */
  unsigned int hsize;
  unsigned int kind;
  uint64_t uid;
} Stack_info;

typedef struct Java_info
{ /* Java call stack */
  unsigned int hsize;
  unsigned int kind;
  uint64_t uid;
} Java_info;

typedef struct OMP_info
{ /* OMP thread state */
  unsigned int hsize;
  unsigned int kind;
  uint32_t omp_state;
  uint32_t pad;
} OMP_info;

typedef struct OMP2_info
{ /* OpenMP user call stack */
  unsigned int hsize;
  unsigned int kind;
  uint32_t omp_state;
  uint32_t pad;
  uint64_t uid;
} OMP2_info;

/* OMP thread states as recorded in the experiment */
/*   Definition of values stored in the experiment PROP_OMPSTATE field */

/*   Can be used with OMP_THR_STATE_STRINGS (below) */
typedef enum
{
  OMP_NO_STATE = 0, /* Not initialized */
  OMP_OVHD_STATE, /* Overhead */
  OMP_WORK_STATE, /* Useful work, excluding reduction, master, single, critical */
  OMP_IBAR_STATE, /* In an implicit barrier */
  OMP_EBAR_STATE, /* In an explicit barrier */
  OMP_IDLE_STATE, /* Slave waiting */
  OMP_SERL_STATE, /* User OMPead not in any OMP parallel region */
  OMP_RDUC_STATE, /* Reduction */
  OMP_LKWT_STATE, /* Waiting for lock */
  OMP_CTWT_STATE, /* Waiting to enter critical section */
  OMP_ODWT_STATE, /* Waiting to execute an ordered section */
  OMP_ATWT_STATE, /* Wait for atomic */
  OMP_TSKWT_STATE, /* Task wait */
  OMP_LAST_STATE
} OMP_THR_STATE;
#define OMP_THR_STATE_STRINGS \
{ \
  NTXT("NO"),       /* OMP_NO_STATE */ \
  NTXT("OVHD"),     /* OMP_OVHD_STATE */ \
  NTXT("WORK"),     /* OMP_WORK_STATE */ \
  NTXT("IBAR"),     /* OMP_IBAR_STATE */ \
  NTXT("EBAR"),     /* OMP_EBAR_STATE */ \
  NTXT("IDLE"),     /* OMP_IDLE_STATE */ \
  NTXT("SERL"),     /* OMP_SERL_STATE */ \
  NTXT("RDUC"),     /* OMP_RDUC_STATE */ \
  NTXT("LKWT"),     /* OMP_LKWT_STATE */ \
  NTXT("CTWT"),     /* OMP_CTWT_STATE */ \
  NTXT("ODWT"),     /* OMP_ODWT_STATE */ \
  NTXT("ATWT"),     /* OMP_ATWT_STATE */ \
  NTXT("TSKWT")     /* OMP_TSKWT_STATE */ \
}
#define OMP_THR_STATE_USTRINGS \
{ \
  GTXT("None"),                 /* OMP_NO_STATE */ \
  GTXT("Overhead"),             /* OMP_OVHD_STATE */ \
  GTXT("Work"),                 /* OMP_WORK_STATE */ \
  GTXT("Implicit Barrier"),     /* OMP_IBAR_STATE */ \
  GTXT("Explicit Barrier"),     /* OMP_EBAR_STATE */ \
  GTXT("Idle"),                 /* OMP_IDLE_STATE */ \
  GTXT("Serial"),               /* OMP_SERL_STATE */ \
  GTXT("Reduction"),            /* OMP_RDUC_STATE */ \
  GTXT("Lock Wait"),            /* OMP_LKWT_STATE */ \
  GTXT("Critical Section Wait"), /* OMP_CTWT_STATE */ \
  GTXT("Ordered Section Wait"), /* OMP_ODWT_STATE */ \
  GTXT("Atomic Wait"),          /* OMP_ATWT_STATE */ \
  GTXT("Task Wait")             /* OMP_TSKWT_STATE */ \
}

/* sub-packet for MPI state information */
typedef struct MPI_info
{ /* MPI thread state */
  unsigned int hsize;
  unsigned int kind;
  uint32_t mpi_state;
  uint32_t pad;
} MPI_info;

/* MPI thread states, as recorded in the experiment */
typedef enum
{
  MPI_NO_STATE = 0,     /* Not initialized */
  MPI_USER,             /* Executing user code, not in MPI */
  MPI_PROG,             /* Executing in the MPI library (progressing) */
  MPI_WAIT              /* Waiting in the MPI library */
} MPI_THR_STATE;

/*
 *	Dyntext file structure
 */
typedef enum
{
  DT_HEADER = 1,
  DT_CODE,
  DT_LTABLE,
  DT_SRCFILE
} DT_type;

typedef struct DT_common
{
  DT_type type;
  unsigned int size;
} DT_common;

typedef struct DT_header
{
  DT_type type;
  unsigned int size;
  hrtime_t time; /* time of loading */
  uint64_t vaddr;
} DT_header;

typedef struct DT_code
{
  DT_type type;
  unsigned int size;
} DT_code;

typedef struct DT_ltable
{
  DT_type type;
  unsigned int size;
} DT_ltable;

typedef struct DT_lineno
{
  unsigned int offset;
  unsigned int lineno;
} DT_lineno;

typedef struct DT_srcfile
{
  DT_type type;
  unsigned int size;
} DT_srcfile;

/*
 *	Archive file structure
 */
#define ARCH_VERSION 0x100 /* version 1.0 */

/* For compatibility with older archives append new types only */
typedef enum
{
  ARCH_SEGMENT_TYPE = 1,
  ARCH_MSG_TYPE,
  ARCH_PLT_TYPE,
  ARCH_MODULE_TYPE,
  ARCH_FUNCTION_TYPE,
  ARCH_LDINSTR_TYPE,
  ARCH_STINSTR_TYPE,
  ARCH_PREFETCH_TYPE,
  ARCH_BRTARGET_TYPE,
  ARCH_JCLASS_TYPE,
  ARCH_JMETHOD_TYPE,
  ARCH_JUNLOAD_TYPE,
  ARCH_INF_TYPE,
  ARCH_JCLASS_LOCATION_TYPE
} ARCH_type;

#define ARCH_TYPE(x,y)      ((ARCH_##x##_TYPE<<8)|y)

typedef struct
{
  unsigned int type : 16;
  unsigned int size : 16;
} ARCH_common;

/* The maximum value that fits into ARCH_common.size */
#define ARCH_MAX_SIZE 0xffff

#define ARCH_SEGMENT ARCH_TYPE(SEGMENT, 0)

typedef struct
{
  ARCH_common common;
  int version;
  uint32_t inode;
  uint32_t textsz;      /* text segment size */
  uint32_t platform;    /* sparc, intel, etc. */
} ARCH_segment;

#define ARCH_MSG ARCH_TYPE(MSG, 0)

typedef struct
{
  ARCH_common common;
  uint32_t errcode;
} ARCH_message;

#define ARCH_INF ARCH_TYPE(INF, 0)

typedef struct
{
  ARCH_common common;
} ARCH_info;

#define ARCH_MODULE ARCH_TYPE(MODULE, 0)

typedef struct
{
  ARCH_common common;
  unsigned int lang_code;
  unsigned int fragmented;
} ARCH_module;

#define ARCH_FUNCTION ARCH_TYPE(FUNCTION, 0)

typedef struct
{
  ARCH_common common;
  uint32_t offset;
  uint32_t size;
  uint32_t save_addr;
} ARCH_function;

#define ARCH_LDINSTR  ARCH_TYPE(LDINSTR, 0)
#define ARCH_STINSTR  ARCH_TYPE(STINSTR, 0)
#define ARCH_PREFETCH ARCH_TYPE(PREFETCH, 0)
#define ARCH_BRTARGET ARCH_TYPE(BRTARGET, 0)

typedef struct
{
  ARCH_common common;
} ARCH_aninfo;

#define ARCH_JCLASS_LOCATION ARCH_TYPE(JCLASS_LOCATION, 3)

typedef struct
{
  CM_Packet comm;
  uint32_t pad;
  uint64_t class_id;
} ARCH_jclass_location;

#define ARCH_JCLASS ARCH_TYPE(JCLASS, 3)

typedef struct
{
  CM_Packet comm;
  uint32_t pad;
  uint64_t class_id;
  hrtime_t tstamp;
} ARCH_jclass;

#define ARCH_JMETHOD ARCH_TYPE(JMETHOD, 3)

typedef struct
{
  CM_Packet comm;
  uint32_t pad;
  uint64_t class_id;
  uint64_t method_id;
} ARCH_jmethod;

#endif /* _DATA_PCKTS_H */
