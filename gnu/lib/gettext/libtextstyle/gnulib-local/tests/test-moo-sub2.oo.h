#include "test-moo-sub1.h"

/* Define a subclass.  */
struct sub2 : struct sub1
{
  int indent;
methods:
  void begin_indent (sub2_t x);
  void end_indent (sub2_t x);
};
