#ifndef STRINGESCAPE_H
#define STRINGESCAPE_H

#include <stddef.h>

size_t   escape(char* in, char *out, size_t outsize);
size_t unescape(char* in, char *out, size_t outsize);

#endif
