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
 *	Routines for managing the target's environment array
 */

#include "config.h"
#include "descendants.h"

#define MAX_LD_PRELOADS 2

/* TprintfT(<level>,...) definitions.  Adjust per module as needed */
#define DBG_LT0 0 // for high-level configuration, unexpected errors/warnings
#define DBG_LT1 1 // for configuration details, warnings
#define DBG_LT2 2
#define DBG_LT3 3
#define DBG_LT4 4

/* original environment settings to be saved for later restoration */
static char *sp_preloads[MAX_LD_PRELOADS];
static char *sp_libpaths[MAX_LD_PRELOADS];
char **sp_env_backup;

static const char *SP_ENV[];
static const char *LD_ENV[];
static const char *SP_PRELOAD[];
static const char *LD_PRELOAD[];
static const char *SP_LIBRARY_PATH[];
static const char *LD_LIBRARY_PATH[];
static int NUM_SP_ENV_VARS;
static int NUM_LD_ENV_VARS;
static int NUM_SP_PRELOADS;
static int NUM_LD_PRELOADS;
static int NUM_SP_LIBPATHS;
static int NUM_LD_LIBPATHS;

static const char *SP_ENV[] = {
  SP_COLLECTOR_PARAMS,      /* data descriptor */
  SP_COLLECTOR_EXPNAME,     /* experiment name */
  SP_COLLECTOR_FOLLOW_SPEC, /* linetrace */
  SP_COLLECTOR_FOUNDER,     /* determine founder exp */
  SP_PRELOAD_STRINGS,       /* LD_PRELOADs for data collection */
  SP_LIBPATH_STRINGS,       /* LD_LIBRARY_PATHs for data collection */
  "SP_COLLECTOR_TRACELEVEL", /* tprintf */
#if DEBUG
  "SP_COLLECTOR_SIGACTION", /* dispatcher, hwprofile */
#endif
  /* JAVA* */
  /* LD_DEBUG=audit,bindings,detail */
  /* LD_ORIGIN=yes */
  NULL
};

static const char *LD_ENV[] = {
  LD_PRELOAD_STRINGS,       /* LD_PRELOADs */
  LD_LIBPATH_STRINGS,       /* LD_LIBRARY_PATHs */
  JAVA_TOOL_OPTIONS,        /* enable -agentlib:collector for JVMTI */
  NULL
};

static const char *SP_PRELOAD[] = {
  SP_PRELOAD_STRINGS,
  NULL
};

static const char *LD_PRELOAD[] = {
  LD_PRELOAD_STRINGS,
  NULL
};

static const char *SP_LIBRARY_PATH[] = {
  SP_LIBPATH_STRINGS,
  NULL
};
static const char *LD_LIBRARY_PATH[] = {
  LD_LIBPATH_STRINGS,
  NULL
};

void
__collector_env_save_preloads ()
{
  /* save the list of SP_PRELOADs */
  int v;
  for (v = 0; SP_PRELOAD[v]; v++)
    {
      sp_preloads[v] = __collector_strdup (CALL_UTIL (getenv)(SP_PRELOAD[v]));
      TprintfT (DBG_LT3, "__collector_env_save_preloads: %s=%s\n", SP_PRELOAD[v], sp_preloads[v]);
    }
  NUM_SP_PRELOADS = v;
  for (v = 0; SP_LIBRARY_PATH[v]; v++)
    {
      sp_libpaths[v] = __collector_strdup (CALL_UTIL (getenv)(SP_LIBRARY_PATH[v]));
      TprintfT (DBG_LT4, "__collector_env_save_preloads: %s=%s\n", SP_LIBRARY_PATH[v],
		sp_libpaths[v] ? sp_libpaths[v] : "NULL");
    }
  NUM_SP_LIBPATHS = v;
  for (v = 0; LD_PRELOAD[v]; v++)
    ;
  NUM_LD_PRELOADS = v;
  for (v = 0; LD_LIBRARY_PATH[v]; v++)
    ;
  NUM_LD_LIBPATHS = v;
  for (v = 0; SP_ENV[v]; v++)
    ;
  NUM_SP_ENV_VARS = v;
  for (v = 0; LD_ENV[v]; v++)
    ;
  NUM_LD_ENV_VARS = v;
}

/* free the memory involved in backing up the environment */
void
__collector_env_backup_free ()
{
  int v = 0;
  TprintfT (DBG_LT2, "env_backup_free()\n");
  for (v = 0; sp_env_backup[v]; v++)
    {
      TprintfT (DBG_LT2, "env_backup_free():sp_env_backup[%d]=%s \n", v, sp_env_backup[v]);
      __collector_freeCSize (__collector_heap, (char *) sp_env_backup[v], __collector_strlen (sp_env_backup[v]) + 1);
    }
  __collector_freeCSize (__collector_heap, (char**) sp_env_backup,
			 (NUM_SP_ENV_VARS + NUM_LD_ENV_VARS + 1) * sizeof (char*));
}

char **
__collector_env_backup ()
{
  TprintfT (DBG_LT2, "env_backup_()\n");
  char **backup = __collector_env_allocate (NULL, 1);
  __collector_env_update (backup);
  TprintfT (DBG_LT2, "env_backup_()\n");
  return backup;
}

/*
   function: env_prepend()
     given an <old_str>, check to see if <str>
     is already defined by it.  If not, allocate
     a new string and concat <envvar>=<str><separator><old_str>
   params:
     old_str: original string
     str: substring to prepend
     return: pointer to updated string or NULL if string was not updated.
 */
static char *
env_prepend (const char *envvar, const char *str, const char *separator,
	     const char *old_str)
{
  if (!envvar || *envvar == 0 || !str || *str == 0)
    {
      /* nothing to do */
      TprintfT (DBG_LT2, "env_prepend(\"%s\", \"%s\", \"%s\", \"%s\") -- nothing to do\n",
		envvar, str, separator, old_str);

      return NULL;
    }
  TprintfT (DBG_LT2, "env_prepend(\"%s\", \"%s\", \"%s\", \"%s\")\n",
	    envvar, str, separator, old_str);
  char *ev;
  size_t strsz;
  if (!old_str || *old_str == 0)
    {
      strsz = __collector_strlen (envvar) + 1 + __collector_strlen (str) + 1;
      ev = (char*) __collector_allocCSize (__collector_heap, strsz, 1);
      if (ev)
	{
	  CALL_UTIL (snprintf)(ev, strsz, "%s=%s", envvar, str);
	  assert (__collector_strlen (ev) + 1 == strsz);
	}
      else
	TprintfT (DBG_LT2, "env_prepend(): could not allocate memory\n");
    }
  else
    {
      char *p = CALL_UTIL (strstr)(old_str, str);
      if (p)
	{
	  TprintfT (DBG_LT2, "env_prepend(): %s=%s was already set\n",
		    envvar, old_str);
	  return NULL;
	}
      strsz = __collector_strlen (envvar) + 1 + __collector_strlen (str) +
	      __collector_strlen (separator) + __collector_strlen (old_str) + 1;
      ev = (char*) __collector_allocCSize (__collector_heap, strsz, 1);
      if (ev)
	{
	  CALL_UTIL (snprintf)(ev, strsz, "%s=%s%s%s", envvar, str, separator, old_str);
	  assert (__collector_strlen (ev) + 1 == strsz);
	}
      else
	TprintfT (DBG_LT2, "env_prepend(): could not allocate memory\n");
    }
  TprintfT (DBG_LT2, "env_prepend(\"%s\", \"%s\", \"%s\", \"%s\") returns \"%s\"\n",
	    envvar, str, separator, old_str, (ev == NULL ? "NULL" : ev));
  return ev;
}

/*
   function: putenv_prepend()
     get environment variable <envvar>, check to see if <str>
     is already defined by it.  If not prepend <str>
     and put it back to environment.
   params:
     envvar: environment variable
     str: substring to find
     return: 0==success, nonzero on failure.
 */
int
putenv_prepend (const char *envvar, const char *str, const char *separator)
{
  if (!envvar || *envvar == 0)
    return 1;
  const char * old_str = CALL_UTIL (getenv)(envvar);
  char * newstr = env_prepend (envvar, str, separator, old_str);
  if (newstr)
    // now put the new variable into the environment
    if (CALL_UTIL (putenv)(newstr) != 0)
      {
	TprintfT (DBG_LT2, "putenv_prepend(): ERROR %s is not set!\n", newstr);
	return 1;
      }
  return 0;
}

/*
   function: env_strip()
     Finds substr in origstr; Removes
     all characters from previous ':' or ' '
     up to and including any trailing ':' or ' '.
   params:
     env: environment variable contents
     str: substring to find
     return: count of instances removed from env
 */
static int
env_strip (char *origstr, const char *substr)
{
  int removed = 0;
  char *p, *q;
  if (origstr == NULL || substr == NULL || *substr == 0)
    return 0;
  while ((p = q = CALL_UTIL (strstr)(origstr, substr)))
    {
      p += __collector_strlen (substr);
      while (*p == ':' || *p == ' ') /* strip trailing separator */
	p++;
      while (*q != ':' && *q != ' ' && *q != '=' && q != origstr) /* strip path */
	q--;
      if (q != origstr) /* restore leading separator (if any) */
	q++;
      __collector_strlcpy (q, p, __collector_strlen (p) + 1);
      removed++;
    }
  return removed;
}

/*
   function: env_ld_preload_strip()
     Removes known libcollector shared objects from envv.
   params:
     var: shared object name (leading characters don't have to match)
     return: 0 = so's removed, non-zero = so's not found.
 */
static int
env_ld_preload_strip (char *envv)
{
  if (!envv || *envv == 0)
    {
      TprintfT (DBG_LT2, "env_ld_preload_strip(): WARNING - envv is NULL\n");
      return -1;
    }
  for (int v = 0; SP_PRELOAD[v]; v++)
    if (env_strip (envv, sp_preloads[v]))
      return 0;
  if (line_mode != LM_CLOSED)
    TprintfT (DBG_LT2, "env_ld_preload_strip(): WARNING - could not strip SP_PRELOADS from '%s'\n",
	      envv);
  return -2;
}

void
__collector_env_print (char * label)
{
#if DEBUG
  TprintfT (DBG_LT2, "__collector_env_print(%s)\n", label);
  for (int v = 0; v < MAX_LD_PRELOADS; v++)
    TprintfT (DBG_LT2, " %s  sp_preloads[%d] (0x%p)=%s\n", label,
	      v, sp_preloads[v], (sp_preloads[v] == NULL ? "NULL" : sp_preloads[v]));
  for (int v = 0; SP_ENV[v]; v++)
    {
      char *s = CALL_UTIL (getenv)(SP_ENV[v]);
      if (s == NULL)
	s = "<null>";
      TprintfT (DBG_LT2, " %s  SP_ENV[%d] (0x%p): %s=\"%s\"\n", label, v, SP_ENV[v], SP_ENV[v], s);
    }
  for (int v = 0; LD_ENV[v]; v++)
    {
      char *s = CALL_UTIL (getenv)(LD_ENV[v]);
      if (s == NULL)
	s = "<null>";
      TprintfT (DBG_LT2, " %s  LD_ENV[%d] (0x%p): %s=\"%s\"\n", label, v, LD_ENV[v], LD_ENV[v], s);
    }
#endif
}

void
__collector_env_printall (char *label, char *envp[])
{
#if DEBUG
  TprintfT (DBG_LT2, "__collector_env_printall(%s): environment @ 0x%p\n", label, envp);
  for (int i = 0; envp[i]; i++)
    Tprintf (DBG_LT2, "\tenv[%d]@0x%p == %s\n", i, envp[i], envp[i]);
#endif
}

/* match collector environment variable */
int
env_match (char *envp[], const char *envvar)
{
  int match = -1;
  if (envp == NULL)
    TprintfT (DBG_LT1, "env_match(%s): NULL envp!\n", envvar);
  else
    {
      int i = 0;
      while ((envp[i] != NULL) && (__collector_strStartWith (envp[i], envvar)))
	i++;
      if ((envp[i] == NULL) || (envp[i][__collector_strlen (envvar)] != '='))
	TprintfT (DBG_LT4, "env_match(): @%p []%s not defined in envp\n", envp, envvar);
      else
	{
	  TprintfT (DBG_LT4, "env_match(): @%p [%d]%s defined in envp\n", envp, i, envp[i]);
	  match = i;
	}
    }
  TprintfT (DBG_LT1, "env_match(%s): found in slot %d\n", envvar, match);
  return (match);
}

/* allocate new environment with collector variables */
/* 1) copy all current envp[] ptrs into a new array, coll_env[] */
/* 2) if collector-related env ptrs not in envp[], append them to coll_env */
/*     from processes' "environ" (allocate_env==1) */
/*     or from sp_env_backup (allocate_env==0)*/
/*     If they already exist in envp, probably is an error... */
/* 3) return coll_env */

/* __collector__env_update() need be called after this to set LD_ENV*/
char **
__collector_env_allocate (char *const old_env[], int allocate_env)
{
  extern char **environ;    /* the process' actual environment */
  char **new_env;           /* a new environment for collection */
  TprintfT (DBG_LT3, "__collector_env_allocate(old_env=0x%p %s environ=0x%p)\n",
	    old_env, (old_env == environ) ? "==" : "!=", environ);
  /* set up a copy of the provided old_env for collector use */
  int old_env_size = 0;

  /* determine number of (used) slots in old_env */
  if (old_env)
    while (old_env[old_env_size] != NULL)
      old_env_size++;
  /* allocate a new vector with additional slots */
  int new_env_alloc_sz = old_env_size + NUM_SP_ENV_VARS + NUM_LD_ENV_VARS + 1;
  new_env = (char**) __collector_allocCSize (__collector_heap, new_env_alloc_sz * sizeof (char*), 1);
  if (new_env == NULL)
    return NULL;
  TprintfT (DBG_LT4, "__collector_env_allocate(): old_env has %d entries, new_env @ 0x%p\n", old_env_size, new_env);

  /* copy provided old_env pointers to new collector environment */
  int new_env_size = 0;
  for (new_env_size = 0; new_env_size < old_env_size; new_env_size++)
    new_env[new_env_size] = old_env[new_env_size];

  /* check each required environment variable, adding as required */
  const char * env_var;
  int v;
  for (v = 0; (env_var = SP_ENV[v]) != NULL; v++)
    {
      if (env_match ((char**) old_env, env_var) == -1)
	{
	  int idx;
	  /* not found in old_env */
	  if (allocate_env)
	    {
	      if ((idx = env_match (environ, env_var)) != -1)
		{
		  /* found in environ */
		  TprintfT (DBG_LT4, "__collector_env_allocate(): [%d]%s env restored!\n",
			    new_env_size, environ[idx]);
		  int varsz = __collector_strlen (environ[idx]) + 1;
		  char * var = (char*) __collector_allocCSize (__collector_heap, varsz, 1);
		  if (var == NULL)
		    return NULL;
		  __collector_strlcpy (var, environ[idx], varsz);
		  new_env[new_env_size++] = var;
		}
	      else
		{
		  /* not found in environ */
		  if ((__collector_strcmp (env_var, SP_COLLECTOR_PARAMS) == 0) ||
		      (__collector_strcmp (env_var, SP_COLLECTOR_EXPNAME) == 0))
		    TprintfT (DBG_LT1, "__collector_env_allocate(): note: %s environment variable not found\n",
			      env_var);
		}
	    }
	  else
	    {
	      if ((idx = env_match (sp_env_backup, env_var)) != -1)
		{
		  /* found in backup */
		  TprintfT (DBG_LT4, "__collector_env_allocate(): [%d]%s env restored!\n",
			    new_env_size, sp_env_backup[idx]);
		  new_env[new_env_size++] = sp_env_backup[idx];
		}
	      else
		{
		  /* not found in environ */
		  if ((__collector_strcmp (env_var, SP_COLLECTOR_PARAMS) == 0) ||
		      (__collector_strcmp (env_var, SP_COLLECTOR_EXPNAME) == 0))
		    TprintfT (DBG_LT1, "__collector_env_allocate(): note: %s environment variable not found\n",
				env_var);
		}
	    }
	}
    }

  for (v = 0; (env_var = LD_ENV[v]) != NULL; v++)
    {
      if (env_match ((char**) old_env, env_var) == -1)
	{
	  int idx;
	  /* not found in old_env */
	  if (allocate_env)
	    {
	      if ((idx = env_match (environ, env_var)) != -1)
		{
		  /* found in environ */
		  TprintfT (DBG_LT4, "__collector_env_allocate(): [%d]%s env restored!\n",
			    new_env_size, environ[idx]);

		  int varsz = __collector_strlen (env_var) + 2;
		  char * var = (char*) __collector_allocCSize (__collector_heap, varsz, 1);
		  if (var == NULL)
		    return NULL;
		  // assume __collector_env_update() will fill content of env_var
		  CALL_UTIL (snprintf)(var, varsz, "%s=", env_var);
		  new_env[new_env_size++] = var;
		}
	    }
	  else
	    {
	      if ((idx = env_match (sp_env_backup, env_var)) != -1)
		{
		  /* found in backup */
		  TprintfT (DBG_LT4, "__collector_env_allocate(): [%d]%s env restored!\n",
			    new_env_size, sp_env_backup[idx]);
		  new_env[new_env_size++] = sp_env_backup[idx];
		}
	    }
	}
    }

  /* ensure new_env vector ends with NULL */
  new_env[new_env_size] = NULL;
  assert (new_env_size <= new_env_alloc_sz);
  TprintfT (DBG_LT4, "__collector_env_allocate(): new_env has %d entries (%d added), new_env=0x%p\n",
	    new_env_size, new_env_size - old_env_size, new_env);
  if (new_env_size != old_env_size && !allocate_env)
    __collector_log_write ("<event kind=\"%s\" id=\"%d\">%d</event>\n",
			   SP_JCMD_CWARN, COL_WARN_EXECENV, new_env_size - old_env_size);
  __collector_env_printall ("__collector_env_allocate", new_env);
  return (new_env);
}

/* unset collection environment variables */
/* if they exist in env... */
/* 1) push non-collectorized version to env */

/* Not mt safe */
void
__collector_env_unset (char *envp[])
{
  int v;
  const char * env_name;
  TprintfT (DBG_LT3, "env_unset(envp=0x%p)\n", envp);
  if (envp == NULL)
    {
      for (v = 0; (env_name = LD_PRELOAD[v]); v++)
	{
	  const char *env_val = CALL_UTIL (getenv)(env_name);
	  if (env_val && CALL_UTIL (strstr)(env_val, sp_preloads[v]))
	    {
	      size_t sz = __collector_strlen (env_name) + 1 + __collector_strlen (env_val) + 1;
	      char * ev = (char*) __collector_allocCSize (__collector_heap, sz, 1);
	      if (ev == NULL)
		return;
	      CALL_UTIL (snprintf)(ev, sz, "%s=%s", env_name, env_val);
	      assert (__collector_strlen (ev) + 1 == sz);
	      TprintfT (DBG_LT4, "env_unset(): old %s\n", ev);
	      env_ld_preload_strip (ev);
	      CALL_UTIL (putenv)(ev);
	      TprintfT (DBG_LT4, "env_unset(): new %s\n", ev);
	    }
	}
      // unset JAVA_TOOL_OPTIONS
      env_name = JAVA_TOOL_OPTIONS;
      const char * env_val = CALL_UTIL (getenv)(env_name);
      if (env_val && CALL_UTIL (strstr)(env_val, COLLECTOR_JVMTI_OPTION))
	{
	  size_t sz = __collector_strlen (env_name) + 1 + __collector_strlen (env_val) + 1;
	  char * ev = (char*) __collector_allocCSize (__collector_heap, sz, 1);
	  if (ev == NULL)
	    return;
	  CALL_UTIL (snprintf)(ev, sz, "%s=%s", env_name, env_val);
	  assert (__collector_strlen (ev) + 1 == sz);
	  TprintfT (DBG_LT4, "env_unset(): old %s\n", ev);
	  env_strip (ev, COLLECTOR_JVMTI_OPTION);
	  CALL_UTIL (putenv)(ev);
	  TprintfT (DBG_LT4, "env_unset(): new %s\n", ev);
	}
      __collector_env_print ("__collector_env_unset");
    }
  else
    {
      __collector_env_printall ("__collector_env_unset, before", envp);
      for (v = 0; (env_name = LD_PRELOAD[v]); v++)
	{
	  int idx = env_match (envp, env_name);
	  if (idx != -1)
	    {
	      char *env_val = envp[idx];
	      TprintfT (DBG_LT4, "env_unset(): old %s\n", env_val);
	      envp[idx] = "junk="; /* xxxx is it ok to use original string? */
	      env_ld_preload_strip (env_val);
	      envp[idx] = env_val;
	      TprintfT (DBG_LT4, "env_unset(): new %s\n", envp[idx]);
	    }
	}
	// unset JAVA_TOOL_OPTIONS
	env_name = JAVA_TOOL_OPTIONS;
	int idx = env_match(envp, env_name);
	if (idx != -1) {
	    char *env_val = envp[idx];
	    TprintfT(DBG_LT4, "env_unset(): old %s\n", env_val);
	    envp[idx] = "junk="; /* xxxx is it ok to use original string? */
	    env_strip(env_val, COLLECTOR_JVMTI_OPTION);
	    envp[idx] = env_val;
	    TprintfT(DBG_LT4, "env_unset(): new %s\n", envp[idx]);
	}
	__collector_env_printall ("__collector_env_unset, after", envp );
    }
}

/* update collection environment variables */
/* update LD_PRELOADs and push them */
/* not mt safe */
void
__collector_env_update (char *envp[])
{
  const char *env_name;
  TprintfT (DBG_LT1, "__collector_env_update(envp=0x%p)\n", envp);
  extern char **environ;
  if (envp == NULL)
    {
      int v;
      TprintfT (DBG_LT2, "__collector_env_update(envp=NULL)\n");
      __collector_env_printall ("  environ array, before", environ);
      __collector_env_print ("  env_update at entry ");

      /* SP_ENV */
      for (v = 0; (env_name = SP_ENV[v]) != NULL; v++)
	{
	  if (env_match (environ, env_name) == -1)
	    {
	      int idx;
	      if ((idx = env_match (sp_env_backup, env_name)) != -1)
		{
		  unsigned strsz = __collector_strlen (sp_env_backup[idx]) + 1;
		  char *ev = (char*) __collector_allocCSize (__collector_heap, strsz, 1);
		  CALL_UTIL (snprintf)(ev, strsz, "%s", sp_env_backup[idx]);
		  if (CALL_UTIL (putenv)(ev) != 0)
		    TprintfT (DBG_LT2, "__collector_env_update(): ERROR %s is not set!\n",
				sp_env_backup[idx]);
		}
	    }
	}
      __collector_env_print ("  env_update after SP_ENV settings ");

      /* LD_LIBRARY_PATH */
      for (v = 0; (env_name = LD_LIBRARY_PATH[v]); v++)
	/* assumes same index used between LD and SP vars */
	if (putenv_prepend (env_name, sp_libpaths[v], ":"))
	  TprintfT (DBG_LT2, "collector: ERROR %s=%s could not be set\n",
		    env_name, sp_libpaths[v]);
      __collector_env_print ("  env_update after LD_LIBRARY_PATH settings ");

      /* LD_PRELOAD */
      for (v = 0; (env_name = LD_PRELOAD[v]); v++)
	/* assumes same index used between LD and SP vars */
	if (putenv_prepend (env_name, sp_preloads[v], " "))
	  TprintfT (DBG_LT2, "collector: ERROR %s=%s could not be set\n",
		    env_name, sp_preloads[v]);
      __collector_env_print ("  env_update after LD_PRELOAD settings ");

      /* JAVA_TOOL_OPTIONS */
      if (java_mode)
	if (putenv_prepend (JAVA_TOOL_OPTIONS, COLLECTOR_JVMTI_OPTION, " "))
	  TprintfT (DBG_LT2, "collector: ERROR %s=%s could not be set\n",
		    JAVA_TOOL_OPTIONS, COLLECTOR_JVMTI_OPTION);
      __collector_env_print ("  env_update after JAVA_TOOL settings ");
    }
  else
    {
      int v;
      int idx;
      TprintfT (DBG_LT2, "__collector_env_update(envp=0x%p) not NULL\n", envp);
      __collector_env_printall ("__collector_env_update, before", envp);
      /* LD_LIBRARY_PATH */
      for (v = 0; (env_name = LD_LIBRARY_PATH[v]); v++)
	{
	  int idx = env_match (envp, env_name);
	  if (idx != -1)
	    {
	      char *env_val = __collector_strchr (envp[idx], '=');
	      if (env_val)
		env_val++; /* skip '=' */
	      /* assumes same index used between LD and SP vars */
	      char *new_str = env_prepend (env_name, sp_libpaths[v],
					   ":", env_val);
	      if (new_str)
		envp[idx] = new_str;
	    }
	}

      /* LD_PRELOAD */
      for (v = 0; (env_name = LD_PRELOAD[v]); v++)
	{
	  int idx = env_match (envp, env_name);
	  if (idx != -1)
	    {
	      char *env_val = __collector_strchr (envp[idx], '=');
	      if (env_val)
		env_val++; /* skip '=' */
	      /* assumes same index used between LD and SP vars */
	      char *new_str = env_prepend (env_name, sp_preloads[v],
					   " ", env_val);
	      if (new_str)
		envp[idx] = new_str;
	    }
	}

      /* JAVA_TOOL_OPTIONS */
      if (java_mode)
	{
	  env_name = JAVA_TOOL_OPTIONS;
	  idx = env_match (envp, env_name);
	  if (idx != -1)
	    {
	      char *env_val = __collector_strchr (envp[idx], '=');
	      if (env_val)
		env_val++; /* skip '=' */
	      char *new_str = env_prepend (env_name, COLLECTOR_JVMTI_OPTION,
					   " ", env_val);
	      if (new_str)
		envp[idx] = new_str;
	    }
	}
    }
  __collector_env_printall ("__collector_env_update, after", environ);
}


/*------------------------------------------------------------- putenv */
int putenv () __attribute__ ((weak, alias ("__collector_putenv")));
int _putenv () __attribute__ ((weak, alias ("__collector_putenv")));

int
__collector_putenv (char * string)
{
  if (CALL_UTIL (putenv) == __collector_putenv ||
      CALL_UTIL (putenv) == NULL)
    { // __collector_libc_funcs_init failed
      CALL_UTIL (putenv) = (int(*)())dlsym (RTLD_NEXT, "putenv");
      if (CALL_UTIL (putenv) == NULL || CALL_UTIL (putenv) == __collector_putenv)
	  CALL_UTIL (putenv) = (int(*)())dlsym (RTLD_DEFAULT, "putenv");
      if (CALL_UTIL (putenv) == NULL || CALL_UTIL (putenv) == __collector_putenv)
	{
	  TprintfT (DBG_LT2, "__collector_putenv(): ERROR: no pointer found.\n");
	  errno = EBUSY;
	  return -1;
	}
    }
  if (user_follow_mode == FOLLOW_NONE)
    return CALL_UTIL (putenv)(string);
  char * envp[] = {string, NULL};
  __collector_env_update (envp);
  return CALL_UTIL (putenv)(envp[0]);
}

/*------------------------------------------------------------- setenv */
int setenv () __attribute__ ((weak, alias ("__collector_setenv")));
int _setenv () __attribute__ ((weak, alias ("__collector_setenv")));

int
__collector_setenv (const char *name, const char *value, int overwrite)
{
  if (CALL_UTIL (setenv) == __collector_setenv ||
      CALL_UTIL (setenv) == NULL)
    { // __collector_libc_funcs_init failed
      CALL_UTIL (setenv) = (int(*)())dlsym (RTLD_NEXT, "setenv");
      if (CALL_UTIL (setenv) == NULL || CALL_UTIL (setenv) == __collector_setenv)
	CALL_UTIL (setenv) = (int(*)())dlsym (RTLD_DEFAULT, "setenv");
      if (CALL_UTIL (setenv) == NULL || CALL_UTIL (setenv) == __collector_setenv)
	{
	  TprintfT (DBG_LT2, "__collector_setenv(): ERROR: no pointer found.\n");
	  errno = EBUSY;
	  return -1;
	}
    }
  if (user_follow_mode == FOLLOW_NONE || !overwrite)
    return CALL_UTIL (setenv)(name, value, overwrite);
  size_t sz = __collector_strlen (name) + 1 + __collector_strlen (value) + 1;
  char *ev = (char*) __collector_allocCSize (__collector_heap, sz, 1);
  if (ev == NULL)
    return CALL_UTIL (setenv)(name, value, overwrite);
  CALL_UTIL (snprintf)(ev, sz, "%s=%s", name, value);
  char * envp[] = {ev, NULL};
  __collector_env_update (envp);
  if (envp[0] == ev)
    {
      __collector_freeCSize (__collector_heap, ev, sz);
      return CALL_UTIL (setenv)(name, value, overwrite);
    }
  else
    {
      char *env_val = __collector_strchr (envp[0], '=');
      if (env_val)
	{
	  *env_val = '\0';
	  env_val++; /* skip '=' */
	}
      return CALL_UTIL (setenv)(envp[0], env_val, overwrite);
    }
}

/*------------------------------------------------------------- unsetenv */
int unsetenv () __attribute__ ((weak, alias ("__collector_unsetenv")));
int _unsetenv () __attribute__ ((weak, alias ("__collector_unsetenv")));

int
__collector_unsetenv (const char *name)
{
  if (CALL_UTIL (unsetenv) == __collector_unsetenv ||
      CALL_UTIL (unsetenv) == NULL)
    { // __collector_libc_funcs_init failed
      CALL_UTIL (unsetenv) = (int(*)())dlsym (RTLD_NEXT, "unsetenv");
      if (CALL_UTIL (unsetenv) == NULL || CALL_UTIL (unsetenv) == __collector_unsetenv)
	CALL_UTIL (unsetenv) = (int(*)())dlsym (RTLD_DEFAULT, "unsetenv");
      if (CALL_UTIL (unsetenv) == NULL || CALL_UTIL (unsetenv) == __collector_unsetenv)
	{
	  TprintfT (DBG_LT2, "__collector_unsetenv(): ERROR: no pointer found.\n");
	  errno = EBUSY;
	  return -1;
	}
    }
  int ret = CALL_UTIL (unsetenv)(name);
  if (user_follow_mode == FOLLOW_NONE)
    return ret;
  TprintfT (DBG_LT2, "__collector_unsetenv(): %d.\n", user_follow_mode);
  size_t sz = __collector_strlen (name) + 1 + 1;
  char *ev = (char*) __collector_allocCSize (__collector_heap, sz, 1);
  if (ev == NULL)
    return ret;
  CALL_UTIL (snprintf)(ev, sz, "%s=", name);
  char * envp[] = {ev, NULL};
  __collector_env_update (envp);
  if (envp[0] == ev)
    __collector_freeCSize (__collector_heap, ev, sz);
  else
    CALL_UTIL (putenv)(envp[0]);
  return ret;
}

/*------------------------------------------------------------- clearenv */
int clearenv () __attribute__ ((weak, alias ("__collector_clearenv")));

int
__collector_clearenv (void)
{
  if (CALL_UTIL (clearenv) == __collector_clearenv || CALL_UTIL (clearenv) == NULL)
    {
      /* __collector_libc_funcs_init failed; look up clearenv now */
      CALL_UTIL (clearenv) = (int(*)())dlsym (RTLD_NEXT, "clearenv");
      if (CALL_UTIL (clearenv) == NULL || CALL_UTIL (clearenv) == __collector_clearenv)
	/* still not found; try again */
	CALL_UTIL (clearenv) = (int(*)())dlsym (RTLD_DEFAULT, "clearenv");
      if (CALL_UTIL (clearenv) == NULL || CALL_UTIL (clearenv) == __collector_clearenv)
	{
	  /* still not found -- a fatal error */
	  TprintfT (DBG_LT2, "__collector_clearenv(): ERROR: %s\n", dlerror ());
	  CALL_UTIL (fprintf)(stderr, "__collector_clearenv(): ERROR: %s\n", dlerror ());
	  errno = EBUSY;
	  return -1;
	}
    }
  int ret = CALL_UTIL (clearenv)();
  if (user_follow_mode == FOLLOW_NONE)
    return ret;
  if (sp_env_backup == NULL)
    {
      TprintfT (DBG_LT2, "__collector_clearenv: ERROR sp_env_backup is not set!\n");
      return ret;
    }
  for (int v = 0; v < NUM_SP_ENV_VARS + NUM_LD_ENV_VARS; v++)
    if (sp_env_backup[v] && CALL_UTIL (putenv)(sp_env_backup[v]) != 0)
      TprintfT (DBG_LT2, "__collector_clearenv: ERROR %s is not set!\n",
		sp_env_backup[v]);
  return ret;
}
