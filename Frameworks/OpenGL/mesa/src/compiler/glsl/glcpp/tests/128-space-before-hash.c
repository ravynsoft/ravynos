 /* Any directive can be preceded by a space. */
 #version 300
 #pragma Testing spaces before hash
 #
 #line 3
 #define FOO
 #ifdef FOO
 yes
 #endif
 #if 0
 #elif defined FOO
 yes again
 #endif
 #if 0
 #else
 for the third time, yes!
 #endif
 #undef FOO
 #ifndef FOO
 yes, of course
 #endif
