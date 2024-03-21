#ifndef COMPAT_SIGOPS_H
#define COMPAT_SIGOPS_H

#include <signal.h>

int compat_sigisemptyset(sigset_t const *set);
int compat_sigorset(sigset_t *dest, sigset_t const *left,
    sigset_t const *right);
int compat_sigandset(sigset_t *dest, sigset_t const *left,
    sigset_t const *right);

#ifdef COMPAT_ENABLE_SIGOPS
#define sigisemptyset compat_sigisemptyset
#define sigorset compat_sigorset
#define sigandset compat_sigandset
#endif

#endif
