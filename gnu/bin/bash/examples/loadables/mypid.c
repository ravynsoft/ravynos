/* This module should be dynamically loaded with enable -f
 * which would create a new builtin named mypid. You'll need
 * the source code for GNU bash to recompile this module.
 *
 * Then, from within bash, enable -f ./mypid enable_mypid, where ./mypid
 * is the binary obtained from running make. Hereafter, `${MYPID}'
 * is a shell builtin variable.
 *
 * This defines an unload hook function that is called when the builtin is
 * deleted with enable -d that will unbind the MYPID variable so future
 * references to it do not attempt to access memory that is no longer part
 * of this process's address space.
 */
#include <config.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "builtins.h"
#include "shell.h"

#define INIT_DYNAMIC_VAR(var, val, gfunc, afunc) \
  do \
    { SHELL_VAR *v = bind_variable (var, (val), 0); \
      if (v) \
	{ \
	  v->dynamic_value = gfunc; \
	  v->assign_func = afunc; \
	} \
    } \
  while (0)

static SHELL_VAR *
assign_mypid (
     SHELL_VAR *self,
     char *value,
     arrayind_t unused,
     char *key )
{
  return (self);
}

static SHELL_VAR *
get_mypid (SHELL_VAR *var)
{
  int rv;
  char *p;

  rv = getpid();
  p = itos (rv);

  FREE (value_cell (var));

  VSETATTR (var, att_integer);
  var_setvalue (var, p);
  return (var);
}

int
enable_mypid_builtin(WORD_LIST *list)
{
  INIT_DYNAMIC_VAR ("MYPID", (char *)NULL, get_mypid, assign_mypid);

  return 0;
}

void
enable_mypid_builtin_unload (char *s)
{
  unbind_variable ("MYPID");
}

char const *enable_mypid_doc[] = {
  "Enable $MYPID.",
  "",
  "Enables use of the ${MYPID} dynamic variable.  ",
  "It will yield the current pid of a subshell.",
  (char *)0
};

struct builtin enable_mypid_struct = {
  "enable_mypid",
  enable_mypid_builtin,
  BUILTIN_ENABLED,
  (char**)(void*)enable_mypid_doc,
  "enable_mypid N",
  0
};
