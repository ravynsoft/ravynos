#include_next <dirent.h>

#ifndef MAXNAMLEN
#define MAXNAMLEN NAME_MAX /* DragonFlyBSD and Android */
#endif
