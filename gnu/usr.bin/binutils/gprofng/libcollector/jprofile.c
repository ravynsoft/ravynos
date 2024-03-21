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

#include "config.h"

#if defined(GPROFNG_JAVA_PROFILING)
#include <alloca.h>
#include <dlfcn.h> /* dlsym()	*/
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/param.h> /* MAXPATHLEN */

#include <jni.h>
#include <jvmti.h>

#include "gp-defs.h"
#include "collector.h"
#include "gp-experiment.h"
#include "tsd.h"

/* TprintfT(<level>,...) definitions.  Adjust per module as needed */
#define DBG_LT0 0 // for high-level configuration, unexpected errors/warnings
#define DBG_LT1 1 // for configuration details, warnings
#define DBG_LT2 2
#define DBG_LT3 3

/* ARCH_STRLEN is defined in dbe, copied here */
#define ARCH_STRLEN(s)      ((CALL_UTIL(strlen)(s) + 4 ) & ~0x3)

/* call frame */
typedef struct
{
  jint lineno;              /* line number in the source file */
  jmethodID method_id;      /* method executed in this frame */
} JVMPI_CallFrame;

/* call trace */
typedef struct
{
  JNIEnv *env_id;           /* Env where trace was recorded */
  jint num_frames;          /* number of frames in this trace */
  JVMPI_CallFrame *frames;  /* frames */
} JVMPI_CallTrace;

extern void __collector_jprofile_enable_synctrace (void);
int __collector_jprofile_start_attach (void);
static int init_interface (CollectorInterface*);
static int open_experiment (const char *);
static int close_experiment (void);
static int detach_experiment (void);
static void jprof_find_asyncgetcalltrace (void);
static char *apistr = NULL;

static ModuleInterface module_interface = {
  "*"SP_JCLASSES_FILE,      /* description, exempt from limit */
  init_interface,           /* initInterface */
  open_experiment,          /* openExperiment */
  NULL,                     /* startDataCollection */
  NULL,                     /* stopDataCollection */
  close_experiment,         /* closeExperiment */
  detach_experiment         /* detachExperiment (fork child) */
};

static CollectorInterface *collector_interface = NULL;
static CollectorModule jprof_hndl = COLLECTOR_MODULE_ERR;
static int __collector_java_attach = 0;
static JavaVM *jvm;
static jmethodID getResource = NULL;
static jmethodID toExternalForm = NULL;

/* Java profiling thread specific data */
typedef struct TSD_Entry
{
  JNIEnv *env;
  hrtime_t tstamp;
} TSD_Entry;

static unsigned tsd_key = COLLECTOR_TSD_INVALID_KEY;
static collector_mutex_t jclasses_lock = COLLECTOR_MUTEX_INITIALIZER;
static int java_gc_on = 0;
static int java_mem_mode = 0;
static int java_sync_mode = 0;
static int is_hotspot_vm = 0;
static void get_jvm_settings ();
static void rwrite (int fd, const void *buf, size_t nbyte);
static void addToDynamicArchive (const char* name, const unsigned char* class_data, int class_data_len);
static void (*AsyncGetCallTrace)(JVMPI_CallTrace*, jint, ucontext_t*) = NULL;
static void (*collector_heap_record)(int, int, void*) = NULL;
static void (*collector_jsync_begin)() = NULL;
static void (*collector_jsync_end)(hrtime_t, void *) = NULL;

#define gethrtime collector_interface->getHiResTime

/*
 * JVMTI declarations
 */

static jvmtiEnv *jvmti;
static void jvmti_VMInit (jvmtiEnv*, JNIEnv*, jthread);
static void jvmti_VMDeath (jvmtiEnv*, JNIEnv*);
static void jvmti_ThreadStart (jvmtiEnv*, JNIEnv*, jthread);
static void jvmti_ThreadEnd (jvmtiEnv*, JNIEnv*, jthread);
static void jvmti_CompiledMethodLoad (jvmtiEnv*, jmethodID, jint, const void*,
				      jint, const jvmtiAddrLocationMap*, const void*);
static void jvmti_CompiledMethodUnload (jvmtiEnv*, jmethodID, const void*);
static void jvmti_DynamicCodeGenerated (jvmtiEnv*, const char*, const void*, jint);
static void jvmti_ClassPrepare (jvmtiEnv*, JNIEnv*, jthread, jclass);
static void jvmti_ClassLoad (jvmtiEnv*, JNIEnv*, jthread, jclass);
//static void jvmti_ClassUnload( jvmtiEnv*, JNIEnv*, jthread, jclass );
static void jvmti_MonitorEnter (jvmtiEnv *, JNIEnv*, jthread, jobject);
static void jvmti_MonitorEntered (jvmtiEnv *, JNIEnv*, jthread, jobject);
#if 0
static void jvmti_MonitorWait (jvmtiEnv *, JNIEnv*, jthread, jobject, jlong);
static void jvmti_MonitorWaited (jvmtiEnv *, JNIEnv*, jthread, jobject, jboolean);
#endif
static void jvmti_ClassFileLoadHook (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jclass class_being_redefined,
				     jobject loader, const char* name, jobject protection_domain,
				     jint class_data_len, const unsigned char* class_data,
				     jint* new_class_data_len, unsigned char** new_class_data);
static void jvmti_GarbageCollectionStart (jvmtiEnv *);
static void
jvmti_GarbageCollectionFinish (jvmtiEnv *);
jvmtiEventCallbacks callbacks = {
  jvmti_VMInit,                 // 50 jvmtiEventVMInit;
  jvmti_VMDeath,                // 51 jvmtiEventVMDeath;
  jvmti_ThreadStart,            // 52 jvmtiEventThreadStart;
  jvmti_ThreadEnd,              // 53 jvmtiEventThreadEnd;
  jvmti_ClassFileLoadHook,      // 54 jvmtiEventClassFileLoadHook;
  jvmti_ClassLoad,              // 55 jvmtiEventClassLoad;
  jvmti_ClassPrepare,           // 56 jvmtiEventClassPrepare;
  NULL,                         // 57 reserved57;
  NULL,                         // 58 jvmtiEventException;
  NULL,                         // 59 jvmtiEventExceptionCatch;
  NULL,                         // 60 jvmtiEventSingleStep;
  NULL,                         // 61 jvmtiEventFramePop;
  NULL,                         // 62 jvmtiEventBreakpoint;
  NULL,                         // 63 jvmtiEventFieldAccess;
  NULL,                         // 64 jvmtiEventFieldModification;
  NULL,                         // 65 jvmtiEventMethodEntry;
  NULL,                         // 66 jvmtiEventMethodExit;
  NULL,                         // 67 jvmtiEventNativeMethodBind;
  jvmti_CompiledMethodLoad,     // 68 jvmtiEventCompiledMethodLoad;
  jvmti_CompiledMethodUnload,   // 69 jvmtiEventCompiledMethodUnload;
  jvmti_DynamicCodeGenerated,   // 70 jvmtiEventDynamicCodeGenerated;
  NULL,                         // 71 jvmtiEventDataDumpRequest;
  NULL,                         // 72 jvmtiEventDataResetRequest;
  NULL, /*jvmti_MonitorWait,*/  // 73 jvmtiEventMonitorWait;
  NULL, /*jvmti_MonitorWaited,*/ // 74 jvmtiEventMonitorWaited;
  jvmti_MonitorEnter,           // 75 jvmtiEventMonitorContendedEnter;
  jvmti_MonitorEntered,         // 76 jvmtiEventMonitorContendedEntered;
  NULL,                         // 77 jvmtiEventMonitorContendedExit;
  NULL,                         // 78 jvmtiEventReserved;
  NULL,                         // 79 jvmtiEventReserved;
  NULL,                         // 80 jvmtiEventReserved;
  jvmti_GarbageCollectionStart, // 81 jvmtiEventGarbageCollectionStart;
  jvmti_GarbageCollectionFinish, // 82 jvmtiEventGarbageCollectionFinish;
  NULL,                         // 83 jvmtiEventObjectFree;
  NULL                          // 84 jvmtiEventVMObjectAlloc;
};

typedef jint (JNICALL JNI_GetCreatedJavaVMs_t)(JavaVM **, jsize, jsize *);

int
init_interface (CollectorInterface *_collector_interface)
{
  collector_interface = _collector_interface;
  return COL_ERROR_NONE;
}

static int
open_experiment (const char *exp)
{
  if (collector_interface == NULL)
    return COL_ERROR_JAVAINIT;
  TprintfT (0, "jprofile: open_experiment %s\n", exp);
  const char *params = collector_interface->getParams ();
  const char *args = params;
  while (args)
    {
      if (__collector_strStartWith (args, "j:") == 0)
	{
	  args += 2;
	  break;
	}
      args = CALL_UTIL (strchr)(args, ';');
      if (args)
	args++;
    }
  if (args == NULL)     /* Java profiling not specified */
    return COL_ERROR_JAVAINIT;
  tsd_key = collector_interface->createKey (sizeof ( TSD_Entry), NULL, NULL);
  if (tsd_key == (unsigned) - 1)
    {
      TprintfT (0, "jprofile: TSD key create failed.\n");
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">TSD key not created</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_JAVAINIT);
      return COL_ERROR_JAVAINIT;
    }
  else
    Tprintf (DBG_LT2, "jprofile: TSD key create succeeded %d.\n", tsd_key);

  args = params;
  while (args)
    {
      if (__collector_strStartWith (args, "H:") == 0)
	{
	  java_mem_mode = 1;
	  collector_heap_record = (void(*)(int, int, void*))dlsym (RTLD_DEFAULT, "__collector_heap_record");
	}
#if 0
      else if (__collector_strStartWith (args, "s:") == 0)
	{
	  java_sync_mode = 1;
	  collector_jsync_begin = (void(*)(hrtime_t, void *))dlsym (RTLD_DEFAULT, "__collector_jsync_begin");
	  collector_jsync_end = (void(*)(hrtime_t, void *))dlsym (RTLD_DEFAULT, "__collector_jsync_end");
	}
#endif
      args = CALL_UTIL (strchr)(args, ';');
      if (args)
	args++;
    }

  /* synchronization tracing is enabled by the synctrace module, later in initialization */
  __collector_java_mode = 1;
  java_gc_on = 1;
  return COL_ERROR_NONE;
}

/* routine called from the syntrace module to enable Java-API synctrace */
void
__collector_jprofile_enable_synctrace ()
{
  if (__collector_java_mode == 0)
    {
      TprintfT (DBG_LT1, "jprofile: not turning on Java synctrace; Java mode not enabled\n");
      return;
    }
  java_sync_mode = 1;
  collector_jsync_begin = (void(*)(hrtime_t, void *))dlsym (RTLD_DEFAULT, "__collector_jsync_begin");
  collector_jsync_end = (void(*)(hrtime_t, void *))dlsym (RTLD_DEFAULT, "__collector_jsync_end");
  TprintfT (DBG_LT1, "jprofile: turning on Java synctrace, and requesting events\n");
}

int
__collector_jprofile_start_attach (void)
{
  if (!__collector_java_mode || __collector_java_asyncgetcalltrace_loaded)
    return 0;
  void *g_sHandle = RTLD_DEFAULT;
  /* Now get the function addresses */
  JNI_GetCreatedJavaVMs_t *pfnGetCreatedJavaVMs;
  pfnGetCreatedJavaVMs = (JNI_GetCreatedJavaVMs_t *) dlsym (g_sHandle, "JNI_GetCreatedJavaVMs");
  if (pfnGetCreatedJavaVMs != NULL)
    {
      TprintfT (0, "jprofile attach: pfnGetCreatedJavaVMs is detected.\n");
      JavaVM * vmBuf[1]; // XXXX only detect on jvm
      jsize nVMs = 0;
      (*pfnGetCreatedJavaVMs)(vmBuf, 1, &nVMs);
      if (vmBuf[0] != NULL && nVMs > 0)
	{
	  jvm = vmBuf[0];
	  JNIEnv* jni_env = NULL;
	  (*jvm)->AttachCurrentThread (jvm, (void **) &jni_env, NULL);
	  Agent_OnLoad (jvm, NULL, NULL);
	  if ((*jvm)->GetEnv (jvm, (void **) &jni_env, JNI_VERSION_1_2) >= 0 && jni_env && jvmti)
	    {
	      jthread thread;
	      (*jvmti)->GetCurrentThread (jvmti, &thread);
	      jvmti_VMInit (jvmti, jni_env, thread);
	      (*jvmti)->GenerateEvents (jvmti, JVMTI_EVENT_COMPILED_METHOD_LOAD);
	      (*jvmti)->GenerateEvents (jvmti, JVMTI_EVENT_DYNAMIC_CODE_GENERATED);
	      __collector_java_attach = 1;
	      (*jvm)->DetachCurrentThread (jvm);
	    }
	}
    }
  return 0;
}

static int
close_experiment (void)
{
  /* fixme XXXXX add content here */
  /* see detach_experiment() */
  __collector_java_mode = 0;
  __collector_java_asyncgetcalltrace_loaded = 0;
  __collector_java_attach = 0;
  java_gc_on = 0;
  java_mem_mode = 0;
  java_sync_mode = 0;
  is_hotspot_vm = 0;
  __collector_mutex_init (&jclasses_lock);
  tsd_key = COLLECTOR_TSD_INVALID_KEY;
  TprintfT (0, "jprofile: experiment closed.\n");
  return 0;
}

static int
detach_experiment (void)
/* fork child.  Clean up state but don't write to experiment */
{
  __collector_java_mode = 0;
  java_gc_on = 0;
  jvm = NULL;
  java_mem_mode = 0;
  java_sync_mode = 0;
  is_hotspot_vm = 0;
  jvmti = NULL;
  apistr = NULL;
  __collector_mutex_init (&jclasses_lock);
  tsd_key = COLLECTOR_TSD_INVALID_KEY;
  TprintfT (0, "jprofile: detached from experiment.\n");
  return 0;
}

JNIEXPORT jint JNICALL
JVM_OnLoad (JavaVM *vm, char *options, void *reserved)
{
  jvmtiError err;
  int use_jvmti = 0;
  if (!__collector_java_mode)
    {
      TprintfT (DBG_LT1, "jprofile: JVM_OnLoad invoked with java mode disabled\n");
      return JNI_OK;
    }
  else
    TprintfT (DBG_LT1, "jprofile: JVM_OnLoad invoked\n");
  jvm = vm;
  jvmti = NULL;
  if ((*jvm)->GetEnv (jvm, (void **) &jvmti, JVMTI_VERSION_1_0) >= 0 && jvmti)
    {
      TprintfT (DBG_LT1, "jprofile: JVMTI found\n");
      use_jvmti = 1;
    }
  if (!use_jvmti)
    {
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\"/>\n",
				     SP_JCMD_CERROR, COL_ERROR_JVMNOTSUPP);
      return JNI_ERR;
    }
  else
    {
      Tprintf (DBG_LT0, "\tjprofile: Initializing for JVMTI\n");
      apistr = "JVMTI 1.0";

      // setup JVMTI
      jvmtiCapabilities cpblts;
      err = (*jvmti)->GetPotentialCapabilities (jvmti, &cpblts);
      if (err == JVMTI_ERROR_NONE)
	{
	  jvmtiCapabilities cpblts_set;
	  CALL_UTIL (memset)(&cpblts_set, 0, sizeof (cpblts_set));

	  /* Add only those capabilities that are among potential ones */
	  cpblts_set.can_get_source_file_name = cpblts.can_get_source_file_name;
	  Tprintf (DBG_LT1, "\tjprofile: adding can_get_source_file_name capability: %u\n", cpblts.can_get_source_file_name);

	  cpblts_set.can_generate_compiled_method_load_events = cpblts.can_generate_compiled_method_load_events;
	  Tprintf (DBG_LT1, "\tjprofile: adding can_generate_compiled_method_load_events capability: %u\n", cpblts.can_generate_compiled_method_load_events);

	  if (java_sync_mode)
	    {
	      cpblts_set.can_generate_monitor_events = cpblts.can_generate_monitor_events;
	      Tprintf (DBG_LT1, "\tjprofile: adding can_generate_monitor_events capability: %u\n", cpblts.can_generate_monitor_events);
	    }
	  if (java_gc_on)
	    {
	      cpblts_set.can_generate_garbage_collection_events = cpblts.can_generate_garbage_collection_events;
	      Tprintf (DBG_LT1, "\tjprofile: adding can_generate_garbage_collection_events capability: %u\n", cpblts.can_generate_garbage_collection_events);
	    }
	  err = (*jvmti)->AddCapabilities (jvmti, &cpblts_set);
	  Tprintf (DBG_LT1, "\tjprofile: AddCapabilities() returns: %d\n", err);
	}
      err = (*jvmti)->SetEventCallbacks (jvmti, &callbacks, sizeof ( callbacks));
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, NULL);
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, NULL);
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, NULL);
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_CLASS_LOAD, NULL);
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_COMPILED_METHOD_LOAD, NULL);
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_COMPILED_METHOD_UNLOAD, NULL);
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_DYNAMIC_CODE_GENERATED, NULL);
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_THREAD_START, NULL);
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_THREAD_END, NULL);
      err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, NULL);
      if (java_gc_on)
	{
	  err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_START, NULL);
	  err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_GARBAGE_COLLECTION_FINISH, NULL);
	}
      if (java_mem_mode)
	{
	  // err = (*jvmti)->SetEventNotificationMode( jvmti, JVMTI_ENABLE, <no event for heap tracing> , NULL );
	  collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\"/>\n",
					 SP_JCMD_CWARN, COL_WARN_NO_JAVA_HEAP);
	  java_mem_mode = 0;
	}
      if (java_sync_mode)
	{
	  err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_MONITOR_CONTENDED_ENTER, NULL);
	  err = (*jvmti)->SetEventNotificationMode (jvmti, JVMTI_ENABLE, JVMTI_EVENT_MONITOR_CONTENDED_ENTERED, NULL);
	  //err = (*jvmti)->SetEventNotificationMode( jvmti, JVMTI_ENABLE, JVMTI_EVENT_MONITOR_WAIT, NULL );
	  //err = (*jvmti)->SetEventNotificationMode( jvmti, JVMTI_ENABLE, JVMTI_EVENT_MONITOR_WAITED, NULL );
	}
      Tprintf (DBG_LT0, "\tjprofile: JVMTI initialized\n");
    }

  /* JVM still uses collector API on Solaris to notify us about dynamically generated code.
   * If we ask it to generate events we'll end up with duplicate entries in the
   * map file.
   */
  if (use_jvmti)
    {
      err = (*jvmti)->GenerateEvents (jvmti, JVMTI_EVENT_DYNAMIC_CODE_GENERATED);
      err = (*jvmti)->GenerateEvents (jvmti, JVMTI_EVENT_COMPILED_METHOD_LOAD);
    }
  Tprintf (DBG_LT1, "\tjprofile: JVM_OnLoad ok\n");
  return JNI_OK;
}

/* This is currently just a placeholder */
JNIEXPORT jint JNICALL
Agent_OnLoad (JavaVM *vm, char *options, void *reserved)
{
  return JVM_OnLoad (vm, options, reserved);
}

static void
rwrite (int fd, const void *buf, size_t nbyte)
{
  size_t left = nbyte;
  size_t res;
  char *ptr = (char*) buf;
  while (left > 0)
    {
      res = CALL_UTIL (write)(fd, ptr, left);
      if (res == -1)
	{
	  /*  XXX: we can't write this record, we probably
	   *  can't write anything else. Ignore.
	   */
	  return;
	}
      left -= res;
      ptr += res;
    }
}

void
get_jvm_settings ()
{
  jint res;
  JNIEnv *jni;
  jclass jcls;
  jmethodID jmid;
  jstring jstrin;
  jstring jstrout;
  const char *str;
  res = (*jvm)->GetEnv (jvm, (void **) &jni, JNI_VERSION_1_2);
  if (res < 0)
    return;

  /* I'm not checking if results are valid as JVM is extremely
   * sensitive to exceptions that might occur during these JNI calls
   * and will die with a fatal error later anyway.
   */
  jcls = (*jni)->FindClass (jni, "java/lang/System");
  jmid = (*jni)->GetStaticMethodID (jni, jcls, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
  jstrin = (*jni)->NewStringUTF (jni, "java.class.path");
  jstrout = (*jni)->CallStaticObjectMethod (jni, jcls, jmid, jstrin);
  str = jstrout ? (*jni)->GetStringUTFChars (jni, jstrout, NULL) : NULL;
  if (str)
    {
      collector_interface->writeLog ("<setting %s=\"%s\"/>\n", SP_JCMD_SRCHPATH, str);
      (*jni)->ReleaseStringUTFChars (jni, jstrout, str);
    }
  jstrin = (*jni)->NewStringUTF (jni, "sun.boot.class.path");
  jstrout = (*jni)->CallStaticObjectMethod (jni, jcls, jmid, jstrin);
  str = jstrout ? (*jni)->GetStringUTFChars (jni, jstrout, NULL) : NULL;
  if (str)
    {
      collector_interface->writeLog ("<setting %s=\"%s\"/>\n", SP_JCMD_SRCHPATH, str);
      (*jni)->ReleaseStringUTFChars (jni, jstrout, str);
    }
  jstrin = (*jni)->NewStringUTF (jni, "java.home");
  jstrout = (*jni)->CallStaticObjectMethod (jni, jcls, jmid, jstrin);
  str = jstrout ? (*jni)->GetStringUTFChars (jni, jstrout, NULL) : NULL;
  if (str)
    {
      collector_interface->writeLog ("<setting %s=\"%s/../src.zip\"/>\n", SP_JCMD_SRCHPATH, str);
      (*jni)->ReleaseStringUTFChars (jni, jstrout, str);
    }
  jstrin = (*jni)->NewStringUTF (jni, "java.vm.version");
  jstrout = (*jni)->CallStaticObjectMethod (jni, jcls, jmid, jstrin);
  str = jstrout ? (*jni)->GetStringUTFChars (jni, jstrout, NULL) : NULL;
  if (str)
    {
      (void) collector_interface->writeLog ("<profile name=\"jprofile\" %s=\"%s\" %s=\"%s\"/>\n",
					    SP_JCMD_JVERSION, str, "api", apistr != NULL ? apistr : "N/A");
      if (__collector_strStartWith (str, "1.4.2_02") < 0)
	{
	  (void) collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\"/>\n",
						SP_JCMD_CWARN, COL_WARN_OLDJAVA);
	}
      (*jni)->ReleaseStringUTFChars (jni, jstrout, str);
    }
  is_hotspot_vm = 0;
  jstrin = (*jni)->NewStringUTF (jni, "sun.management.compiler");
  jstrout = (*jni)->CallStaticObjectMethod (jni, jcls, jmid, jstrin);
  str = jstrout ? (*jni)->GetStringUTFChars (jni, jstrout, NULL) : NULL;
  if (str && __collector_strncmp (str, "HotSpot", 7) == 0)
    is_hotspot_vm = 1;

  /* Emulate System.setProperty( "collector.init", "true") */
  jmid = (*jni)->GetStaticMethodID (jni, jcls, "setProperty",
				    "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
  jstrin = (*jni)->NewStringUTF (jni, "collector.init");
  jstrout = (*jni)->NewStringUTF (jni, "true");
  (*jni)->CallStaticObjectMethod (jni, jcls, jmid, jstrin, jstrout);
}

/*
 * JVMTI code
 */

static void
jvmti_VMInit (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread)
{
  jint class_count = 0;
  jclass *classes = NULL;
  int i;
  TprintfT (DBG_LT1, "jprofile: jvmti_VMInit called\n");
  get_jvm_settings ();

  /* determine loaded classes */
  (*jvmti_env)->GetLoadedClasses (jvmti_env, &class_count, &classes);
  TprintfT (DBG_LT1, "jprofile: jvmti_VMInit initializing %d classes\n", class_count);
  for (i = 0; i < class_count; i++)
    {
      // PushLocalFrame
      jvmti_ClassPrepare (jvmti_env, jni_env, NULL, classes[i]);
      // PopLocalFrame
      // DeleteLocalRef( classes[i] );
    }
  (*jvmti_env)->Deallocate (jvmti_env, (unsigned char*) classes);
  getResource = (*jni_env)->GetMethodID (jni_env, (*jni_env)->FindClass (jni_env, "java/lang/ClassLoader"), "getResource", "(Ljava/lang/String;)Ljava/net/URL;");
  toExternalForm = (*jni_env)->GetMethodID (jni_env, (*jni_env)->FindClass (jni_env, "java/net/URL"), "toExternalForm", "()Ljava/lang/String;");

  /* find the stack unwind routine */
  jprof_find_asyncgetcalltrace ();
}

static void
jvmti_VMDeath (jvmtiEnv *jvmti_env, JNIEnv* jni_env)
{
  __collector_java_mode = 0;
  TprintfT (DBG_LT1, "jprofile: jvmti_VMDeath event received\n");
}

static void
jvmti_ThreadStart (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread)
{
  jvmtiError err;
  jvmtiThreadInfo t_info;
  char *thread_name, *group_name, *parent_name;
  hrtime_t hrt;
  collector_thread_t tid;
  thread_name = group_name = parent_name = NULL;
  hrt = gethrtime ();
  tid = __collector_thr_self ();
  TprintfT (DBG_LT1, "jprofile: jvmti_ThreadStart: thread: %lu jni_env=%p jthread=%p\n",
	    (unsigned long) tid, jni_env, thread);
  err = (*jvmti_env)->GetThreadInfo (jvmti_env, thread, &t_info);
  if (err == JVMTI_ERROR_NONE)
    {
      jvmtiThreadGroupInfo g_info;
      thread_name = t_info.name;
      if (t_info.thread_group)
	{
	  err = (*jvmti_env)->GetThreadGroupInfo (jvmti_env, t_info.thread_group, &g_info);
	  if (err == JVMTI_ERROR_NONE)
	    {
	      group_name = g_info.name;
	      if (g_info.parent)
		{
		  jvmtiThreadGroupInfo p_info;
		  err = (*jvmti_env)->GetThreadGroupInfo (jvmti_env, g_info.parent, &p_info);
		  if (err == JVMTI_ERROR_NONE)
		    {
		      parent_name = p_info.name;
		      // DeleteLocalRef( p_info.parent );
		    }
		  // DeleteLocalRef( g_info.parent );
		}
	    }
	}
      // DeleteLocalRef( t_info.thread_group );
      // DeleteLocalRef( t_info.context_class_loader );
    }
  if (thread_name == NULL)
    thread_name = "";
  if (group_name == NULL)
    group_name = "";
  if (parent_name == NULL)
    parent_name = "";
  collector_interface->writeLog ("<event kind=\"%s\" tstamp=\"%u.%09u\" name=\"%s\" grpname=\"%s\" prntname=\"%s\" tid=\"%lu\" jthr=\"0x%lx\" jenv=\"0x%lx\"/>\n",
				 SP_JCMD_JTHRSTART,
				 (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC),
				 thread_name,
				 group_name,
				 parent_name,
				 (unsigned long) tid,
				 thread,
				 jni_env
				 );
  TSD_Entry *tsd = collector_interface->getKey (tsd_key);
  if (tsd)
    tsd->env = jni_env;
}

static void
jvmti_ThreadEnd (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread)
{
  hrtime_t hrt = gethrtime ();
  collector_thread_t tid = __collector_thr_self ();
  TprintfT (DBG_LT1, "jprofile: jvmti_ThreadEnd: thread: %lu jni_env=%p jthread=%p\n",
	    (unsigned long) tid, jni_env, thread);

  collector_interface->writeLog ("<event kind=\"%s\" tstamp=\"%u.%09u\" tid=\"%lu\"  jthr=\"0x%lx\" jenv=\"0x%lx\"/>\n",
				 SP_JCMD_JTHREND,
				 (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC),
				 (unsigned long) tid,
				 thread,
				 jni_env
				 );
  TSD_Entry *tsd = collector_interface->getKey (tsd_key);
  if (tsd)
    tsd->env = NULL;
}

/* The following definitions are borrowed from file jvmticmlr.h, part of jdk7 */
typedef enum
{
  JVMTI_CMLR_DUMMY = 1,
  JVMTI_CMLR_INLINE_INFO = 2
} jvmtiCMLRKind;

/*
 * Record that represents arbitrary information passed through JVMTI
 * CompiledMethodLoadEvent void pointer.
 */
typedef struct _jvmtiCompiledMethodLoadRecordHeader
{
  jvmtiCMLRKind kind;       /* id for the kind of info passed in the record */
  jint majorinfoversion;    /* major and minor info version values. Init'ed */
  jint minorinfoversion;    /* to current version value in jvmtiExport.cpp. */
  struct _jvmtiCompiledMethodLoadRecordHeader* next;
} jvmtiCompiledMethodLoadRecordHeader;

/*
 * Record that gives information about the methods on the compile-time
 * stack at a specific pc address of a compiled method. Each element in
 * the methods array maps to same element in the bcis array.
 */
typedef struct _PCStackInfo
{
  void* pc;                 /* the pc address for this compiled method */
  jint numstackframes;      /* number of methods on the stack */
  jmethodID* methods;       /* array of numstackframes method ids */
  jint* bcis;               /* array of numstackframes bytecode indices */
} PCStackInfo;

/*
 * Record that contains inlining information for each pc address of
 * an nmethod.
 */
typedef struct _jvmtiCompiledMethodLoadInlineRecord
{
  jvmtiCompiledMethodLoadRecordHeader header; /* common header for casting */
  jint numpcs; /* number of pc descriptors in this nmethod */
  PCStackInfo* pcinfo; /* array of numpcs pc descriptors */
} jvmtiCompiledMethodLoadInlineRecord;

static void
jvmti_CompiledMethodLoad (jvmtiEnv *jvmti_env, jmethodID method,
			  jint code_size, const void *code_addr, jint map_length,
			  const jvmtiAddrLocationMap *map,
			  const void *compile_info)
{
  TprintfT (DBG_LT2, "jprofile: jvmti_CompiledMethodLoad: mid=0x%lx addr=%p sz=0x%lu map=%p info=%p\n",
	    (unsigned long) method, code_addr, (long) code_size, map, compile_info);
  char name[32];
  CALL_UTIL (snprintf)(name, sizeof (name), "0x%lx", (unsigned long) method);

  /* Parse compile_info to get pc -> bci mapping.
   * Don't interpret compile_info from JVMs other than HotSpot.
   */
  int lntsize = 0;
  DT_lineno *lntable = NULL;
  if (compile_info != NULL && is_hotspot_vm)
    {
      Tprintf (DBG_LT2, "Mapping from compile_info:\n");
      jvmtiCompiledMethodLoadRecordHeader *currec =
	      (jvmtiCompiledMethodLoadRecordHeader*) compile_info;
      while (currec != NULL)
	{
	  if (currec->kind == JVMTI_CMLR_INLINE_INFO)
	    {
	      jvmtiCompiledMethodLoadInlineRecord *inrec =
		      (jvmtiCompiledMethodLoadInlineRecord*) currec;
	      if (inrec->numpcs <= 0)
		break;
	      lntsize = inrec->numpcs;
	      lntable = (DT_lineno*) alloca (lntsize * sizeof (DT_lineno));
	      PCStackInfo *pcrec = inrec->pcinfo;
	      DT_lineno *lnorec = lntable;
	      for (int i = 0; i < lntsize; ++i)
		{
		  for (int j = pcrec->numstackframes - 1; j >= 0; --j)
		    if (pcrec->methods[j] == method)
		      {
			lnorec->offset = (char*) pcrec->pc - (char*) code_addr;
			lnorec->lineno = pcrec->bcis[j];
			Tprintf (DBG_LT2, "   pc: 0x%lx  bci: 0x%lx\n",
				 (long) lnorec->offset, (long) lnorec->lineno);
			++lnorec;
			break;
		      }
		  ++pcrec;
		}
	      break;
	    }
	  currec = currec->next;
	}
    }
  else if (map != NULL)
    {
      Tprintf (DBG_LT2, "Mapping from jvmtiAddrLocationMap:\n");
      lntsize = map_length;
      lntable = (DT_lineno*) alloca (lntsize * sizeof (DT_lineno));
      DT_lineno *lnorec = lntable;
      for (int i = 0; i < map_length; ++i)
	{
	  lnorec->offset = (char*) map[i].start_address - (char*) code_addr;
	  lnorec->lineno = (unsigned int) map[i].location;
	  Tprintf (DBG_LT2, "   pc: 0x%lx  bci: 0x%lx\n",
		   (long) lnorec->offset, (long) lnorec->lineno);
	  ++lnorec;
	}
    }
  __collector_int_func_load (DFUNC_JAVA, name, NULL, (void*) code_addr,
			     code_size, lntsize, lntable);
}

static void
jvmti_CompiledMethodUnload (jvmtiEnv *jvmti_env, jmethodID method, const void* code_addr)
{
  __collector_int_func_unload (DFUNC_API, (void*) code_addr);
}

static void
jvmti_DynamicCodeGenerated (jvmtiEnv *jvmti_env, const char*name, const void *code_addr, jint code_size)
{
  __collector_int_func_load (DFUNC_API, (char*) name, NULL, (void*) code_addr,
			     code_size, 0, NULL);
}

static void
addToDynamicArchive (const char* name, const unsigned char* class_data, int class_data_len)
{
  char path[MAXPATHLEN + 1];
  mode_t fmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  mode_t dmode = fmode | S_IXUSR | S_IXGRP | S_IXOTH;
  if (name == NULL)
    name = "";
  const char *expdir = collector_interface->getExpDir ();
  if (CALL_UTIL (strlen)(expdir) +
      CALL_UTIL (strlen)(SP_DYNAMIC_CLASSES) +
      CALL_UTIL (strlen)(name) + 8 > sizeof (path))
    return;
  CALL_UTIL (snprintf)(path, sizeof (path), "%s/%s/%s.class", expdir, SP_DYNAMIC_CLASSES, name);

  /* Create all path components step by step starting with SP_DYNAMIC_CLASSES */
  char *str = path + CALL_UTIL (strlen)(expdir) + 1 + CALL_UTIL (strlen)(SP_DYNAMIC_CLASSES);
  while (str)
    {
      *str = '\0';
      if (CALL_UTIL (mkdir)(path, dmode) != 0)
	{
	  /* Checking for EEXIST is not enough, access() is more reliable */
	  if (CALL_UTIL (access)(path, F_OK) != 0)
	    {
	      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
					     SP_JCMD_CERROR, COL_ERROR_MKDIR, errno, path);
	      return;
	    }
	}
      *str++ = '/';
      str = CALL_UTIL (strchr)(str, '/');
    }

  int fd = CALL_UTIL (open)(path, O_WRONLY | O_CREAT | O_TRUNC, fmode);
  if (fd < 0)
    {
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_OVWOPEN, errno, path);
      return;
    }
  rwrite (fd, class_data, class_data_len);
  CALL_UTIL (close)(fd);
}

static void
jvmti_ClassFileLoadHook (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jclass class_being_redefined,
			 jobject loader, const char* name, jobject protection_domain, jint class_data_len,
			 const unsigned char* class_data, jint* new_class_data_len, unsigned char** new_class_data)
{
  jclass loaderlass;
  int err;
  jvmtiPhase phase_ptr;
  char *cname = NULL;
  (*jvmti_env)->GetPhase (jvmti_env, &phase_ptr);

  /* skip non live phases */
  if (phase_ptr != JVMTI_PHASE_LIVE)
    return;

  /* skip system class loaders */
  if (!loader)
    return;
  loaderlass = (*jni_env)->GetObjectClass (jni_env, loader);
  err = (*jvmti_env)->GetClassSignature (jvmti_env, loaderlass, &cname, NULL);
  if (err != JVMTI_ERROR_NONE || !cname || *cname == (char) 0)
    return;

  /* skip classes loaded with AppClassLoader (java.class.path) */
  if (__collector_strcmp (cname, "Lsun/misc/Launcher$AppClassLoader;") == 0)
    return;
  addToDynamicArchive (name, class_data, (int) class_data_len);
}

#define NO_CLASS_NAME "<noname>"
#define NO_SOURCE_FILE "<Unknown>"

static void
record_jclass (uint64_t class_id, hrtime_t hrt, const char *cname, const char *sname)
{
  size_t clen = ARCH_STRLEN (cname);
  size_t slen = ARCH_STRLEN (sname);
  size_t sz = sizeof (ARCH_jclass) + clen + slen;
  ARCH_jclass *jcls = (ARCH_jclass*) alloca (sz);
  jcls->comm.tsize = sz;
  jcls->comm.type = ARCH_JCLASS;
  jcls->class_id = class_id;
  jcls->tstamp = hrt;
  char *str = (char*) (jcls + 1);
  size_t i = CALL_UTIL (strlcpy)(str, cname, clen);
  str += i;
  while (i++ < clen)
    *str++ = (char) 0; /* pad with 0's */
  i = CALL_UTIL (strlcpy)(str, sname, slen);
  str += i;
  while (i++ < slen)
    *str++ = (char) 0; /* pad with 0's */
  collector_interface->writeDataPacket (jprof_hndl, (CM_Packet*) jcls);
}

static void
record_jmethod (uint64_t class_id, uint64_t method_id,
		const char *mname, const char *msign)
{
  size_t mnlen = mname ? ARCH_STRLEN (mname) : 0;
  size_t mslen = msign ? ARCH_STRLEN (msign) : 0;
  size_t sz = sizeof (ARCH_jmethod) + mnlen + mslen;
  ARCH_jmethod *jmth = (ARCH_jmethod*) alloca (sz);
  if (jmth == NULL)
    {
      TprintfT (DBG_LT1, "jprofile: record_jmethod ERROR: failed to alloca(%ld)\n", (long) sz);
      return;
    }
  jmth->comm.tsize = sz;
  jmth->comm.type = ARCH_JMETHOD;
  jmth->class_id = class_id;
  jmth->method_id = method_id;
  char *str = (char*) (jmth + 1);
  if (mname)
    {
      size_t i = CALL_UTIL (strlcpy)(str, mname, mnlen);
      str += i;
      while (i++ < mnlen)
	*str++ = (char) 0; /* pad with 0's */
    }
  if (msign)
    {
      size_t i = CALL_UTIL (strlcpy)(str, msign, mslen);
      str += i;
      while (i++ < mslen)
	*str++ = (char) 0; /* pad with 0's */
    }
  collector_interface->writeDataPacket (jprof_hndl, (CM_Packet*) jmth);
}

static void
jvmti_ClassPrepare (jvmtiEnv *jvmti_env, JNIEnv* jni_env,
		    jthread thread, jclass klass)
{
  hrtime_t hrt;
  jint mnum;
  jmethodID *mptr;
  char *cname, *sname;
  char *str1 = NULL;
  int err = (*jvmti_env)->GetClassSignature (jvmti_env, klass, &str1, NULL);
  if (err != JVMTI_ERROR_NONE || str1 == NULL || *str1 == (char) 0)
    cname = NO_CLASS_NAME;
  else
    cname = str1;
  if (*cname != 'L')
    {
      DprintfT (SP_DUMP_JAVA | SP_DUMP_TIME, "jvmti_ClassPrepare: GetClassSignature failed. err=%d cname=%s\n", err, cname);
      return;
    }
  char *str2 = NULL;
  err = (*jvmti_env)->GetSourceFileName (jvmti_env, klass, &str2);
  if (err != JVMTI_ERROR_NONE || str2 == NULL || *str2 == (char) 0)
    sname = NO_SOURCE_FILE;
  else
    sname = str2;
  DprintfT (SP_DUMP_JAVA | SP_DUMP_TIME, "jvmti_ClassPrepare: cname=%s sname=%s\n", STR (cname), STR (sname));

  /* Lock the whole file */
  __collector_mutex_lock (&jclasses_lock);
  hrt = gethrtime ();
  record_jclass ((unsigned long) klass, hrt, cname, sname);
  (*jvmti_env)->Deallocate (jvmti_env, (unsigned char *) str1);
  (*jvmti_env)->Deallocate (jvmti_env, (unsigned char *) str2);
  err = (*jvmti_env)->GetClassMethods (jvmti_env, klass, &mnum, &mptr);
  if (err == JVMTI_ERROR_NONE)
    {
      for (int i = 0; i < mnum; i++)
	{
	  char *mname, *msign;
	  err = (*jvmti_env)->GetMethodName (jvmti_env, mptr[i], &mname, &msign, NULL);
	  if (err != JVMTI_ERROR_NONE)
	    continue;
	  record_jmethod ((unsigned long) klass, (unsigned long) mptr[i], mname, msign);
	  // DeleteLocalRef( mptr[i] );
	}
      (*jvmti_env)->Deallocate (jvmti_env, (unsigned char*) mptr);
    }
  /* Unlock the file */
  __collector_mutex_unlock (&jclasses_lock);
}

/*
 * The CLASS_LOAD event is enabled to enable AsyncGetCallTrace
 */
static void
jvmti_ClassLoad (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jclass klass)
{
  char *cname;
  char *str1 = NULL;
  int err = (*jvmti_env)->GetClassSignature (jvmti_env, klass, &str1, NULL);
  if (err != JVMTI_ERROR_NONE || str1 == NULL || *str1 == (char) 0)
    cname = NO_CLASS_NAME;
  else
    cname = str1;
  jstring str = NULL;
  const char* resourceName;
  jobject classLoader = NULL;
  err = (*jvmti)->GetClassLoader (jvmti, klass, &classLoader);
  DprintfT (SP_DUMP_JAVA | SP_DUMP_TIME, "jprofile: jvmti_ClassLoad err=%d cname=%s\n", err, STR (cname));
  if (err == 0)
    {
      if (classLoader == NULL)
	{
	  // bootstrap class loader
	  resourceName = "";
	}
      else
	{
	  char* name = (char *) alloca ((CALL_UTIL (strlen)(str1) + 32) * sizeof (char));
	  CALL_UTIL (strlcpy)(name, str1 + 1, CALL_UTIL (strlen)(str1));
	  name[CALL_UTIL (strlen)(name) - 1] = '\0'; // remove the last ';'
	  char* p;
	  for (p = name; *p != '\0'; p++)
	    if (*p == '.')
	      *p = '/';
	  CALL_UTIL (strlcat)(name, ".class", CALL_UTIL (strlen)(name) + CALL_UTIL (strlen)(".class") + 1);
	  if (getResource == NULL || toExternalForm == NULL)
	    {
	      resourceName = "";
	      DprintfT (SP_DUMP_JAVA | SP_DUMP_TIME, "jvmti_ClassLoad: class %s failed to get path with method missing\n", STR (cname));
	    }
	  else
	    {
	      jobject url = (*jni_env)->CallObjectMethod (jni_env, classLoader, getResource, (*jni_env)->NewStringUTF (jni_env, name));
	      if (url == NULL)
		{
		  resourceName = "";
		  DprintfT (SP_DUMP_JAVA | SP_DUMP_TIME, "jvmti_ClassLoad: class %s failed to get path\n", STR (cname));
		}
	      else
		{
		  str = (jstring) (*jni_env)->CallObjectMethod (jni_env, url, toExternalForm);
		  resourceName = (*jni_env)->GetStringUTFChars (jni_env, str, NULL);
		  DprintfT (SP_DUMP_JAVA | SP_DUMP_TIME, "jvmti_ClassLoad: ARCH_JCLASS_LOCATION(Ox%x) class_id=0x%lx  className='%s' fileName '%s'\n",
			    (int) ARCH_JCLASS_LOCATION, (unsigned long) klass, STR (cname), STR (resourceName));
		  size_t clen = ARCH_STRLEN (cname);
		  size_t slen = ARCH_STRLEN (resourceName);
		  size_t sz = sizeof (ARCH_jclass) + clen + slen;
		  ARCH_jclass_location *jcls = (ARCH_jclass_location*) alloca (sz);
		  jcls->comm.tsize = sz;
		  jcls->comm.type = ARCH_JCLASS_LOCATION;
		  jcls->class_id = (unsigned long) klass;
		  char *str = (char*) (jcls + 1);
		  size_t i = CALL_UTIL (strlcpy)(str, cname, clen);
		  str += i;
		  while (i++ < clen)
		    {
		      *str++ = (char) 0; /* pad with 0's */
		    }
		  i = CALL_UTIL (strlcpy)(str, resourceName, slen);
		  str += i;
		  while (i++ < slen)
		    {
		      *str++ = (char) 0; /* pad with 0's */
		    }
		  /* Lock the whole file */
		  __collector_mutex_lock (&jclasses_lock);
		  collector_interface->writeDataPacket (jprof_hndl, (CM_Packet*) jcls);
		  /* Unlock the file */
		  __collector_mutex_unlock (&jclasses_lock);
		}
	    }
	}
    }
}

static void
jvmti_MonitorEnter (jvmtiEnv *jvmti_env, JNIEnv* jni_env,
		    jthread thread, jobject object)
{
  if (collector_jsync_begin)
    collector_jsync_begin ();
  TSD_Entry *tsd = collector_interface->getKey (tsd_key);
  if (tsd == NULL)
    return;
  tsd->tstamp = gethrtime ();
}

static void
jvmti_MonitorEntered (jvmtiEnv *jvmti_env, JNIEnv* jni_env,
		      jthread thread, jobject object)
{
  TSD_Entry *tsd = collector_interface->getKey (tsd_key);
  if (tsd == NULL)
    return;
  if (collector_jsync_end)
    collector_jsync_end (tsd->tstamp, object);
}

static void
jvmti_GarbageCollectionStart (jvmtiEnv *jvmti_env)
{
  hrtime_t hrt = gethrtime ();
  collector_interface->writeLog ("<event kind=\"%s\" tstamp=\"%u.%09u\"/>\n",
				 SP_JCMD_GCSTART,
				 (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC)
				 );
  TprintfT (DBG_LT1, "jprofile: jvmti_GarbageCollectionStart.\n");
}

static void
jvmti_GarbageCollectionFinish (jvmtiEnv *jvmti_env)
{
  hrtime_t hrt = gethrtime ();
  collector_interface->writeLog ("<event kind=\"%s\" tstamp=\"%u.%09u\"/>\n",
				 SP_JCMD_GCEND,
				 (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC)
				 );
  TprintfT (DBG_LT1, "jprofile: jvmti_GarbageCollectionFinish.\n");
}

#if 0
static void
jvmti_MonitorWait (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread,
		   jobject object, jlong timed_out)
{
  if (collector_sync_begin)
    collector_sync_begin ();
  TSD_Entry *tsd = collector_interface->getKey (tsd_key);
  if (tsd == NULL)
    return;
  tsd->tstamp = gethrtime ();
}

static void
jvmti_MonitorWaited (jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread,
		     jobject object, jboolean timed_out)
{
  TSD_Entry *tsd = collector_interface->getKey (tsd_key);
  if (tsd == NULL)
    return;
  if (collector_sync_end)
    collector_sync_end (tsd->tstamp, object);
}
#endif

static void
jprof_find_asyncgetcalltrace ()
{
  void *jvmhandle;
  if (__collector_VM_ReadByteInstruction == NULL)
    __collector_VM_ReadByteInstruction = (int(*)()) dlsym (RTLD_DEFAULT, "Async_VM_ReadByteInstruction");

  /* look for stack unwind function using default path */
  AsyncGetCallTrace = (void (*)(JVMPI_CallTrace*, jint, ucontext_t*))
	  dlsym (RTLD_DEFAULT, "AsyncGetCallTrace");
  if (AsyncGetCallTrace != NULL)
    {
      __collector_java_asyncgetcalltrace_loaded = 1;
      TprintfT (DBG_LT1, "jprofile: AsyncGetCallTrace found with RTLD_DEFAULT\n");
    }
  else
    {
      /* not found there, find libjvm.so, and ask again */
      jvmhandle = dlopen ("libjvm.so", RTLD_LAZY | RTLD_NOLOAD);
      if (jvmhandle != NULL)
	{
	  AsyncGetCallTrace = (void (*)(JVMPI_CallTrace*, jint, ucontext_t*))
		  dlsym (jvmhandle, "AsyncGetCallTrace");
	}
    }

  if (AsyncGetCallTrace == NULL)
    {
      /* we could not find it -- write collector error */
      TprintfT (0, "jprofile: ERROR -- AsyncGetCallTrace not found in address space\n");
      char *err = dlerror ();
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_JVMNOJSTACK, err ? err : "");
      __collector_java_mode = 0;
    }
  else
    {
      __collector_java_asyncgetcalltrace_loaded = 1;
      TprintfT (DBG_LT1, "jprofile: AsyncGetCallTrace initialized in jprof_jvmpi_init_done_event\n");
    }
}

int
__collector_ext_jstack_unwind (char *ptr, int sz, ucontext_t *uc)
{
  if (AsyncGetCallTrace == NULL)
    {
      TprintfT (DBG_LT0, "jprofile: __collector_ext_jstack_unwind: AsyncGetCallTrace is NULL\n");
      return 0;
    }

  TSD_Entry *tsd = collector_interface->getKey (tsd_key);
  if (tsd == NULL)
    {
      TprintfT (DBG_LT3, "jprofile: __collector_ext_jstack_unwind: tsd is NULL\n");
      return 0;
    }
  if (__collector_java_attach && tsd->env == NULL && jvmti != NULL && jvm != NULL)
    {
      TprintfT (DBG_LT3, "jprofile: __collector_ext_jstack_unwind: tsd->env is NULL under attach\n");
      JNIEnv* jni_env = NULL;
      (*jvm)->GetEnv (jvm, (void **) &jni_env, JNI_VERSION_1_2);
      tsd->env = jni_env;
    }
  if (tsd->env == NULL)
    {
      TprintfT (DBG_LT3, "jprofile: __collector_ext_jstack_unwind: tsd->env is NULL\n");
      return 0;
    }

  /* skip the Java stack whenever another signal handler is present */
  if (uc->uc_link)
    {
      TprintfT (DBG_LT3, "jprofile: __collector_ext_jstack_unwind: uc->uc_link is non-NULL\n");
      return 0;
    }
  /* we don't expect Java frames in signal handlers, so
   * unroll the list of saved contexts to the topmost one
   */
  while (uc->uc_link)
    uc = uc->uc_link;
  Java_info *jinfo = (Java_info*) ptr;
  jinfo->kind = JAVA_INFO;
  jinfo->hsize = sizeof (Java_info);
  ptr += sizeof (Java_info);
  sz -= sizeof (Java_info);

  JVMPI_CallTrace jtrace;
  jtrace.env_id = tsd->env;
  jtrace.frames = (JVMPI_CallFrame*) ptr;

  /* nframes is how many frames we have room for */
  jint nframes = sz / sizeof (JVMPI_CallFrame);

#if WSIZE(64)
  /* bug 6909545: garbage in 64-bit JAVA_INFO */
  CALL_UTIL (memset)(jtrace.frames, 0, nframes * sizeof (JVMPI_CallFrame));
#endif

#if ARCH(SPARC)
  // 21328946 JDK bug 8129933 causes <no java callstack recorded> on sparc-Linux
  // convert from ucontext_t to sigcontext
  struct sigcontext sctx;
  sctx.sigc_regs.tpc = uc->uc_mcontext.mc_gregs[MC_PC];
  __collector_memcpy (sctx.sigc_regs.u_regs, &uc->uc_mcontext.mc_gregs[3], sizeof (sctx.sigc_regs.u_regs));
  uc = (ucontext_t *) (&sctx);
#endif /* SPARC */
  AsyncGetCallTrace (&jtrace, nframes, uc);

  if (jtrace.num_frames == nframes)
    {
      JVMPI_CallFrame *last = &jtrace.frames[nframes - 1];
      last->method_id = (jmethodID) SP_TRUNC_STACK_MARKER;
      last->lineno = 0;
    }

  /* nframes is how many frames we actually got */
  nframes = jtrace.num_frames;
  TprintfT (DBG_LT3, "jprofile: __collector_ext_jstack_unwind: AsyncGetCallTrace jtrace.numframes = %d\n", nframes);
  if (nframes <= 0)
    {
      /* negative values are error codes */
      TprintfT (0, "jprofile: __collector_ext_jstack_unwind: AsyncGetCallTrace returned error: jtrace.numframes = %d\n", nframes);
      nframes = 1;
      JVMPI_CallFrame *err = (JVMPI_CallFrame*) ptr;
      err->lineno = jtrace.num_frames; // bci = error code
      err->method_id = 0; // artificial method id
    }
  jinfo->hsize += nframes * sizeof (JVMPI_CallFrame);
  return jinfo->hsize;
}

/*
 *	Collector Java API implementation
 */
void
Java_com_sun_forte_st_collector_CollectorAPI__1sample(JNIEnv *jEnv, jclass jCls, jstring jName)
{
  JNIEnv *jni;
  jint res = (*jvm)->GetEnv (jvm, (void **) &jni, JNI_VERSION_1_2);
  if (res < 0)
    return;
  const char *name = jName ? (*jni)->GetStringUTFChars (jni, jName, NULL) : NULL;
  __collector_sample ((char*) name);
}

void
Java_com_sun_forte_st_collector_CollectorAPI__1pause(JNIEnv *jEnv, jclass jCls)
{
  __collector_pause_m ("JAPI");
}

void
Java_com_sun_forte_st_collector_CollectorAPI__1resume(JNIEnv *jEnv, jclass jCls)
{
  __collector_resume ();
}

void
Java_com_sun_forte_st_collector_CollectorAPI__1terminate(JNIEnv *jEnv, jclass jCls)
{
  __collector_terminate_expt ();
}
#endif /* GPROFNG_JAVA_PROFILING */

static void init_module () __attribute__ ((constructor));
static void
init_module ()
{
#if defined(GPROFNG_JAVA_PROFILING)
  __collector_dlsym_guard = 1;
  RegModuleFunc reg_module = (RegModuleFunc) dlsym (RTLD_DEFAULT, "__collector_register_module");
  __collector_dlsym_guard = 0;
  if (reg_module)
    {
      jprof_hndl = reg_module (&module_interface);
      TprintfT (0, "jprofile: init_module.\n");
    }
#endif /* GPROFNG_JAVA_PROFILING */
}

int __collector_java_mode = 0;
int __collector_java_asyncgetcalltrace_loaded = 0;
