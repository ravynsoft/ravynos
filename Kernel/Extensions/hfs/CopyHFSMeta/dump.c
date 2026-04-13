#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#define MIN(a, b) \
	({ __typeof(a) _a = (a); __typeof(b) _b = (b);	\
		(_a < _b) ? _a : _b; })

enum { WIDTH = 16, };

/*
 * Debug functions only.
 */
void
DumpData(const void *data, size_t len)
{
	unsigned char *base = (unsigned char*)data;
	unsigned char *end = base + len;
	unsigned char *cp = base;
	int allzeroes = 0;

	while (cp < end) {
		unsigned char *tend = MIN(end, cp + WIDTH);
		unsigned char *tmp;
		int i;
		size_t gap = (cp + WIDTH) - tend;

		if (gap != 0 || tend == end)
			allzeroes = 0;
		if (allzeroes) {
			for (tmp = cp; tmp < tend; tmp++) {
				if (*tmp) {
					allzeroes = 0;
					break;
				}
			}
			if (allzeroes == 1) {
				printf(". . .\n");
				allzeroes = 2;
			}
			if (allzeroes) {
				cp += WIDTH;
				continue;
			}
		}
		allzeroes = 1;

		printf("%04x:  ", (int)(cp - base));
		for (i = 0, tmp = cp; tmp < tend; tmp++) {
			printf("%02x", *tmp);
			if (++i % 2 == 0)
				printf(" ");
			if (*tmp)
				allzeroes = 0;
		}
		for (i = (int)gap; i >= 0; i--) {
			printf("  ");
			if (i % 2 == 1)
				printf(" ");
		}
		printf("    |");
		for (tmp = cp; tmp < tend; tmp++) {
			printf("%c", isalnum(*tmp) ? *tmp : '.');
		}
		for (i = 0; i < gap; i++) {
			printf(" ");
		}
		printf("|\n");
		cp += WIDTH;
	}

	return;

}
