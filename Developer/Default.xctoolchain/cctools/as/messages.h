#ifndef MESSAGES_H_
#define MESSAGES_H_

#include "as.h"
#include "expr.h"
#include "struc-symbol.h"

extern int bad_error;
extern int arch_multiple;

extern void as_warn(
    const char *format,
     ...) __attribute__ ((format (printf, 1, 2)));

extern void as_warn_where(
    char *file,
	unsigned int line,
	const char *format,
	...) __attribute__ ((format (printf, 3, 4)));

extern void as_warn_where_with_column(
     char *file,
	unsigned int line,
	unsigned int column,
	const char *format,
	...)  __attribute__ ((format (printf, 4, 5)));

extern void as_bad(
    const char *format,
     ...) __attribute__ ((format (printf, 1, 2)));

extern void as_fatal(
    const char *format,
     ...) __attribute__ ((format (printf, 1, 2)));

extern void sprint_value(
	char *, signed_expr_t);

#endif /* MESSAGES_H_ */
