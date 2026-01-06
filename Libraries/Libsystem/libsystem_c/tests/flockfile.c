#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "darwintest.h"

T_DECL(flockfile_preserve_errno, "flockfile preserves errno")
{
	errno = EBUSY;
	flockfile(stderr);
	T_ASSERT_EQ(errno, EBUSY, "flockfile preserves errno");
}

T_DECL(funlockfile_preserve_errno, "funlockfile preserves errno")
{
	errno = EBUSY;
	funlockfile(stderr);
	T_ASSERT_EQ(errno, EBUSY, "funlockfile preserves errno");
}

T_DECL(ftrylockfile_preserve_errno, "ftrylockfile preserves errno")
{
	errno = EBUSY;
	ftrylockfile(stderr);
	T_ASSERT_EQ(errno, EBUSY, "ftrylockfile preserves errno");
}

