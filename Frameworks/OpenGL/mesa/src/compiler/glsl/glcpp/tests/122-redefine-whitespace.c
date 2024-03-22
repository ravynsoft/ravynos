/* Original definitions. */
#define TWO  ( 1+1 )
#define FOUR (2 + 2)
#define SIX  (3 + 3)
#define EIGHT (8 + 8)

/* Redefinitions with whitespace in same places, but different amounts, (so no
 * error). */
#define TWO	(	1+1   )
#define FOUR    (2	+  2)
#define SIX	(3/*comment is whitespace*/+   /* collapsed */ /* to */ /* one */ /* space */  3)

/* Trailing whitespace (no error) */
#define EIGHT (8 + 8)       

/* Redefinitions with whitespace in different places. Each of these should
 * trigger an error. */
#define TWO  (1 + 1)
#define FOUR ( 2+2 )
#define SIX  (/*not*/3 + 3/*expected*/)
