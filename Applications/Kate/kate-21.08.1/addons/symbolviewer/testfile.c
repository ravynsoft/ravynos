/** Test File...very demoniac for parsing... **/
#include <stdio.h>
#include <stdlib.h>

#ifdef (_cplusplus)
{
#define VALUE 5

#define MACRO(x)    (x^2)

#    define abs_float(x)            \
          ( ((x)<0) ? -(x) : (x) )
              

typedef struct
   {
    pTest *pNext;
    pTest *pPrev;
   } 
   Another_test, *pTest;

typedef struct xauth
{
  unsigned short family;
  char *address;
} Xauth;

typedef struct {
    color    to_move;
    occupant board[8][8];
   } game;

typedef game  gt_data;
    
/* 
 A comment with a function hello() { } 
*/
// Continued...

RockType *
     MyMusicFunction(
          void *Red,
          int Hot, // Comment double slash
          char Chili, /* Comment inline */
          unsigned long Peppers)
// A comment..just to make some noise...
{
 // Passed first stage ???
 // Ok..get ready for the second one !
 if(I_Have_Failed() >= 0 && /* comments everywhere :} */
    This_Appears() == 1)
     {
      printf(QLatin1String("Damn !!! Better going to bed :(("));
      if ( vs ) // here you are ;}
     activateSpace( vs->currentView() );
     } 
}

// Test escaped quote
void test() { printf("foo \"\n"); }
void test2() { printf("foo \"\n"); }

// Test tabs in macro
#define MY_MACRO(x) foo(x)
#define MY_MACRO2(x)	foo(x)
#define	MY_MACRO3(x)	foo(x)

// Test quoted characters ('"', '{', '}')
void quoteTest1() { if (token == '"') printf("foo\n"); }
void quoteTest2() { if (token == '{') printf("foo\n"); }
void quoteTest3() { if (token == '}') printf("foo\n"); }
void quoteTest4() { printf("foo\n"); }

