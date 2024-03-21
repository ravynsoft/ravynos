#ifndef COMPAT_PIPE2_H
#define COMPAT_PIPE2_H

#include <fcntl.h>
#include <unistd.h>

int compat_pipe2(int pipefd[2], int flags);
#ifdef COMPAT_ENABLE_PIPE2
#define pipe2 compat_pipe2
#endif

#endif
