#ifndef _TEST_H
#define _TEST_H

char *__curtestname = "<none>";

#define test_run(func)                      \
	do {                                \
		char *orig = __curtestname; \
		__curtestname = #func;      \
		func();                     \
		__curtestname = orig;       \
	} while (0)

#define test_assert(cond)                                                                          \
	do {                                                                                       \
		if (!(cond)) {                                                                     \
			fprintf(stderr, "%s:%d: %s: test_assert failed: %s\n", __FILE__, __LINE__, \
				__curtestname, #cond);                                             \
			abort();                                                                   \
		}                                                                                  \
	} while (0)

#endif
