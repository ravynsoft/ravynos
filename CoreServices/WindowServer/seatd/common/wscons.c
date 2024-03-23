#include <stdlib.h>
#include <string.h>

#if defined(__NetBSD__)
#include <stdlib.h>
#include <sys/stat.h>
#endif

#include "wscons.h"

#define STRLEN(s) ((sizeof(s) / sizeof(s[0])) - 1)

#if defined(__NetBSD__)
int path_is_wscons(const char *path) {
	static const char wskbd[] = "/dev/wskbd";
	static const char wsmouse[] = "/dev/wsmouse";
	static const char wsmux[] = "/dev/wsmux";
	return strncmp(path, wskbd, STRLEN(wskbd)) == 0 ||
	       strncmp(path, wsmouse, STRLEN(wsmouse)) == 0 ||
	       strncmp(path, wsmux, STRLEN(wsmouse)) == 0;
}
#else
int path_is_wscons(const char *path) {
	(void)path;
	return 0;
}
#endif
