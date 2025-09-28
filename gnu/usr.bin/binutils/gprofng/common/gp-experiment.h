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

#ifndef _EXPERIMENT_H
#define _EXPERIMENT_H

/* version numbers define experiment format */
#define SUNPERF_VERNUM          12
#define SUNPERF_VERNUM_MINOR    4

/* backward compatibility down to: */
#define SUNPERF_VERNUM_LEAST    12

#include "Emsgnum.h" /* for COL_ERROR_*, etc. symbols */

#define SP_REMOTE_PROTOCOL_VERSION "12.4.1"

#define SP_GROUP_HEADER         "#analyzer experiment group"

/* Experiment name macro definitions */

/* for descendant experiments */
#define DESCENDANT_EXPT_KEY     ".er/_"
#define IS_DESC_EXPT(exptname)  (strstr(exptname,DESCENDANT_EXPT_KEY) != NULL)
#define IS_FNDR_EXPT(exptname)  (strstr(exptname,DESCENDANT_EXPT_KEY) == NULL)

/* File name definitions */
#define SP_ARCHIVES_DIR         "archives"
#define SP_ARCHIVE_LOG_FILE     "archive.log"
#define SP_LOG_FILE             "log.xml"
#define SP_NOTES_FILE           "notes"
#define SP_IFREQ_FILE           "ifreq"
#define SP_MAP_FILE             "map.xml"
#define SP_LABELS_FILE          "labels.xml"
#define SP_DYNTEXT_FILE         "dyntext"
#define SP_OVERVIEW_FILE        "overview"
#define SP_PROFILE_FILE         "profile"
#define SP_SYNCTRACE_FILE       "synctrace"
#define SP_IOTRACE_FILE         "iotrace"
#define SP_OMPTRACE_FILE        "omptrace"
#define SP_MPVIEW_FILE          "mpview.dat3"
#define SP_HWCNTR_FILE          "hwcounters"
#define SP_HEAPTRACE_FILE       "heaptrace"
#define SP_JCLASSES_FILE        "jclasses"
#define SP_DYNAMIC_CLASSES      "jdynclasses"
#define SP_RACETRACE_FILE       "dataraces"
#define SP_DEADLOCK_FILE        "deadlocks"
#define SP_FRINFO_FILE          "frameinfo"
#define SP_WARN_FILE            "warnings.xml"

#define SP_LIBCOLLECTOR_NAME    "libgp-collector.so"
#define SP_LIBAUDIT_NAME        "libcollect-ng.so"

/* XML tags */
#define SP_TAG_COLLECTOR        "collector"
#define SP_TAG_CPU              "cpu"
#define SP_TAG_DATAPTR          "dataptr"
#define SP_TAG_EVENT            "event"
#define SP_TAG_EXPERIMENT       "experiment"
#define SP_TAG_FIELD            "field"
#define SP_TAG_PROCESS          "process"
#define SP_TAG_PROFILE          "profile"
#define SP_TAG_PROFDATA         "profdata"
#define SP_TAG_PROFPCKT         "profpckt"
#define SP_TAG_SETTING          "setting"
#define SP_TAG_STATE            "state"
#define SP_TAG_SYSTEM           "system"
#define SP_TAG_POWERM           "powerm"
#define SP_TAG_FREQUENCY        "frequency"
#define SP_TAG_DTRACEFATAL      "dtracefatal"

/* records for log and loadobjects files */
/* note that these are in alphabetical order */
#define SP_JCMD_ARCH            "architecture"
#define SP_JCMD_ARCHIVE         "archive_run"
#define SP_JCMD_ARGLIST         "arglist"
#define SP_JCMD_BLKSZ           "blksz"
#define SP_JCMD_CERROR          "cerror"
#define SP_JCMD_CLASS_LOAD      "class_load"
#define SP_JCMD_CLASS_UNLOAD    "class_unload"
#define SP_JCMD_COLLENV         "collenv"
#define SP_JCMD_COMMENT         "comment"
#define SP_JCMD_CPUID           "cpuid"
#define SP_JCMD_CWARN           "cwarn"
#define SP_JCMD_CWD             "cwd"
#define SP_JCMD_CVERSION        "cversion"
#define SP_JCMD_DATARACE        "datarace"
#define SP_JCMD_DEADLOCK        "deadlock"
#define SP_JCMD_DELAYSTART      "delay_start"
#define SP_JCMD_DESC_START      "desc_start"
#define SP_JCMD_DESC_STARTED    "desc_started"
#define SP_JCMD_DVERSION        "dversion"
#define SP_JCMD_EXEC_START      "exec_start"
#define SP_JCMD_EXEC_ERROR      "exec_error"
#define SP_JCMD_EXIT            "exit"
#define SP_JCMD_EXPT_DURATION   "exp_duration"
#define SP_JCMD_FAKETIME        "faketime"
#define SP_JCMD_FN_LOAD         "fn_load"
#define SP_JCMD_FN_UNLOAD       "fn_unload"
#define SP_JCMD_FUN_MAP         "fun_map"
#define SP_JCMD_FUN_UNMAP       "fun_unmap"
#define SP_JCMD_HEAPTRACE       "heaptrace"
#define SP_JCMD_HOSTNAME        "hostname"
#define SP_JCMD_HWC_DEFAULT     "hwc_default"
#define SP_JCMD_HW_COUNTER      "hwcounter"
#define SP_JCMD_HW_SIM_CTR      "hwsimctr"
#define SP_JCMD_IOTRACE         "iotrace"
#define SP_JCMD_JCM_LOAD        "jcm_load"
#define SP_JCMD_JCM_UNLOAD      "jcm_unload"
#define SP_JCMD_JCM_MAP         "jcm_map"
#define SP_JCMD_JCM_UNMAP       "jcm_unmap"
#define SP_JCMD_JTHREND         "jthread_end"
#define SP_JCMD_JTHRSTART       "jthread_start"
#define SP_JCMD_GCEND           "gc_end"
#define SP_JCMD_GCSTART         "gc_start"
#define SP_JCMD_JVERSION        "jversion"
//#define SP_JCMD_KPROFILE        "kprofile"    /* TBR */
#define SP_JCMD_LIMIT           "limit"
#define SP_JCMD_LINETRACE       "linetrace"
#define SP_JCMD_LO_OPEN         "lo_open"
#define SP_JCMD_LO_CLOSE        "lo_close"
#define SP_JCMD_MOD_OPEN        "mod_open"
#define SP_JCMD_MPIEXP          "MPIexperiment"
#define SP_JCMD_MPI_NO_TRACE    "MPI_no_trace"
#define SP_JCMD_MPIOMPVER       "mpi_openmpi_version"
#define SP_JCMD_MPITRACEVER     "mpi_trace_version"
#define SP_JCMD_MPIPP           "mpipp"
#define SP_JCMD_MPIPPERR        "mpipp_err"
#define SP_JCMD_MPIPPWARN       "mpipp_warn"
#define SP_JCMD_MPISTATE        "mpistate"
#define SP_JCMD_MPITRACE        "mpitrace" /* backwards compat only */
#define SP_JCMD_MPVIEW          "mpview"
#define SP_JCMD_MSGTRACE        "msgtrace"
#define SP_JCMD_NOIDLE          "noidle"
#define SP_JCMD_OMPTRACE        "omptrace"
#define SP_JCMD_OS              "os"
#define SP_JCMD_PAGESIZE        "pagesize"
#define SP_JCMD_PAUSE           "pause"
#define SP_JCMD_PAUSE_SIG       "pause_signal"
#define SP_JCMD_PROFILE         "profile"
#define SP_JCMD_RESUME          "resume"
#define SP_JCMD_RUN             "run"
#define SP_JCMD_SAMPLE          "sample"
#define SP_JCMD_SAMPLE_PERIOD   "sample_period"
#define SP_JCMD_SAMPLE_SIG      "sample_signal"
#define SP_JCMD_SEGMENT_MAP     "seg_map"
#define SP_JCMD_SEGMENT_UNMAP   "seg_unmap"
#define SP_JCMD_SRCHPATH        "search_path"
#define SP_JCMD_STACKBASE       "stackbase"
#define SP_JCMD_SUNPERF         "sunperf"
#define SP_JCMD_SYNCTRACE       "synctrace"
#define SP_JCMD_TERMINATE       "terminate"
#define SP_JCMD_THREAD_PAUSE    "thread_pause"
#define SP_JCMD_THREAD_RESUME   "thread_resume"
#define SP_JCMD_USERNAME        "username"
#define SP_JCMD_VERSION         "version"
#define SP_JCMD_WSIZE           "wsize"

/* strings naming memory-segments */
#define SP_MAP_ANON             "Anon"
#define SP_MAP_HEAP             "Heap"
#define SP_MAP_STACK            "Stack"
#define SP_MAP_SHMEM            "SHMid"
#define SP_MAP_UNRESOLVABLE     "Unresolvable"

#define SP_UNKNOWN_NAME         "(unknown)"

#define MAX_STACKDEPTH 2048
#endif /* _EXPERIMENT_H */
