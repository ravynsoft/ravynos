//
//  systemx.c
//  hfs
//
//  Created by Chris Suter on 8/12/15.
//
//

#include <stdarg.h>
#include <libgen.h>
#include <spawn.h>
#include <fcntl.h>
#include <unistd.h>

#include "systemx.h"
#include "test-utils.h"

int __attribute__((sentinel)) systemx(const char *prog, ...)
{
	const char *args[64];

	va_list ap;

	const char **parg = args;

	*parg++ = basename((char *)prog);

	va_start(ap, prog);

	bool quiet = false;

	while ((*parg = va_arg(ap, char *))) {
		if (*parg == SYSTEMX_QUIET) {
			quiet = true;
		} else
			++parg;
	}

	va_end(ap);

	posix_spawn_file_actions_t facts, *pfacts = NULL;

	if (quiet) {
		posix_spawn_file_actions_init(&facts);
		posix_spawn_file_actions_addopen(&facts, STDOUT_FILENO, "/dev/null", O_APPEND, 0);
		pfacts = &facts;
	}

	pid_t pid;
	assert_no_err(posix_spawn(&pid, prog, pfacts, NULL, (char * const *)args, NULL));

	if (pfacts)
		posix_spawn_file_actions_destroy(pfacts);

	int status;
	assert(ignore_eintr(waitpid(pid, &status, 0), -1) == pid);

	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	else
		return -1;
}
