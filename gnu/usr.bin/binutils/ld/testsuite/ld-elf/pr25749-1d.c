#include <stdint.h>

extern void *_binary_pr25749_1_c_start;
extern void *_binary_pr25749_1_c_end;

intptr_t
size (void)
{
  return ((intptr_t) &_binary_pr25749_1_c_end
	  - (intptr_t) &_binary_pr25749_1_c_start);
}

extern void *_begin __attribute__ ((visibility("hidden")));

intptr_t
size_p (void)
{
  return (intptr_t) &_begin;
}
