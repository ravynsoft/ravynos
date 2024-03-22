#line 0
#error line 0 error
#line 25
#error line 25 error
#line 0 1
#error source 1, line 0 error
#line 30 2
#error source 2, line 30 error
#line 45 2 /* A line with a comment */
#define NINETY 90
#define TWO 2
#line NINETY TWO /* A #line line with macro expansion */
#define FUNCTION_LIKE_MACRO(source, line) source line
#line FUNCTION_LIKE_MACRO(180,2)
