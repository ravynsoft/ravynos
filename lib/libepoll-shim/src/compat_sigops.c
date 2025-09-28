#include "compat_sigops.h"

#include <string.h>

#if defined(__OpenBSD__) || defined(__APPLE__)
int
compat_sigisemptyset(sigset_t const *set)
{
	return *set == 0;
}

int
compat_sigandset(sigset_t *dest, sigset_t const *left, sigset_t const *right)
{
	*dest = (*left) & (*right);
	return 0;
}
#elif defined(__NetBSD__)
int
compat_sigisemptyset(sigset_t const *set)
{
	sigset_t e;
	__sigemptyset(&e);
	return __sigsetequal(set, &e);
}

int
compat_sigandset(sigset_t *dest, sigset_t const *left, sigset_t const *right)
{
	memcpy(dest, left, sizeof(sigset_t));
	__sigandset(right, dest);
	return 0;
}
#elif defined(__DragonFly__) || defined(__FreeBSD__)
int
compat_sigisemptyset(sigset_t const *set)
{
	for (int i = 0; i < _SIG_WORDS; ++i) {
		if (set->__bits[i] != 0) {
			return 0;
		}
	}
	return 1;
}

int
compat_sigandset(sigset_t *dest, sigset_t const *left, sigset_t const *right)
{
	for (int i = 0; i < _SIG_WORDS; ++i) {
		dest->__bits[i] = left->__bits[i] & right->__bits[i];
	}
	return 0;
}
#endif
