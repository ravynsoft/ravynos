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

/*
 *	Lineage events for process fork, exec, etc.
 */

#include "config.h"
#include <string.h>
#include <elf.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <spawn.h>

#include "descendants.h"

#define LT_MAXNAMELEN 1024
#define LT_MAXPATHLEN 1024

int __collector_linetrace_shutdown_hwcs_6830763_XXXX = 0;
int dbg_current_mode = FOLLOW_NONE; /* for debug only */
unsigned line_key = COLLECTOR_TSD_INVALID_KEY;
line_mode_t line_mode = LM_DORMANT;
int user_follow_mode = FOLLOW_ON;
int java_mode = 0;

static char *user_follow_spec;
static char new_lineage[LT_MAXNAMELEN];
static char curr_lineage[LT_MAXNAMELEN];
static char linetrace_exp_dir_name[LT_MAXPATHLEN + 1]; // experiment directory

/* lineage tracking for descendants of this process */

static int fork_linenum = 0;
static int line_initted = 0;
static collector_mutex_t fork_lineage_lock = COLLECTOR_MUTEX_INITIALIZER;
static collector_mutex_t clone_lineage_lock = COLLECTOR_MUTEX_INITIALIZER;

// For a given Linux, which lib functions have more than one GLIBC version? Do this:
// objdump -T `find /lib /lib64 -name "*.so*"` | grep GLIBC | grep text | grep \(
static pid_t (*__real_fork) (void) = NULL;
static pid_t (*__real_vfork) (void) = NULL;
static int (*__real_execve) (const char *file, char *const argv[],
			     char *const envp[]) = NULL;
static int (*__real_execvp) (const char *file, char *const argv[]) = NULL;
static int (*__real_execv) (const char *file, char *const argv[]) = NULL;
static int (*__real_execle) (const char *path, const char *arg, ...) = NULL;
static int (*__real_execlp) (const char *file, const char *arg, ...) = NULL;
static int (*__real_execl) (const char *file, const char *arg, ...) = NULL;
static int (*__real_clone) (int (*fn) (void *), void *child_stack,
			   int flags, void *arg, ...) = NULL;
static int (*__real_grantpt) (int fd) = NULL;
static char *(*__real_ptsname) (int fd) = NULL;
static FILE *(*__real_popen) (const char *command, const char *type) = NULL;
static int clone_linenum = 0;
static FILE *(*__real_popen_2_17) (const char *command, const char *type) = NULL;
static FILE *(*__real_popen_2_2_5) (const char *command, const char *type) = NULL;
static FILE *(*__real_popen_2_1) (const char *command, const char *type) = NULL;
static FILE *(*__real_popen_2_0) (const char *command, const char *type) = NULL;

static int (*__real_posix_spawn_2_17) (pid_t *pid, const char *path,
			       const posix_spawn_file_actions_t *file_actions,
			       const posix_spawnattr_t *attrp,
			       char *const argv[], char *const envp[]) = NULL;
static int (*__real_posix_spawn_2_15) (pid_t *pid, const char *path,
			       const posix_spawn_file_actions_t *file_actions,
			       const posix_spawnattr_t *attrp,
			       char *const argv[], char *const envp[]) = NULL;
static int (*__real_posix_spawn_2_2_5) (pid_t *pid, const char *path,
				const posix_spawn_file_actions_t *file_actions,
				const posix_spawnattr_t *attrp,
				char *const argv[], char *const envp[]) = NULL;
static int (*__real_posix_spawn_2_2) (pid_t *pid, const char *path,
			      const posix_spawn_file_actions_t *file_actions,
			      const posix_spawnattr_t *attrp,
			      char *const argv[], char *const envp[]) = NULL;

static int (*__real_posix_spawnp_2_17) (pid_t *pid, const char *file,
				const posix_spawn_file_actions_t *file_actions,
				const posix_spawnattr_t *attrp,
				char *const argv[], char *const envp[]) = NULL;
static int (*__real_posix_spawnp_2_15) (pid_t *pid, const char *file,
				const posix_spawn_file_actions_t *file_actions,
				const posix_spawnattr_t *attrp,
				char *const argv[], char *const envp[]) = NULL;
static int (*__real_posix_spawnp_2_2_5) (pid_t *pid, const char *file,
				 const posix_spawn_file_actions_t *file_actions,
				 const posix_spawnattr_t *attrp,
				 char *const argv[], char *const envp[]) = NULL;
static int (*__real_posix_spawnp_2_2) (pid_t *pid, const char *file,
			       const posix_spawn_file_actions_t *file_actions,
			       const posix_spawnattr_t *attrp,
			       char *const argv[], char *const envp[]) = NULL;
static int (*__real_system) (const char *command) = NULL;
static int (*__real_posix_spawn) (pid_t *pid, const char *path,
				const posix_spawn_file_actions_t *file_actions,
				const posix_spawnattr_t *attrp,
				char *const argv[], char *const envp[]) = NULL;
static int (*__real_posix_spawnp) (pid_t *pid, const char *file,
				const posix_spawn_file_actions_t *file_actions,
				const posix_spawnattr_t *attrp,
				char *const argv[], char *const envp[]) = NULL;
static int (*__real_setuid) (uid_t uid) = NULL;
static int (*__real_seteuid) (uid_t euid) = NULL;
static int (*__real_setreuid) (uid_t ruid, uid_t euid) = NULL;
static int (*__real_setgid) (gid_t gid) = NULL;
static int (*__real_setegid) (gid_t egid) = NULL;
static int (*__real_setregid) (gid_t rgid, gid_t egid)= NULL;
static void linetrace_dormant ();
static int check_follow_fork ();
static int check_follow_exec (const char *execfile);
static int check_follow_combo (const char *execfile);
static int path_collectable (const char *execfile);
static char * build_experiment_path (char *instring, size_t instring_sz, const char *lineage_str);
static int init_lineage_intf ();

/* -------  "Previously dbx-visible" function prototypes ----------------- */
static int linetrace_follow_experiment (const char *follow_spec, const char *lineage_str, const char *execfile);
static char *lineage_from_expname (char *lineage_str, size_t lstr_sz, const char *expname);
static void linetrace_ext_fork_prologue (const char *variant, char * new_lineage, int *following_fork);
static void linetrace_ext_fork_epilogue (const char *variant, const pid_t ret, char * new_lineage, int *following_fork);
static char **linetrace_ext_exec_prologue (const char *variant,
					   const char* path, char *const argv[], char *const envp[], int *following_exec);
static void linetrace_ext_exec_epilogue (const char *variant, char *const envp[], const int ret, int *following_exec);
static void linetrace_ext_combo_prologue (const char *variant, const char *cmd, int *following_combo);
static void linetrace_ext_combo_epilogue (const char *variant, const int ret, int *following_combo);

#ifdef DEBUG
static int
get_combo_flag ()
{
  int * guard = NULL;
  int combo_flag = ((line_mode == LM_TRACK_LINEAGE) ? ((CHCK_REENTRANCE (guard)) ? 1 : 0) : 0);
  return combo_flag;
}
#endif /* DEBUG */

/* must be called for potentially live experiment */
int
__collector_ext_line_init (int *precord_this_experiment,
			   const char * progspec, const char * progname)
{
  *precord_this_experiment = 1;
  TprintfT (DBG_LT0, "__collector_ext_line_init(%s)\n", progspec);
  if (NULL_PTR (fork))
    if (init_lineage_intf ())
      {
	TprintfT (DBG_LT0, "__collector_ext_line_init() ERROR: initialization failed.\n");
	return COL_ERROR_LINEINIT;
      }
  /* check the follow spec */
  user_follow_spec = CALL_UTIL (getenv)(SP_COLLECTOR_FOLLOW_SPEC);
  if (user_follow_spec != NULL)
    {
      TprintfT (DBG_LT0, "collector: %s=%s\n", SP_COLLECTOR_FOLLOW_SPEC, user_follow_spec);
      if (!linetrace_follow_experiment (user_follow_spec, curr_lineage, progname))
	{
	  *precord_this_experiment = 0;
	  TprintfT (DBG_LT0, "collector: -F =<regex> does not match, will NOT be followed\n");
	}
      else
	TprintfT (DBG_LT0, "collector: -F =<regex> matches, will be followed\n");
      user_follow_mode = FOLLOW_ALL;
    }
  __collector_env_save_preloads ();
  TprintfT (DBG_LT0, "__collector_ext_line_init(), progname=%s, followspec=%s, followthis=%d\n",
	    progname, user_follow_spec ? user_follow_spec : "NULL",
	    *precord_this_experiment);
  line_mode = LM_TRACK_LINEAGE; /* even if we don't follow, we report the interposition */
  line_initted = 1;
  return COL_ERROR_NONE;
}

/*
 * int __collector_ext_line_install(args)
 * Check args to determine which line events to follow.
 * Create tsd key for combo flag.
 */
int
__collector_ext_line_install (char *args, const char * expname)
{
  if (!line_initted)
    {
      TprintfT (DBG_LT0, "__collector_ext_line_install(%s) ERROR: init hasn't be called yet\n", args);
      return COL_ERROR_EXPOPEN;
    }
  TprintfT (DBG_LT0, "__collector_ext_line_install(%s, %s)\n", args, expname);
  line_key = __collector_tsd_create_key (sizeof (int), NULL, NULL);

  /* determine experiment name */
  __collector_strlcpy (linetrace_exp_dir_name, expname, sizeof (linetrace_exp_dir_name));
  lineage_from_expname (curr_lineage, sizeof (curr_lineage), linetrace_exp_dir_name);
  user_follow_mode = CALL_UTIL (atoi)(args);
  TprintfT (DBG_LT0, "__collector_ext_line_install() user_follow_mode=0x%X, linetrace_exp_dir_name=%s\n",
	    user_follow_mode, linetrace_exp_dir_name);

  // determine java mode
  char * java_follow_env = CALL_UTIL (getenv)(JAVA_TOOL_OPTIONS);
  if (java_follow_env != NULL && CALL_UTIL (strstr)(java_follow_env, COLLECTOR_JVMTI_OPTION))
    java_mode = 1;

  // backup collector specific env
  if (sp_env_backup == NULL)
    {
      sp_env_backup = __collector_env_backup ();
      TprintfT (DBG_LT0, "__collector_ext_line_install creating sp_env_backup -- 0x%p\n", sp_env_backup);
    }
  else
    TprintfT (DBG_LT0, "__collector_ext_line_install existing sp_env_backup -- 0x%p\n", sp_env_backup);
  if (user_follow_mode == FOLLOW_NONE)
    __collector_env_unset (NULL);

  char logmsg[256];
  logmsg[0] = '\0';
  if (user_follow_mode != FOLLOW_NONE)
    CALL_UTIL (strlcat)(logmsg, "fork|exec|combo", sizeof (logmsg));
  size_t slen = __collector_strlen (logmsg);
  if (slen > 0)
    logmsg[slen] = '\0';
  else
    CALL_UTIL (strlcat)(logmsg, "none", sizeof (logmsg));

  /* report which line events are followed */
  (void) __collector_log_write ("<setting %s=\"%s\"/>\n", SP_JCMD_LINETRACE, logmsg);
  TprintfT (DBG_LT0, "__collector_ext_line_install(%s): %s \n", expname, logmsg);
  return COL_ERROR_NONE;
}

char *
lineage_from_expname (char *lineage_str, size_t lstr_sz, const char *expname)
{
  TprintfT (DBG_LT0, "lineage_from_expname(%s, %s)\n", lineage_str, expname);
  char *p = NULL;
  if (lstr_sz < 1 || !lineage_str || !expname)
    {
      TprintfT (DBG_LT0, "lineage_from_expname(): ERROR, null string\n");
      return NULL;
    }
  /* determine lineage from experiment name */
  p = __collector_strrchr (expname, '/');
  if ((p == NULL) || (*++p != '_'))
    {
      lineage_str[0] = 0;
      TprintfT (DBG_LT2, "lineage_from_expname(): expt=%s lineage=\".\" (founder)\n", expname);
    }
  else
    {
      size_t tmp = __collector_strlcpy (lineage_str, p, lstr_sz);
      if (tmp >= lstr_sz)
	TprintfT (DBG_LT0, "lineage_from_expname(): ERROR: expt=%s lineage=\"%s\" truncated %ld characters\n",
		  expname, lineage_str, (long) (tmp - lstr_sz));
      lineage_str[lstr_sz - 1] = 0;
      p = __collector_strchr (lineage_str, '.');
      if (p != NULL)
	*p = '\0';
      TprintfT (DBG_LT2, "lineage_from_expname(): expt=%s lineage=\"%s\"\n", expname, lineage_str);
    }
  return lineage_str;
}

/*
 * void __collector_line_cleanup (void)
 * Disable logging. Clear backup ENV.
 */
void
__collector_line_cleanup (void)
{
  if (line_mode == LM_CLOSED)
    {
      TprintfT (DBG_LT0, "__collector_line_cleanup(): WARNING, line is already closed\n");
      return;
    }
  else if (line_mode == LM_DORMANT)
    TprintfT (DBG_LT0, "__collector_line_cleanup(): ERROR, line is DORMANT\n");
  else
    TprintfT (DBG_LT0, "__collector_line_cleanup()\n");
  line_mode = LM_CLOSED;
  user_follow_mode = FOLLOW_NONE;
  dbg_current_mode = FOLLOW_NONE; /* for debug only */
  line_key = COLLECTOR_TSD_INVALID_KEY;
  java_mode = 0;
  if (sp_env_backup != NULL)
    {
      __collector_env_backup_free ();
      sp_env_backup = NULL;
    }
  return;
}

/*
 * void __collector_ext_line_close (void)
 *	Disable logging.  Cleans ENV vars. Clear backup ENV.
 */
void
__collector_ext_line_close (void)
{
  TprintfT (DBG_LT0, "__collector_ext_line_close()\n");
  __collector_line_cleanup ();
  __collector_env_unset (NULL);
  return;
}

/*
 * void linetrace_dormant(void)
 *	Disable logging.  Preserve ENV vars.
 */
static void
linetrace_dormant (void)
{
  if (line_mode == LM_DORMANT)
    {
      TprintfT (DBG_LT0, "linetrace_dormant() -- already dormant\n");
      return;
    }
  else if (line_mode == LM_CLOSED)
    {
      TprintfT (DBG_LT0, "linetrace_dormant(): ERROR, line is already CLOSED\n");
      return;
    }
  else
    TprintfT (DBG_LT0, "linetrace_dormant()\n");
  line_mode = LM_DORMANT;
  return;
}

static int
check_follow_fork ()
{
  int follow = (user_follow_mode != FOLLOW_NONE);
  TprintfT (DBG_LT0, "check_follow_fork()=%d\n", follow);
  return follow;
}

static int
check_follow_exec (const char *execfile)
{
  int follow = (user_follow_mode != FOLLOW_NONE);
  if (follow)
    {
      /* revise based on collectability of execfile */
      follow = path_collectable (execfile);
    }
  TprintfT (DBG_LT0, "check_follow_exec(%s)=%d\n", execfile, follow);
  return follow;
}

static int
check_follow_combo (const char *execfile)
{
  int follow = (user_follow_mode != FOLLOW_NONE);
  TprintfT (DBG_LT0, "check_follow_combo(%s)=%d\n", execfile, follow);
  return follow;
}

static int
check_fd_dynamic (int fd)
{
  TprintfT (DBG_LT0, "check_fd_dynamic(%d)\n", fd);
  off_t off = CALL_UTIL (lseek)(fd, (off_t) 0, SEEK_END);
  size_t sz = (size_t) 8192; /* one page should suffice */
  if (sz > off)
    sz = off;
  char *p = CALL_UTIL (mmap64_)((char *) 0, sz, PROT_READ, MAP_PRIVATE, fd, (off64_t) 0);
  if (p == MAP_FAILED)
    {
      TprintfT (DBG_LT0, "check_fd_dynamic(): ERROR/WARNING: mmap failed for `%d'\n", fd);
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CWARN,
				    COL_WARN_NOFOLLOW, "mmap-failed");
      return 0;
    }
  char elfclass = p[EI_CLASS];
  if ((p[EI_MAG0] != ELFMAG0) ||
      (p[EI_MAG1] != ELFMAG1) ||
      (p[EI_MAG2] != ELFMAG2) ||
      (p[EI_MAG3] != ELFMAG3) ||
      (elfclass != ELFCLASS32 && elfclass != ELFCLASS64)
      )
    {
      TprintfT (DBG_LT0, "check_fd_dynamic(): WARNING: Command `%d' is not executable ELF!\n", fd);
      CALL_UTIL (munmap)(p, sz);
      return 1;
    }
  Elf32_Ehdr *ehdr32 = (Elf32_Ehdr*) p;
  Elf64_Ehdr *ehdr64 = (Elf64_Ehdr*) p;
  Elf64_Off e_phoff;
  Elf64_Half e_phnum;
  Elf64_Half e_phentsize;
  if (elfclass == ELFCLASS32)
    {
      e_phoff = ehdr32->e_phoff;
      e_phnum = ehdr32->e_phnum;
      e_phentsize = ehdr32->e_phentsize;
    }
  else
    {
      e_phoff = ehdr64->e_phoff;
      e_phnum = ehdr64->e_phnum;
      e_phentsize = ehdr64->e_phentsize;
    }
  if ((sizeof (Elf32_Ehdr) > sz) ||
      (sizeof (Elf64_Ehdr) > sz) ||
      (e_phoff + e_phentsize * (e_phnum - 1) > sz))
    {
      TprintfT (DBG_LT0, "check_fd_dynamic(): WARNING: Command `%d' ELF file did not fit in page!\n", fd);
#if 0
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CWARN,
				    COL_WARN_RISKYFOLLOW, "ELF header size");
#endif
      CALL_UTIL (munmap)(p, sz);
      return 1;
    }
  TprintfT (DBG_LT2, "check_fd_dynamic(): elfclass=%d, e_phoff=%lu e_phnum=%lu e_phentsize=%lu\n",
	    (int) elfclass, (unsigned long) e_phoff, (unsigned long) e_phnum,
	    (unsigned long) e_phentsize);
  int dynamic = 0;
  Elf64_Half i;
  for (i = 0; i < e_phnum; i++)
    {
      if (elfclass == ELFCLASS32)
	{
	  if (PT_DYNAMIC ==
	      ((Elf32_Phdr*) (p + e_phoff + e_phentsize * i))->p_type)
	    {
	      dynamic = 1;
	      break;
	    }
	}
      else
	{
	  if (PT_DYNAMIC ==
	      ((Elf64_Phdr*) (p + e_phoff + e_phentsize * i))->p_type)
	    {
	      dynamic = 1;
	      break;
	    }
	}
    }
  if (!dynamic)
    {
      TprintfT (DBG_LT0, "check_fd_dynamic(): ERROR/WARNING: Command `%d' is not a dynamic executable!\n", fd);
#if 0
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CWARN,
				    COL_WARN_NOFOLLOW, "!dynamic");
#endif
    }
  else
    TprintfT (DBG_LT2, "check_fd_dynamic(): Command `%d' is a dynamic executable!\n", fd);
  CALL_UTIL (munmap)(p, sz);
  return dynamic;
}

static int
check_dynamic (const char *execfile)
{
  TprintfT (DBG_LT2, "check_dynamic(%s)\n", execfile);
  int fd = CALL_UTIL (open)(execfile, O_RDONLY);
  if (fd == -1)
    {
      TprintfT (DBG_LT0, "check_dynamic(): ERROR/WARNING: Command `%s' could not be opened!\n", execfile);
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CWARN,
				    COL_WARN_RISKYFOLLOW, "open");
      return 1; /* follow, though exec will presumably fail */
    }
  int ret = check_fd_dynamic (fd);
  CALL_UTIL (close)(fd);
  return ret;
}

static int
path_collectable (const char *execfile)
{
  TprintfT (DBG_LT0, "path_collectable(%s)\n", execfile);
  /* Check that execfile exists and is a collectable executable */
  /* logging warning when collection is likely to be unsuccessful */
  /* (if check isn't accurate, generally best not to include it) */

  if (execfile && !__collector_strchr (execfile, '/'))
    { /* got an unqualified name */
      /* XXXX locate execfile on PATH to be able to check it */
      TprintfT (DBG_LT0, "path_collectable(): WARNING: Can't check unqualified executable `%s'\n", execfile);
#if 0
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CWARN,
				    COL_WARN_RISKYFOLLOW, "path");
#endif
      return 1; /* follow unqualified execfile unchecked */
    }
  struct stat sbuf;
  if (stat (execfile, &sbuf))
    { /* can't stat it */
      TprintfT (DBG_LT0, "path_collectable(): WARNING: Can't stat `%s'\n", execfile);
#if 0
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CWARN,
				    COL_WARN_RISKYFOLLOW, "stat");
#endif
      return 1; /* follow, though exec will probably fail */
    }
  TprintfT (DBG_LT2, "path_collectable(%s) mode=0%o uid=%d gid=%d\n",
	    execfile, sbuf.st_mode, sbuf.st_uid, sbuf.st_gid);
  if (((sbuf.st_mode & S_IXUSR) == 0) || ((sbuf.st_mode & S_IFMT) == S_IFDIR))
    {
      TprintfT (DBG_LT0, "path_collectable(): WARNING: Command `%s' is NOT an executable file!\n", execfile);
#if 0
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CWARN,
				    COL_WARN_RISKYFOLLOW, "mode");
#endif
      return 1; /* follow, though exec will presumably fail */
    }
  /* XXXX setxid(root) is OK iff libcollector is registered as secure */
  /* XXXX setxid(non-root) is OK iff umask is accomodating */
  if (((sbuf.st_mode & S_ISUID) != 0) || ((sbuf.st_mode & S_ISGID) != 0))
    {
      TprintfT (DBG_LT0, "path_collectable(): WARNING: Command `%s' is SetXID!\n", execfile);
#if 0
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CWARN,
				    COL_WARN_RISKYFOLLOW, "setxid");
#endif
      return 1; /* follow, though collection may be unreliable */
    }
  if (!check_dynamic (execfile))
    {
      TprintfT (DBG_LT0, "path_collectable(%s) WARNING/ERROR: not dynamic, not collectng!\n", execfile);
      return 0; /* don't follow, collection will fail unpredictably */
    }
  TprintfT (DBG_LT2, "path_collectable(%s) OK!\n", execfile);
  return 1; /* OK to follow */
}

static char *
build_experiment_path (char * instring, size_t instring_sz, const char *lineage_str)
{
  TprintfT (DBG_LT0, "build_experiment_path(,%ld, %s)\n",
	    (long) instring_sz, lineage_str);
  const char *p = CALL_UTIL (strstr)(linetrace_exp_dir_name, DESCENDANT_EXPT_KEY);
  int basedir_sz;
  if (p)
    basedir_sz = p - linetrace_exp_dir_name + 4; /* +3 because of DESCENDANT_EXPT_KEY */
  else
    basedir_sz = __collector_strlen (linetrace_exp_dir_name) + 1;
  int additional_sz = __collector_strlen (lineage_str) + 4;
  if (basedir_sz + additional_sz > instring_sz)
    {
      TprintfT (DBG_LT0, "build_experiment_path(%s,%s): ERROR: path too long: %d > %ld\n",
		linetrace_exp_dir_name, lineage_str,
		basedir_sz + additional_sz, (long) instring_sz);
      *instring = 0;
      return NULL;
    }
  __collector_strlcpy (instring, linetrace_exp_dir_name, basedir_sz);
  size_t slen = __collector_strlen (instring);
  CALL_UTIL (snprintf)(instring + slen, instring_sz - slen, "/%s.er", lineage_str);
  assert (__collector_strlen (instring) + 1 == basedir_sz + additional_sz);
  return instring;
}

static void
check_reuid_change (uid_t ruid, uid_t euid)
{
  uid_t curr_ruid = getuid ();
  uid_t curr_euid = geteuid ();
  mode_t curr_umask = umask (0);
  umask (curr_umask); /* restore original umask */
  int W_oth = !(curr_umask & S_IWOTH);
  TprintfT (DBG_LT0, "check_reuid_change(%d,%d): umask=%03o\n", ruid, euid, curr_umask);
  TprintfT (DBG_LT0, "check_reuid_change(): umask W usr=%d grp=%d oth=%d\n",
	    (int) (!(curr_umask & S_IWUSR)), (int) (!(curr_umask & S_IWGRP)), W_oth);
  if (ruid != -1)
    {
      TprintfT (DBG_LT0, "check_reuid_change(%d->%d)\n", curr_ruid, ruid);
      if ((curr_euid == 0) && (ruid != 0) && !W_oth)
	{
	  /* changing to non-root ID, with umask blocking writes by other */
	  TprintfT (DBG_LT0, "check_reuid_change(): ERROR/WARNING: umask blocks write other after ruid change (%d->%d)\n",
		    curr_ruid, ruid);
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">umask %03o ruid %d->%d</event>\n",
					SP_JCMD_CWARN, COL_WARN_IDCHNG, curr_umask, curr_ruid, ruid);
	}
    }
  if (euid != -1)
    {
      TprintfT (DBG_LT0, "check_reuid_change(%d->%d)\n", curr_euid, euid);
      if ((curr_euid == 0) && (euid != 0) && !W_oth)
	{
	  /* changing to non-root ID, with umask blocking writes by other */
	  TprintfT (DBG_LT0, "check_reuid_change(): ERROR/WARNING: umask blocks write other after euid change (%d->%d)\n",
		    curr_euid, euid);
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">umask %03o euid %d->%d</event>\n",
					SP_JCMD_CWARN, COL_WARN_IDCHNG, curr_umask, curr_euid, euid);
	}
    }
}

static void
check_regid_change (gid_t rgid, gid_t egid)
{
  gid_t curr_rgid = getgid ();
  gid_t curr_egid = getegid ();
  uid_t curr_euid = geteuid ();
  mode_t curr_umask = umask (0);
  umask (curr_umask); /* restore original umask */
  int W_oth = !(curr_umask & S_IWOTH);
  TprintfT (DBG_LT0, "check_regid_change(%d,%d): umask=%03o euid=%d\n",
	    rgid, egid, curr_umask, curr_euid);
  TprintfT (DBG_LT0, "umask W usr=%d grp=%d oth=%d\n",
	    (int) (!(curr_umask & S_IWUSR)), (int) (!(curr_umask & S_IWGRP)), W_oth);
  if (rgid != -1)
    {
      TprintfT (DBG_LT0, "check_regid_change(%d->%d)\n", curr_rgid, rgid);
      if ((curr_euid == 0) && (rgid != 0) && !W_oth)
	{
	  /* changing to non-root ID, with umask blocking writes by other */
	  TprintfT (DBG_LT0, "check_regid_change(): WARNING/ERROR: umask blocks write other after rgid change (%d->%d)\n",
		    curr_rgid, rgid);
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">umask %03o rgid %d->%d</event>\n",
					SP_JCMD_CWARN, COL_WARN_IDCHNG, curr_umask, curr_rgid, rgid);
	}
    }
  if (egid != -1)
    {
      TprintfT (DBG_LT0, "check_regid_change(): check_egid_change(%d->%d)\n", curr_egid, egid);
      if ((curr_euid == 0) && (egid != 0) && !W_oth)
	{
	  /* changing to non-root ID, with umask blocking writes by other */
	  TprintfT (DBG_LT0, "check_regid_change(): WARNING/ERROR: umask blocks write other after egid change (%d->%d)\n",
		    curr_egid, egid);
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">umask %03o egid %d->%d</event>\n",
					SP_JCMD_CWARN, COL_WARN_IDCHNG, curr_umask, curr_egid, egid);
	}
    }
}

static int
init_lineage_intf ()
{
  void *dlflag;
  TprintfT (DBG_LT2, "init_lineage_intf()\n");

  static int nesting_check = 0;
  if (nesting_check >= 2)
    {
      /* segv before stack blows up */
      nesting_check /= (nesting_check - 2);
    }
  nesting_check++;

  __real_fork = dlsym (RTLD_NEXT, "fork");
  if (__real_fork == NULL)
    {
      __real_fork = dlsym (RTLD_DEFAULT, "fork");
      if (__real_fork == NULL)
	return 1;
      dlflag = RTLD_DEFAULT;
    }
  else
    dlflag = RTLD_NEXT;
  TprintfT (DBG_LT2, "init_lineage_intf() using RTLD_%s\n",
	    dlflag == RTLD_DEFAULT ? "DEFAULT" : "NEXT");
  __real_vfork = dlsym (dlflag, "vfork");
  __real_execve = dlsym (dlflag, "execve");
  __real_execvp = dlsym (dlflag, "execvp");
  __real_execv = dlsym (dlflag, "execv");
  __real_execle = dlsym (dlflag, "execle");
  __real_execlp = dlsym (dlflag, "execlp");
  __real_execl = dlsym (dlflag, "execl");
  __real_clone = dlsym (dlflag, "clone");

  __real_popen_2_17 = dlvsym (dlflag, "popen", "GLIBC_2.17");
  __real_popen_2_2_5 = dlvsym (dlflag, "popen", "GLIBC_2.2.5");
  __real_popen_2_1 = dlvsym (dlflag, "popen", "GLIBC_2.1");
  __real_popen_2_0 = dlvsym (dlflag, "popen", "GLIBC_2.0");
  if (__real_popen_2_17)
    __real_popen = __real_popen_2_17;
  else if (__real_popen_2_2_5)
    __real_popen = __real_popen_2_2_5;
  else if (__real_popen_2_1)
    __real_popen = __real_popen_2_1;
  else if (__real_popen_2_0)
    __real_popen = __real_popen_2_0;
  else
    __real_popen = dlsym (dlflag, "popen");

  __real_posix_spawn_2_17 = dlvsym (dlflag, "posix_spawn", "GLIBC_2.17");
  __real_posix_spawn_2_15 = dlvsym (dlflag, "posix_spawn", "GLIBC_2.15");
  __real_posix_spawn_2_2_5 = dlvsym (dlflag, "posix_spawn", "GLIBC_2.2.5");
  __real_posix_spawn_2_2 = dlvsym (dlflag, "posix_spawn", "GLIBC_2.2");
  if (__real_posix_spawn_2_17)
    __real_posix_spawn = __real_posix_spawn_2_17;
  else if (__real_posix_spawn_2_15)
    __real_posix_spawn = __real_posix_spawn_2_15;
  else if (__real_posix_spawn_2_2_5)
    __real_posix_spawn = __real_posix_spawn_2_2_5;
  else if (__real_posix_spawn_2_2)
    __real_posix_spawn = __real_posix_spawn_2_2;
  else
    __real_posix_spawn = dlsym (dlflag, "posix_spawn");

  __real_posix_spawnp_2_17 = dlvsym (dlflag, "posix_spawnp", "GLIBC_2.17");
  __real_posix_spawnp_2_15 = dlvsym (dlflag, "posix_spawnp", "GLIBC_2.15");
  __real_posix_spawnp_2_2_5 = dlvsym (dlflag, "posix_spawnp", "GLIBC_2.2.5");
  __real_posix_spawnp_2_2 = dlvsym (dlflag, "posix_spawnp", "GLIBC_2.2");
  if (__real_posix_spawnp_2_17)
    __real_posix_spawnp = __real_posix_spawnp_2_17;
  else if (__real_posix_spawnp_2_15)
    __real_posix_spawnp = __real_posix_spawnp_2_15;
  else if (__real_posix_spawnp_2_2_5)
    __real_posix_spawnp = __real_posix_spawnp_2_2_5;
  else if (__real_posix_spawnp_2_2)
    __real_posix_spawnp = __real_posix_spawnp_2_2;
  else
    __real_posix_spawnp = dlsym (dlflag, "posix_spawnp");

  __real_grantpt = dlsym (dlflag, "grantpt");
  __real_ptsname = dlsym (dlflag, "ptsname");
  __real_system = dlsym (dlflag, "system");
  __real_setuid = dlsym (dlflag, "setuid");
  __real_seteuid = dlsym (dlflag, "seteuid");
  __real_setreuid = dlsym (dlflag, "setreuid");
  __real_setgid = dlsym (dlflag, "setgid");
  __real_setegid = dlsym (dlflag, "setegid");
  __real_setregid = dlsym (dlflag, "setregid");

#define PR_FUNC(f)  TprintfT (DBG_LT2, "linetrace.c: " #f ": @%p\n", f)
  PR_FUNC (__real_fork);
  PR_FUNC (__real_vfork);
  PR_FUNC (__real_execve);
  PR_FUNC (__real_execvp);
  PR_FUNC (__real_execv);
  PR_FUNC (__real_execle);
  PR_FUNC (__real_execlp);
  PR_FUNC (__real_execl);
  PR_FUNC (__real_clone);
  PR_FUNC (__real_grantpt);
  PR_FUNC (__real_ptsname);
  PR_FUNC (__real_popen);
  PR_FUNC (__real_popen_2_17);
  PR_FUNC (__real_popen_2_2_5);
  PR_FUNC (__real_popen_2_1);
  PR_FUNC (__real_popen_2_0);
  PR_FUNC (__real_posix_spawn_2_17);
  PR_FUNC (__real_posix_spawn_2_15);
  PR_FUNC (__real_posix_spawn_2_2_5);
  PR_FUNC (__real_posix_spawn_2_2);
  PR_FUNC (__real_posix_spawnp_2_17);
  PR_FUNC (__real_posix_spawnp_2_15);
  PR_FUNC (__real_posix_spawnp_2_2_5);
  PR_FUNC (__real_posix_spawnp_2_2);
  PR_FUNC (__real_system);
  PR_FUNC (__real_posix_spawn);
  PR_FUNC (__real_posix_spawnp);
  PR_FUNC (__real_setuid);
  PR_FUNC (__real_seteuid);
  PR_FUNC (__real_setreuid);
  PR_FUNC (__real_setgid);
  PR_FUNC (__real_setegid);
  PR_FUNC (__real_setregid);

  return 0;
}

/*------------------------------------------------------------------------ */
/* Note: The following _prologue and _epilogue functions used to be dbx-visible.

   They are used to appropriately manage lineage-changing events, by
   quiescing and re-enabling/re-setting experiment collection before and after,
   and logging the lineage-change in the process/experiment undertaking it.
   As shown by the interposition functions for fork, exec, etc., which follow,
   the _prologue should be called immediately prior (such as a breakpoint
   action defined at function entry) and the _epilogue called immediately
   after (such as a breakpoint action defined at function return).
 */

/*
   Notes on MT from Solaris 10 man pthread_atfork:

     All multithreaded applications that call fork() in  a  POSIX
     threads  program and do more than simply call exec(2) in the
     child of the fork need to ensure that the child is protected
     from deadlock.

     Since the "fork-one" model results in duplicating  only  the
     thread  that  called fork(), it is possible that at the time
     of the call another thread in the parent owns a  lock.  This
     thread  is  not  duplicated  in the child, so no thread will
     unlock this lock in the child.  Deadlock occurs if the  sin-
     gle thread in the child needs this lock.

     The problem is more serious with locks in libraries.   Since
     a  library writer does not know if the application using the
     library calls fork(), the library must protect  itself  from
     such  a  deadlock  scenario.   If the application that links
     with this library calls fork() and does not call  exec()  in
     the  child,  and if it needs a library lock that may be held
     by some other thread  in  the  parent  that  is  inside  the
     library  at  the time of the fork, the application deadlocks
     inside the library.
 */

static void
linetrace_ext_fork_prologue (const char *variant, char * n_lineage, int *following_fork)
{
  TprintfT (DBG_LT0, "linetrace_ext_fork_prologue; variant=%s; new_lineage=%s; follow=%d\n",
	    variant, n_lineage, *following_fork);
  __collector_env_print ("fork_prologue start");
  if (dbg_current_mode != FOLLOW_NONE)
    TprintfT (DBG_LT0, "linetrace_ext_fork_prologue(%s) ERROR: dbg_current_mode=%d, changing to FOLLOW_FORK!\n",
		variant, dbg_current_mode);
  dbg_current_mode = FOLLOW_ON;
  if (__collector_strncmp ((char *) variant, "clone", sizeof ("clone") - 1) == 0)
    {
      __collector_mutex_lock (&clone_lineage_lock);
      CALL_UTIL (snprintf)(n_lineage, LT_MAXNAMELEN, "%s_C%d", curr_lineage, ++clone_linenum);
      __collector_mutex_unlock (&clone_lineage_lock);
    }
  else
    {
      __collector_mutex_lock (&fork_lineage_lock);
      CALL_UTIL (snprintf)(n_lineage, LT_MAXNAMELEN, "%s_f%d", curr_lineage, ++fork_linenum);
      __collector_mutex_unlock (&fork_lineage_lock);
    }
  *following_fork = check_follow_fork ();

  /* write message before suspending, or it won't be written */
  hrtime_t ts = GETRELTIME ();
  TprintfT (DBG_LT0, "linetrace_ext_fork_prologue; variant=%s; new_lineage=%s; follow=%d\n",
	    variant, n_lineage, *following_fork);
  __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" variant=\"%s\" lineage=\"%s\" follow=\"%d\"/>\n",
			 SP_JCMD_DESC_START,
			 (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC),
			 variant, n_lineage, *following_fork);
  __collector_ext_dispatcher_thread_timer_suspend ();
  __collector_ext_hwc_lwp_suspend ();
  __collector_env_print ("fork_prologue end");
}

static void
linetrace_ext_fork_epilogue (const char *variant, const pid_t ret, char * n_lineage, int *following_fork)
{
  if (dbg_current_mode == FOLLOW_NONE)
    TprintfT (DBG_LT0, "linetrace_ext_fork_epilogue(%s) ERROR: dbg_current_mode=%d!\n",
	      variant, dbg_current_mode);
  /* compute descendant experiment name */
  char new_exp_name[LT_MAXPATHLEN];
  /* save exp_name to global var */
  if (!build_experiment_path (new_exp_name, sizeof (new_exp_name), n_lineage))
    TprintfT (DBG_LT0, "linetrace_ext_fork_epilogue(%s): ERROR SP_COLLECTOR_EXPNAME not set\n", n_lineage);
  TprintfT (DBG_LT0, "linetrace_ext_fork_epilogue(%s):%d() returned %d %s; child experiment name = %s\n",
	    variant, *following_fork, ret, (ret ? "parent" : "child"), new_exp_name);
  if (ret == 0)
    {
      /* *************************************child */
      __collector_env_print ("fork_epilogue child at start");
      /* start a new line */
      fork_linenum = 0;
      __collector_mutex_init (&fork_lineage_lock);
      clone_linenum = 0;
      __collector_mutex_init (&clone_lineage_lock);
      __collector_env_update (NULL);
      __collector_env_print ("fork_epilogue child after env_update");
      __collector_clean_state ();
      __collector_env_print ("fork_epilogue child after clean_slate");
      __collector_line_cleanup ();
      __collector_env_print ("fork_epilogue child after line_cleanup");
      if (*following_fork)
	{
	  /* stop recording this experiment, but preserve env vars */
	  linetrace_dormant ();
	  __collector_env_print ("fork_epilogue child after linetrace_dormant");

	  //static char exp_name_env[LT_MAXPATHLEN];
	  char * exp_name_env = CALL_UTIL (calloc)(LT_MAXPATHLEN, 1);
	  CALL_UTIL (snprintf)(exp_name_env, LT_MAXPATHLEN, "%s=%s", SP_COLLECTOR_EXPNAME, new_exp_name);
	  CALL_UTIL (putenv)(exp_name_env);

	  const char *params = CALL_UTIL (getenv)(SP_COLLECTOR_PARAMS);
	  int ret;
	  if (*new_exp_name == 0)
	    TprintfT (DBG_LT0, "linetrace_ext_fork_epilogue: ERROR: getenv(%s) undefined -- new expt aborted!\n",
		      SP_COLLECTOR_EXPNAME);
	  else if (params == NULL)
	    TprintfT (DBG_LT0, "linetrace_ext_fork_epilogue: ERROR: getenv(%s) undefined -- new expt aborted!\n",
		      SP_COLLECTOR_PARAMS);
	  else if ((ret = __collector_open_experiment (new_exp_name, params, SP_ORIGIN_FORK)))
	    TprintfT (DBG_LT0, "linetrace_ext_fork_epilogue: ERROR: '%s' open failed, ret=%d\n",
		      new_exp_name, ret);
	  else
	    TprintfT (DBG_LT0, "linetrace_ext_fork_epilogue: opened(%s)\n", new_exp_name);
	  TprintfT (DBG_LT0, "linetrace_ext_fork_epilogue(%s) returning to *child*\n", variant);
	}
      else
	{
	  /* disable current and further linetrace experiment resumption */
	  TprintfT (DBG_LT0, "linetrace_ext_fork_epilogue(%s) child calling line_close\n", variant);
	  __collector_ext_line_close ();
	}
      __collector_env_print ("fork_epilogue child at end");
      /* *************************************end child */
    }
  else
    {
      /* *************************************parent */
      __collector_env_print ("fork_epilogue parent at start");
      __collector_ext_dispatcher_thread_timer_resume ();
      __collector_ext_hwc_lwp_resume ();
      hrtime_t ts = GETRELTIME ();
      char msg[256 + LT_MAXPATHLEN];
      if (ret >= 0)
	CALL_UTIL (snprintf)(msg, sizeof (msg), "pid=%d", ret);
      else
	{
	  /* delete stillborn experiment? */
	  char errmsg[256];
	  strerror_r (errno, errmsg, sizeof (errmsg));
	  CALL_UTIL (snprintf)(msg, sizeof (msg), "err %s", errmsg);
	}
      __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" variant=\"%s\" lineage=\"%s\" follow=\"%d\" msg=\"%s\"/>\n",
			     SP_JCMD_DESC_STARTED,
			     (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC),
			     variant, n_lineage, *following_fork, msg);
      /* environment remains set for collection */
      __collector_env_print ("fork_epilogue parent at end");
      /* *************************************end parent */
    }
  dbg_current_mode = FOLLOW_NONE;
  *following_fork = 0;
}

static char**
linetrace_ext_exec_prologue_end (const char *variant, const char* cmd_string,
				 char *const envp[], int following_exec)
{
  char **coll_env;
  TprintfT (DBG_LT0, "linetrace_ext_exec_prologue_end; variant=%s; cmd_string=%s; follow=%d\n",
	    variant, cmd_string, following_exec);
  /* write message before suspending, or it won't be written */
  hrtime_t ts = GETRELTIME ();
  __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" variant=\"%s\" lineage=\"%s\" follow=\"%d\" msg=\"%s\"/>\n",
			 SP_JCMD_EXEC_START,
			 (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC),
			 variant, new_lineage, following_exec, cmd_string);
  if (following_exec)
    {
      coll_env = __collector_env_allocate (envp, 0);
      __collector_env_update (coll_env);
      extern char **environ; /* the process' actual environment */
      if (environ == envp)   /* user selected process environment */
	environ = coll_env;
    }
  else
    coll_env = (char**) envp;
  __collector_env_printall ("linetrace_ext_exec_prologue_end", coll_env);
  if (!CALL_UTIL (strstr)(variant, "posix_spawn"))
    {
      __collector_linetrace_shutdown_hwcs_6830763_XXXX = 1;
      __collector_suspend_experiment ("suspend_for_exec");
      __collector_linetrace_shutdown_hwcs_6830763_XXXX = 0;
    }
  if (CALL_UTIL (strstr)(variant, "posix_spawn"))
    {
      __collector_ext_dispatcher_thread_timer_suspend ();
      __collector_linetrace_shutdown_hwcs_6830763_XXXX = 1;
      __collector_ext_hwc_lwp_suspend ();
      __collector_linetrace_shutdown_hwcs_6830763_XXXX = 0;
    }
  return (coll_env);
}

static char**
linetrace_ext_exec_prologue (const char *variant,
			     const char* path, char *const argv[],
			     char *const envp[], int *following_exec)
{
  char cmd_string[_POSIX_ARG_MAX] = {'\0'};

  if (dbg_current_mode != FOLLOW_NONE)
    TprintfT (DBG_LT0, "linetrace_ext_exec_prologue() ERROR: dbg_current_mode=%d, changing to FOLLOW_EXEC!\n", dbg_current_mode);
  dbg_current_mode = FOLLOW_ON;
  *following_exec = check_follow_exec (path);
  if (path != NULL)
    {
      /* escape any newline, " or \ characters in the exec command */
      TprintfT (DBG_LT3, "linetrace_ext_exec_prologue(): arg0=%s\n", path);
      /* leave space in log message for terminator (and header) */
      __collector_strlcpy (cmd_string, path, sizeof (cmd_string));
      size_t len;
      unsigned argn = 0;
      if (argv[0])
	{
	  char *p;
	  while (((p = argv[++argn]) != 0) &&
		 (len = __collector_strlen (cmd_string)) < sizeof (cmd_string) - 2)
	    {
	      cmd_string[len++] = ' ';
	      __collector_strlcpy (cmd_string + len, p, sizeof (cmd_string) - len);
	    }
	}
    }
  TprintfT (DBG_LT0, "linetrace_ext_exec_prologue(%s), lineage=%s, follow=%d, prog=%s, path=%s \n",
	    variant, new_lineage, *following_exec, cmd_string, path);
  return linetrace_ext_exec_prologue_end (variant, cmd_string, envp, *following_exec);
}

static void
linetrace_ext_exec_epilogue (const char *variant, char *const envp[], const int ret, int *following_exec)
{
  /* For exec, this routine is only entered if the exec failed */
  /* However, posix_spawn() is expected to return */
  if (dbg_current_mode == FOLLOW_NONE)
    TprintfT (DBG_LT0, "linetrace_ext_exec_epilogue() ERROR: dbg_current_mode=%d!\n", dbg_current_mode);
  TprintfT (DBG_LT0, "linetrace_ext_exec_epilogue(%s):%d returned: %d, errno=%d\n",
	    variant, *following_exec, ret, errno);
  if (!CALL_UTIL (strstr)(variant, "posix_spawn"))
    {
      __collector_linetrace_shutdown_hwcs_6830763_XXXX = 1;
      __collector_resume_experiment ();
      __collector_linetrace_shutdown_hwcs_6830763_XXXX = 0;
    }
  if (CALL_UTIL (strstr)(variant, "posix_spawn"))
    {
      __collector_ext_dispatcher_thread_timer_resume ();
      __collector_linetrace_shutdown_hwcs_6830763_XXXX = 1;
      __collector_ext_hwc_lwp_resume ();
      __collector_linetrace_shutdown_hwcs_6830763_XXXX = 0;
    }
  hrtime_t ts = GETRELTIME ();
  char msg[256];
  if (ret)
    {
      char errmsg[256];
      strerror_r (errno, errmsg, sizeof (errmsg));
      CALL_UTIL (snprintf)(msg, sizeof (msg), "err %s", errmsg);
    }
  else
    CALL_UTIL (snprintf)(msg, sizeof (msg), "rc=%d", ret);
  if (!CALL_UTIL (strstr)(variant, "posix_spawn"))
    __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" variant=\"%s\" lineage=\"%s\" follow=\"%d\" msg=\"%s\"/>\n",
			   SP_JCMD_EXEC_ERROR,
			   (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC),
			   variant, new_lineage, *following_exec, msg);
  if (envp == NULL)
    TprintfT (DBG_LT0, "linetrace_ext_exec_epilogue() ERROR: envp NULL after %s!\n", variant);
  dbg_current_mode = FOLLOW_NONE;
  *following_exec = 0;
}

static void
linetrace_ext_combo_prologue (const char *variant, const char *cmd, int *following_combo)
{
  char cmd_string[_POSIX_ARG_MAX] = {'\0'};
  char execfile[_POSIX_ARG_MAX] = {'\0'};

  if (dbg_current_mode != FOLLOW_NONE)
    TprintfT (DBG_LT0, "linetrace_ext_combo_prologue() ERROR: dbg_current_mode=%d!  changing to FOLLOW_ON\n",
	      dbg_current_mode);
  dbg_current_mode = FOLLOW_ON;
  if (cmd != NULL)
    {
      /* extract executable name from combo command */
      unsigned len = strcspn (cmd, " ");
      __collector_strlcpy (execfile, cmd, len + 1);

      /* escape any newline, " or \ characters in the combo command */
      /* leave space in log message for terminator (and header) */
      __collector_strlcpy (cmd_string, cmd, sizeof (cmd_string));
    }

  *following_combo = check_follow_combo (execfile);
  TprintfT (DBG_LT0, "linetrace_ext_combo_prologue(%s) follow=%d, prog=%s\n\n",
	    variant, *following_combo, cmd_string);

  /* Construct the lineage string for the new image */
  new_lineage[0] = 0;
  __collector_strcat (new_lineage, "XXX");

  /* write message before suspending, or it won't be written */
  hrtime_t ts = GETRELTIME ();
  __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" variant=\"%s\" lineage=\"%s\" follow=\"%d\" msg=\"%s\"/>\n",
			 SP_JCMD_DESC_START,
			 (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC),
			 variant, new_lineage, *following_combo, cmd_string);
  if (*following_combo)
    {
      __collector_env_update (NULL);
      TprintfT (DBG_LT0, "linetrace_ext_combo_prologue(): Following %s(\"%s\")\n", variant, execfile);
    }
  __collector_ext_dispatcher_thread_timer_suspend ();
  __collector_linetrace_shutdown_hwcs_6830763_XXXX = 1;
  __collector_ext_hwc_lwp_suspend ();
  __collector_linetrace_shutdown_hwcs_6830763_XXXX = 0;
}

static void
linetrace_ext_combo_epilogue (const char *variant, const int ret, int *following_combo)
{
  if (dbg_current_mode == FOLLOW_NONE)
    TprintfT (DBG_LT0, "linetrace_ext_combo_epilogue() ERROR: dbg_current_mode=FOLLOW_NONE\n");
  TprintfT (DBG_LT0, "linetrace_ext_combo_epilogue(%s):%d() returned %d\n",
	    variant, *following_combo, ret);
  __collector_ext_dispatcher_thread_timer_resume ();
  __collector_linetrace_shutdown_hwcs_6830763_XXXX = 1;
  __collector_ext_hwc_lwp_resume ();
  __collector_linetrace_shutdown_hwcs_6830763_XXXX = 0;
  hrtime_t ts = GETRELTIME ();
  __collector_log_write ("<event kind=\"%s\" tstamp=\"%u.%09u\" variant=\"%s\" follow=\"%d\" msg=\"rc=%d\"/>\n",
			 SP_JCMD_DESC_STARTED,
			 (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC),
			 variant, *following_combo, ret);

  dbg_current_mode = FOLLOW_NONE;
  *following_combo = 0;
}

/*------------------------------------------------------------- fork */
pid_t fork () __attribute__ ((weak, alias ("__collector_fork")));
pid_t _fork () __attribute__ ((weak, alias ("__collector_fork")));

pid_t
__collector_fork (void)
{
  pid_t ret;
  if (NULL_PTR (fork))
    {
      TprintfT (DBG_LT0, "__collector_fork() calling init_lineage_intf()\n");
      init_lineage_intf ();
    }
  __collector_env_print ("__collector_fork start");
  int * guard = NULL;
  int combo_flag = (line_mode == LM_TRACK_LINEAGE) ? ((CHCK_REENTRANCE (guard)) ? 1 : 0) : 0;
  TprintfT (DBG_LT0, "__collector_fork() interposition: line_mode=%d combo=%d\n",
	    line_mode, combo_flag);
  if ((line_mode != LM_TRACK_LINEAGE) || combo_flag)
    {
      TprintfT (DBG_LT0, "__collector_fork() not following, returning CALL_REAL(fork)()\n");
      return CALL_REAL (fork)();
    }
  int following_fork = 0;
  linetrace_ext_fork_prologue ("fork", new_lineage, &following_fork);

  /* since libpthread/fork ends up calling fork1, it's a combo */
  PUSH_REENTRANCE (guard);
  ret = CALL_REAL (fork)();
  POP_REENTRANCE (guard);
  linetrace_ext_fork_epilogue ("fork", ret, new_lineage, &following_fork);
  return ret;
}

/*------------------------------------------------------------- vfork */
/* vfork interposition in the usual sense is not possible, since vfork(2)
   relies on specifics of the stack frames in the parent and child which
   only work when the child's exec (or _exit) are in the same stack frame
   as the vfork: this isn't the case when there's interposition on exec.
   As a workaround, the interposing vfork calls fork1 instead of the real
   vfork.  Note that fork1 is somewhat less efficient than vfork, and requires
   additional memory, which may result in a change of application behaviour
   when libcollector is loaded (even when collection is not active),
   affecting not only direct use of vfork by the subject application,
   but also indirect use through system, popen, and the other combos.
 */
pid_t vfork () __attribute__ ((weak, alias ("__collector_vfork")));
pid_t _vfork () __attribute__ ((weak, alias ("__collector_vfork")));

pid_t
__collector_vfork (void)
{
  if (NULL_PTR (vfork))
    init_lineage_intf ();

  int * guard = NULL;
  int combo_flag = (line_mode == LM_TRACK_LINEAGE) ? ((CHCK_REENTRANCE (guard)) ? 1 : 0) : 0;

  TprintfT (DBG_LT0, "__collector_vfork() interposing: line_mode=%d combo=%d\n",
	    line_mode, combo_flag);
  if ((line_mode != LM_TRACK_LINEAGE) || combo_flag)
    return CALL_REAL (fork)();

  /* this warning is also appropriate for combos which use vfork,
     however, let's assume this is achieved elsewhere */
  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n", SP_JCMD_CWARN,
				COL_WARN_VFORK, "fork");

  char new_lineage[LT_MAXNAMELEN];
  new_lineage[0] = 0;
  int following_fork = 0;
  linetrace_ext_fork_prologue ("vfork", new_lineage, &following_fork);

  pid_t ret = CALL_REAL (fork)();
  linetrace_ext_fork_epilogue ("vfork", ret, new_lineage, &following_fork);
  return ret;
}

/*------------------------------------------------------------- execve */
int execve () __attribute__ ((weak, alias ("__collector_execve")));

int
__collector_execve (const char* path, char *const argv[], char *const envp[])
{
  static char **coll_env = NULL; /* environment for collection */
  if (NULL_PTR (execve))
    init_lineage_intf ();
  int * guard = NULL;
  int combo_flag = (line_mode == LM_TRACK_LINEAGE) ? ((CHCK_REENTRANCE (guard)) ? 1 : 0) : 0;
  TprintfT (DBG_LT0,
	    "__collector_execve(path=%s, argv[0]=%s, env[0]=%s) interposing: line_mode=%d combo=%d\n",
	    path ? path : "NULL",
	    argv ? (argv[0] ? argv[0] : "NULL") : "NULL",
	    envp ? (envp[0] ? envp[0] : "NULL") : "NULL",
	    line_mode, combo_flag);
  if (line_mode == LM_CLOSED) /* ensure collection environment is sanitised */
    __collector_env_unset ((char**) envp);
  if (line_mode != LM_TRACK_LINEAGE || combo_flag)
    return CALL_REAL (execve)(path, argv, envp);

  int following_exec = 0;
  coll_env = linetrace_ext_exec_prologue ("execve", path, argv, envp, &following_exec);
  TprintfT (DBG_LT2, "__collector_execve(): coll_env=0x%p\n", coll_env);
  __collector_env_printall ("__collector_execve", coll_env);
  int ret = CALL_REAL (execve)(path, argv, coll_env);
  linetrace_ext_exec_epilogue ("execve", envp, ret, &following_exec);
  return ret;
}

int execvp () __attribute__ ((weak, alias ("__collector_execvp")));

int
__collector_execvp (const char* file, char *const argv[])
{
  extern char **environ; /* the process' actual environment */
  char ** envp = environ;
  if (NULL_PTR (execvp))
    init_lineage_intf ();
  int * guard = NULL;
  int combo_flag = (line_mode == LM_TRACK_LINEAGE) ? ((CHCK_REENTRANCE (guard)) ? 1 : 0) : 0;
  TprintfT (DBG_LT0,
	    "__collector_execvp(file=%s, argv[0]=%s) interposing: line_mode=%d combo=%d\n",
	    file ? file : "NULL", argv ? (argv[0] ? argv[0] : "NULL") : "NULL",
	    line_mode, combo_flag);
  if (line_mode == LM_CLOSED) /* ensure collection environment is sanitised */
    __collector_env_unset ((char**) envp);
  if ((line_mode != LM_TRACK_LINEAGE) || combo_flag)
    return CALL_REAL (execvp)(file, argv);

  int following_exec = 0;
#ifdef DEBUG
  char **coll_env = /* environment for collection */
#endif /* DEBUG */
	  linetrace_ext_exec_prologue ("execvp", file, argv, envp, &following_exec);
  TprintfT (DBG_LT0, "__collector_execvp(): coll_env=0x%p\n", coll_env);

  int ret = CALL_REAL (execvp)(file, argv);
  linetrace_ext_exec_epilogue ("execvp", envp, ret, &following_exec);
  return ret;
}

int execv () __attribute__ ((weak, alias ("__collector_execv")));

int
__collector_execv (const char* path, char *const argv[])
{
  int ret;
  extern char **environ; /* the process' actual environment */
  char ** envp = environ;
  TprintfT (DBG_LT0, "__collector_execv(path=%s, argv[0]=%s) interposing: line_mode=%d combo=%d\n",
	    path ? path : "NULL", argv ? (argv[0] ? argv[0] : "NULL") : "NULL",
	    line_mode, get_combo_flag ());

  ret = __collector_execve (path, argv, envp);
  return ret;
}

int execle (const char* path, const char *arg0, ...) __attribute__ ((weak, alias ("__collector_execle")));

int
__collector_execle (const char* path, const char *arg0, ...)
{
  TprintfT (DBG_LT0,
	    "__collector_execle(path=%s, arg0=%s) interposing: line_mode=%d combo=%d\n",
	    path ? path : "NULL", arg0 ? arg0 : "NULL",
	    line_mode, get_combo_flag ());

  char **argp;
  va_list args;
  char **argvec;
  register char **environmentp;
  int nargs = 0;
  char *nextarg;

  va_start (args, arg0);
  while (va_arg (args, char *) != (char *) 0)
    nargs++;

  /*
   * save the environment pointer, which is at the end of the
   * variable argument list
   */
  environmentp = va_arg (args, char **);
  va_end (args);

  /*
   * load the arguments in the variable argument list
   * into the argument vector, and add the terminating null pointer
   */
  va_start (args, arg0);
  /* workaround for bugid 1242839 */
  argvec = (char **) alloca ((size_t) ((nargs + 2) * sizeof (char *)));
  argp = argvec;
  *argp++ = (char *) arg0;
  while ((nextarg = va_arg (args, char *)) != (char *) 0)
    *argp++ = nextarg;
  va_end (args);
  *argp = (char *) 0;
  return __collector_execve (path, argvec, environmentp);
}

int execlp (const char* file, const char *arg0, ...) __attribute__ ((weak, alias ("__collector_execlp")));

int
__collector_execlp (const char* file, const char *arg0, ...)
{
  TprintfT (DBG_LT0,
	    "__collector_execlp(file=%s, arg0=%s) interposing: line_mode=%d combo=%d\n",
	    file ? file : "NULL", arg0 ? arg0 : "NULL",
	    line_mode, get_combo_flag ());
  char **argp;
  va_list args;
  char **argvec;
  int nargs = 0;
  char *nextarg;

  va_start (args, arg0);
  while (va_arg (args, char *) != (char *) 0)
    nargs++;
  va_end (args);

  /*
   * load the arguments in the variable argument list
   * into the argument vector and add the terminating null pointer
   */
  va_start (args, arg0);

  /* workaround for bugid 1242839 */
  argvec = (char **) alloca ((size_t) ((nargs + 2) * sizeof (char *)));
  argp = argvec;
  *argp++ = (char *) arg0;
  while ((nextarg = va_arg (args, char *)) != (char *) 0)
    *argp++ = nextarg;
  va_end (args);
  *argp = (char *) 0;
  return __collector_execvp (file, argvec);
}

int execl (const char* path, const char *arg0, ...) __attribute__ ((weak, alias ("__collector_execl")));

int
__collector_execl (const char* path, const char *arg0, ...)
{
  TprintfT (DBG_LT0,
	    "__collector_execl(path=%s, arg0=%s) interposing: line_mode=%d combo=%d\n",
	    path ? path : "NULL", arg0 ? arg0 : "NULL",
	    line_mode, get_combo_flag ());
  char **argp;
  va_list args;
  char **argvec;
  extern char **environ;
  int nargs = 0;
  char *nextarg;
  va_start (args, arg0);
  while (va_arg (args, char *) != (char *) 0)
    nargs++;
  va_end (args);

  /*
   * load the arguments in the variable argument list
   * into the argument vector and add the terminating null pointer
   */
  va_start (args, arg0);

  /* workaround for bugid 1242839 */
  argvec = (char **) alloca ((size_t) ((nargs + 2) * sizeof (char *)));
  argp = argvec;
  *argp++ = (char *) arg0;
  while ((nextarg = va_arg (args, char *)) != (char *) 0)
    *argp++ = nextarg;
  va_end (args);
  *argp = (char *) 0;
  return __collector_execve (path, argvec, environ);
}

#include <spawn.h>

/*-------------------------------------------------------- posix_spawn */
// map interposed symbol versions
static int
gprofng_posix_spawn (int(real_posix_spawn) (),
		     pid_t *pidp, const char *path,
		     const posix_spawn_file_actions_t *file_actions,
		     const posix_spawnattr_t *attrp,
		     char *const argv[], char *const envp[])
{
  int ret;
  static char **coll_env = NULL; /* environment for collection */
  if (real_posix_spawn == NULL)
    {
      TprintfT (DBG_LT0, "gprofng_posix_spawn (path=%s) interposin ERROR, posix_spawn() not found by dlsym\n",
		path ? path : "NULL");
      return -1; /* probably should set errno */
    }
  int *guard = NULL;
  int combo_flag = (line_mode == LM_TRACK_LINEAGE &&
		    CHCK_REENTRANCE (guard)) ? 1 : 0;
  TprintfT (DBG_LT0, "gprofng_posix_spawn @%p (%s, argv[0]=%s, env[0]=%s)"
	    "line_mode=%d combo=%d\n", (real_posix_spawn), path ? path : "NULL",
	    (argv && argv[0]) ? argv[0] : "NULL",
	    (envp && envp[0]) ? envp[0] : "NULL", line_mode, combo_flag);
  if (line_mode == LM_CLOSED) /* ensure collection environment is sanitised */
    __collector_env_unset ((char**) envp);

  if (line_mode != LM_TRACK_LINEAGE || combo_flag)
    return (real_posix_spawn) (pidp, path, file_actions, attrp, argv, envp);
  int following_exec = 0;
  coll_env = linetrace_ext_exec_prologue ("posix_spawn", path, argv, envp,
					  &following_exec);
  __collector_env_printall ("gprofng_posix_spawn", coll_env);
  PUSH_REENTRANCE (guard);
  ret = real_posix_spawn (pidp, path, file_actions, attrp, argv, coll_env);
  POP_REENTRANCE (guard);
  linetrace_ext_exec_epilogue ("posix_spawn", envp, ret, &following_exec);
  return ret;
}

#define DCL_POSIX_SPAWN(dcl_f) \
int dcl_f (pid_t *pidp, const char *path, \
	   const posix_spawn_file_actions_t *file_actions, \
	   const posix_spawnattr_t *attrp, \
	   char *const argv[], char *const envp[]) \
  { \
    if (__real_posix_spawn == NULL) \
      init_lineage_intf (); \
    return gprofng_posix_spawn (__real_posix_spawn, pidp, path, file_actions, \
				attrp, argv, envp); \
  }


DCL_FUNC_VER (DCL_POSIX_SPAWN, posix_spawn_2_17, posix_spawn@GLIBC_2.17)
DCL_FUNC_VER (DCL_POSIX_SPAWN, posix_spawn_2_15, posix_spawn@GLIBC_2.15)
DCL_FUNC_VER (DCL_POSIX_SPAWN, posix_spawn_2_2_5, posix_spawn@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_POSIX_SPAWN, posix_spawn_2_2, posix_spawn@GLIBC_2.2)
DCL_POSIX_SPAWN (posix_spawn)

/*-------------------------------------------------------- posix_spawnp */
static int
gprofng_posix_spawnp (int (real_posix_spawnp) (),
                      pid_t *pidp, const char *path,
                      const posix_spawn_file_actions_t *file_actions,
                      const posix_spawnattr_t *attrp,
                      char *const argv[], char *const envp[])
{
  static char **coll_env = NULL; /* environment for collection */

  if (real_posix_spawnp == NULL)
    {
      TprintfT (DBG_LT0, "gprofng_posix_spawnp (path=%s) interposin ERROR\n",
		path ? path : "NULL");
      return -1; /* probably should set errno */
    }
  int *guard = NULL;
  int combo_flag = (line_mode == LM_TRACK_LINEAGE) ? ((CHCK_REENTRANCE (guard)) ? 1 : 0) : 0;
  TprintfT (DBG_LT0, "gprofng_posix_spawnp @%p (path=%s, argv[0]=%s, env[0]=%s) line_mode=%d combo=%d\n",
	    real_posix_spawnp, path ? path : "NULL",
	    argv && argv[0] ? argv[0] : "NULL",
	    envp && envp[0] ? envp[0] : "NULL", line_mode, combo_flag);

  if (line_mode == LM_CLOSED) /* ensure collection environment is sanitized */
    __collector_env_unset ((char**) envp);
  if (line_mode != LM_TRACK_LINEAGE || combo_flag)
    return (real_posix_spawnp) (pidp, path, file_actions, attrp, argv, envp);
  int following_exec = 0;
  coll_env = linetrace_ext_exec_prologue ("posix_spawnp", path, argv, envp, &following_exec);
  TprintfT (DBG_LT0, "__collector_posix_spawnp(): coll_env=0x%p\n", coll_env);
  __collector_env_printall ("__collector_posix_spawnp", coll_env);
  PUSH_REENTRANCE (guard);
  int ret = real_posix_spawnp (pidp, path, file_actions, attrp, argv, coll_env);
  POP_REENTRANCE (guard);
  linetrace_ext_exec_epilogue ("posix_spawnp", envp, ret, &following_exec);
  return ret;
}

#define DCL_POSIX_SPAWNP(dcl_f) \
int dcl_f (pid_t *pidp, const char *path, \
	   const posix_spawn_file_actions_t *file_actions, \
	   const posix_spawnattr_t *attrp, \
	   char *const argv[], char *const envp[]) \
  { \
    if (__real_posix_spawnp == NULL) \
      init_lineage_intf (); \
    return gprofng_posix_spawnp (__real_posix_spawnp, pidp, path, \
				 file_actions, attrp, argv, envp); \
  }

DCL_FUNC_VER (DCL_POSIX_SPAWNP, posix_spawnp_2_17, posix_spawnp@GLIBC_2.17)
DCL_FUNC_VER (DCL_POSIX_SPAWNP, posix_spawnp_2_15, posix_spawnp@GLIBC_2.15)
DCL_FUNC_VER (DCL_POSIX_SPAWNP, posix_spawnp_2_2_5, posix_spawnp@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_POSIX_SPAWNP, posix_spawnp_2_2, posix_spawnp@GLIBC_2.2)
DCL_POSIX_SPAWNP (posix_spawnp)

/*------------------------------------------------------------- system */
int system () __attribute__ ((weak, alias ("__collector_system")));

int
__collector_system (const char *cmd)
{
  if (NULL_PTR (system))
    init_lineage_intf ();
  TprintfT (DBG_LT0,
	    "__collector_system(cmd=%s) interposing: line_mode=%d combo=%d\n",
	    cmd ? cmd : "NULL", line_mode, get_combo_flag ());
  int *guard = NULL;
  if (line_mode == LM_TRACK_LINEAGE)
    INIT_REENTRANCE (guard);
  if (guard == NULL)
    return CALL_REAL (system)(cmd);
  int following_combo = 0;
  linetrace_ext_combo_prologue ("system", cmd, &following_combo);
  PUSH_REENTRANCE (guard);
  int ret = CALL_REAL (system)(cmd);
  POP_REENTRANCE (guard);
  linetrace_ext_combo_epilogue ("system", ret, &following_combo);
  return ret;
}

/*------------------------------------------------------------- popen */
// map interposed symbol versions
#define DCL_POPEN(dcl_f) \
  FILE *dcl_f (const char *cmd, const char *mode) \
  { \
    if (__real_popen == NULL) \
      init_lineage_intf (); \
    TprintfT (DBG_LT0, #dcl_f " (%s) interposing: line_mode=%d combo=%d\n", \
	      cmd ? cmd : "NULL", line_mode, get_combo_flag ()); \
    int *guard = NULL; \
    if (line_mode == LM_TRACK_LINEAGE) \
      INIT_REENTRANCE (guard); \
    if (guard == NULL) \
      return __real_popen (cmd, mode); \
    int following_combo = 0; \
    linetrace_ext_combo_prologue ("popen", cmd, &following_combo); \
    PUSH_REENTRANCE (guard); \
    FILE *ret = __real_popen (cmd, mode); \
    POP_REENTRANCE (guard); \
    linetrace_ext_combo_epilogue ("popen", ret == NULL ? -1 : 0, \
				  &following_combo); \
    return ret; \
  }

DCL_FUNC_VER (DCL_POPEN, popen_2_17, popen@GLIBC_2.17)
DCL_FUNC_VER (DCL_POPEN, popen_2_2_5, popen@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_POPEN, popen_2_1, popen@GLIBC_2.1)
DCL_FUNC_VER (DCL_POPEN, popen_2_0, popen@GLIBC_2.0)
DCL_POPEN (popen)

/*------------------------------------------------------------- grantpt */
int grantpt () __attribute__ ((weak, alias ("__collector_grantpt")));

int
__collector_grantpt (const int fildes)
{
  if (NULL_PTR (grantpt))
    init_lineage_intf ();
  TprintfT (DBG_LT0,
	    "__collector_grantpt(%d) interposing: line_mode=%d combo=%d\n",
	    fildes, line_mode, get_combo_flag ());
  int *guard = NULL;
  if (line_mode == LM_TRACK_LINEAGE)
    INIT_REENTRANCE (guard);
  if (guard == NULL)
    return CALL_REAL (grantpt)(fildes);
  int following_combo = 0;
  linetrace_ext_combo_prologue ("grantpt", "/usr/lib/pt_chmod", &following_combo);
  PUSH_REENTRANCE (guard);
  int ret = CALL_REAL (grantpt)(fildes);
  POP_REENTRANCE (guard);
  linetrace_ext_combo_epilogue ("grantpt", ret, &following_combo);
  return ret;
}

/*------------------------------------------------------------- ptsname */
char *ptsname () __attribute__ ((weak, alias ("__collector_ptsname")));

char *
__collector_ptsname (const int fildes)
{
  if (NULL_PTR (ptsname))
    init_lineage_intf ();
  TprintfT (DBG_LT0,
	    "__collector_ptsname(%d) interposing: line_mode=%d combo=%d\n",
	    fildes, line_mode, get_combo_flag ());
  int *guard = NULL;
  if (line_mode == LM_TRACK_LINEAGE)
    INIT_REENTRANCE (guard);
  if (guard == NULL)
    return CALL_REAL (ptsname)(fildes);
  int following_combo = 0;
  linetrace_ext_combo_prologue ("ptsname", "/usr/lib/pt_chmod", &following_combo);
  PUSH_REENTRANCE (guard);
  char *ret = CALL_REAL (ptsname)(fildes);
  POP_REENTRANCE (guard);
  linetrace_ext_combo_epilogue ("ptsname", (ret == NULL) ? (-1) : 1, &following_combo);
  return ret;
}

/*------------------------------------------------------------- clone */
/* clone can be fork-like or pthread_create-like, depending on whether
 * the flag CLONE_VM is set. If CLONE_VM is not set, then we interpose
 * clone in the way similar to interposing fork; if CLONE_VM is set,
 * then we interpose clone in the way similar to interposing pthread_create.
 * One special case is not handled: when CLONE_VM is set but CLONE_THREAD
 * is not, if the parent process exits earlier than the child process,
 * experiment will close, losing data from child process.
 */
typedef struct __collector_clone_arg
{
  int (*fn)(void *);
  void * arg;
  char * new_lineage;
  int following_fork;
} __collector_clone_arg_t;

static int
__collector_clone_fn (void *fn_arg)
{
  int (*fn)(void *) = ((__collector_clone_arg_t*) fn_arg)->fn;
  void * arg = ((__collector_clone_arg_t*) fn_arg)->arg;
  char * new_lineage = ((__collector_clone_arg_t*) fn_arg)->new_lineage;
  int following_fork = ((__collector_clone_arg_t*) fn_arg)->following_fork;
  __collector_freeCSize (__collector_heap, fn_arg, sizeof (__collector_clone_arg_t));
  linetrace_ext_fork_epilogue ("clone", 0, new_lineage, &following_fork);
  return fn (arg);
}

int clone (int (*fn)(void *), void *, int, void *, ...) __attribute__ ((weak, alias ("__collector_clone")));

int
__collector_clone (int (*fn)(void *), void *child_stack, int flags, void *arg,
		   ... /* pid_t *ptid, struct user_desc *tls, pid_t *" ctid" */)
{
  int ret;
  va_list va;
  if (flags & CLONE_VM)
    {
      va_start (va, arg);
      ret = __collector_ext_clone_pthread (fn, child_stack, flags, arg, va);
      va_end (va);
    }
  else
    {
      if (NULL_PTR (clone))
	init_lineage_intf ();
      int *guard = NULL;
      int combo_flag = (line_mode == LM_TRACK_LINEAGE) ? ((CHCK_REENTRANCE (guard)) ? 1 : 0) : 0;
      TprintfT (DBG_LT0, "__collector_clone() interposition: line_mode=%d combo=%d\n",
		line_mode, combo_flag);
      char new_lineage[LT_MAXNAMELEN];
      int following_fork = 0;
      __collector_clone_arg_t *funcinfo = __collector_allocCSize (__collector_heap, sizeof (__collector_clone_arg_t), 1);
      (*funcinfo).fn = fn;
      (*funcinfo).arg = arg;
      (*funcinfo).new_lineage = new_lineage;
      (*funcinfo).following_fork = 0;
      pid_t * ptid = NULL;
      struct user_desc * tls = NULL;
      pid_t * ctid = NULL;
      int num_args = 0;
      va_start (va, arg);
      if (flags & (CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID))
	{
	  ptid = va_arg (va, pid_t *);
	  tls = va_arg (va, struct user_desc*);
	  ctid = va_arg (va, pid_t *);
	  num_args = 3;
	}
      else if (flags & CLONE_SETTLS)
	{
	  ptid = va_arg (va, pid_t *);
	  tls = va_arg (va, struct user_desc*);
	  num_args = 2;
	}
      else if (flags & CLONE_PARENT_SETTID)
	{
	  ptid = va_arg (va, pid_t *);
	  num_args = 1;
	}
      if ((line_mode != LM_TRACK_LINEAGE) || combo_flag || funcinfo == NULL)
	{
	  switch (num_args)
	    {
	    case 3:
	      ret = CALL_REAL (clone)(fn, child_stack, flags, arg, ptid, tls, ctid);
	      break;
	    case 2:
	      ret = CALL_REAL (clone)(fn, child_stack, flags, arg, ptid, tls);
	      break;
	    case 1:
	      ret = CALL_REAL (clone)(fn, child_stack, flags, arg, ptid);
	      break;
	    default:
	      ret = CALL_REAL (clone)(fn, child_stack, flags, arg);
	      break;
	    }

	  va_end (va);
	  return ret;
	}
      linetrace_ext_fork_prologue ("clone", new_lineage, &following_fork);
      (*funcinfo).following_fork = following_fork;
      switch (num_args)
	{
	case 3:
	  ret = CALL_REAL (clone)(__collector_clone_fn, child_stack, flags, funcinfo, ptid, tls, ctid);
	  break;
	case 2:
	  ret = CALL_REAL (clone)(__collector_clone_fn, child_stack, flags, funcinfo, ptid, tls);
	  break;
	case 1:
	  ret = CALL_REAL (clone)(__collector_clone_fn, child_stack, flags, funcinfo, ptid);
	  break;
	default:
	  ret = CALL_REAL (clone)(__collector_clone_fn, child_stack, flags, funcinfo);
	  break;
	}
      va_end (va);
      if (ret < 0)
	__collector_freeCSize (__collector_heap, funcinfo, sizeof (__collector_clone_arg_t));
      TprintfT (DBG_LT0, "__collector_clone() interposing: pid=%d\n", ret);
      linetrace_ext_fork_epilogue ("clone", ret, new_lineage, &following_fork);
    }
  return ret;
}

/*-------------------------------------------------------------------- setuid */
int setuid () __attribute__ ((weak, alias ("__collector_setuid")));
int _setuid () __attribute__ ((weak, alias ("__collector_setuid")));

int
__collector_setuid (uid_t ruid)
{
  if (NULL_PTR (setuid))
    init_lineage_intf ();
  TprintfT (DBG_LT0, "__collector_setuid(0x%x) interposing\n", ruid);
  check_reuid_change (ruid, -1);
  int ret = CALL_REAL (setuid)(ruid);
  TprintfT (DBG_LT0, "__collector_setuid(0x%x) returning %d\n", ruid, ret);
  return ret;
}

/*------------------------------------------------------------------- seteuid */
int seteuid () __attribute__ ((weak, alias ("__collector_seteuid")));
int _seteuid () __attribute__ ((weak, alias ("__collector_seteuid")));

int
__collector_seteuid (uid_t euid)
{
  if (NULL_PTR (seteuid))
    init_lineage_intf ();
  TprintfT (DBG_LT0, "__collector_seteuid(0x%x) interposing\n", euid);
  check_reuid_change (-1, euid);
  int ret = CALL_REAL (seteuid)(euid);
  TprintfT (DBG_LT0, "__collector_seteuid(0x%x) returning %d\n", euid, ret);
  return ret;
}

/*------------------------------------------------------------------ setreuid */
int setreuid () __attribute__ ((weak, alias ("__collector_setreuid")));
int _setreuid () __attribute__ ((weak, alias ("__collector_setreuid")));

int
__collector_setreuid (uid_t ruid, uid_t euid)
{
  if (NULL_PTR (setreuid))
    init_lineage_intf ();
  TprintfT (DBG_LT0, "__collector_setreuid(0x%x,0x%x) interposing\n", ruid, euid);
  check_reuid_change (ruid, euid);
  int ret = CALL_REAL (setreuid)(ruid, euid);
  TprintfT (DBG_LT0, "__collector_setreuid(0x%x,0x%x) returning %d\n", ruid, euid, ret);
  return ret;
}

/*-------------------------------------------------------------------- setgid */
int setgid () __attribute__ ((weak, alias ("__collector_setgid")));
int _setgid () __attribute__ ((weak, alias ("__collector_setgid")));

int
__collector_setgid (gid_t rgid)
{
  if (NULL_PTR (setgid))
    init_lineage_intf ();
  TprintfT (DBG_LT0, "__collector_setgid(0x%x) interposing\n", rgid);
  check_regid_change (rgid, -1);
  int ret = CALL_REAL (setgid)(rgid);
  TprintfT (DBG_LT0, "__collector_setgid(0x%x) returning %d\n", rgid, ret);
  return ret;
}

/*------------------------------------------------------------------- setegid */
int setegid () __attribute__ ((weak, alias ("__collector_setegid")));
int _setegid () __attribute__ ((weak, alias ("__collector_setegid")));

int
__collector_setegid (gid_t egid)
{
  if (NULL_PTR (setegid))
    init_lineage_intf ();
  TprintfT (DBG_LT0, "__collector_setegid(0x%x) interposing\n", egid);
  check_regid_change (-1, egid);
  int ret = CALL_REAL (setegid)(egid);
  TprintfT (DBG_LT0, "__collector_setegid(0x%x) returning %d\n", egid, ret);
  return ret;
}

/*------------------------------------------------------------------ setregid */
int setregid () __attribute__ ((weak, alias ("__collector_setregid")));
int _setregid () __attribute__ ((weak, alias ("__collector_setregid")));

int
__collector_setregid (gid_t rgid, gid_t egid)
{
  if (NULL_PTR (setregid))
    init_lineage_intf ();
  TprintfT (DBG_LT0, "__collector_setregid(0x%x,0x%x) interposing\n", rgid, egid);
  check_regid_change (rgid, egid);
  int ret = CALL_REAL (setregid)(rgid, egid);
  TprintfT (DBG_LT0, "__collector_setregid(0x%x,0x%x) returning %d\n", rgid, egid, ret);
  return ret;
}

/*------------------------------------------------------- selective following */

static int
linetrace_follow_experiment (const char *follow_spec, const char *lineage_str, const char *progname)
{
  regex_t regex_desc;
  if (!follow_spec)
    {
      TprintfT (DBG_LT0, "linetrace_follow_experiment(): MATCHES NULL follow_spec\n");
      return 1;
    }
  int ercode = regcomp (&regex_desc, follow_spec, REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
  if (ercode)
    {
      // syntax error in parsing string
#ifdef DEBUG
      char errbuf[256];
      regerror (ercode, &regex_desc, errbuf, sizeof (errbuf));
      TprintfT (DBG_LT0, "linetrace_follow_experiment: regerror()=%s\n", errbuf);
#endif
      return 1;
    }
  TprintfT (DBG_LT0, "linetrace_follow_experiment(): compiled spec='%s'\n", follow_spec);
  if (lineage_str)
    {
      if (!regexec (&regex_desc, lineage_str, 0, NULL, 0))
	{
	  TprintfT (DBG_LT0, "linetrace_follow_experiment(): MATCHES lineage (follow_spec=%s,lineage=%s)\n",
		    follow_spec, lineage_str);
	  return 1;
	}
    }
  if (progname)
    {
      if (!regexec (&regex_desc, progname, 0, NULL, 0))
	{
	  TprintfT (DBG_LT0, "linetrace_follow_experiment(): MATCHES progname (follow_spec=%s,progname=%s)\n",
		    follow_spec, progname);
	  return 1;
	}
    }
  TprintfT (DBG_LT0, "linetrace_follow_experiment(): DOES NOT MATCH (follow_spec=%s,lineage=%s,progname=%s)\n",
	    follow_spec, lineage_str ? lineage_str : "NULL",
	    progname ? progname : "NULL");
  return 0;
}
