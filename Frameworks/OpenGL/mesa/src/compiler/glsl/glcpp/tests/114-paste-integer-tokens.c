#define PASTE3(a,b,c) a ## b ## c
#define PASTE4(a,b,c,d) a ## b ## c ## d
#define PASTE5(a,b,c,d,e) a ## b ## c ## d ## e
4. HTTP code for Not Found: PASTE3(__LINE__, __FILE__ , __LINE__)
5. Hexadecimal for 20560: PASTE4(__LINE__, __FILE__, __LINE__, __FILE__)
6: Zip code for Nortonville, KS: PASTE5(__LINE__, __LINE__, __FILE__, __LINE__,  __FILE__)
7. James Bond, as a number: PASTE3(__FILE__, __FILE__, __LINE__)
