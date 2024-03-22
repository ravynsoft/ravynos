/* The body can include C expressions with ++ and -- */
a = x++;
b = ++x;
c = x--;
d = --x;
/* But these are not legal in preprocessor expressions. */
#if x++ > 4
#endif
