/* Test 3 STT_GNU_IFUNC symbols.  */

#include "ifunc-sel.h"

int global __attribute__ ((visibility ("hidden"))) = -1;

static int
one (void)
{
  return 1;
}

static int
minus_one (void)
{
  return -1;
}

static int
zero (void) 
{
  return 0;
}

void * foo1_ifunc (void) __asm__ ("foo1");
__asm__(".type foo1, %gnu_indirect_function");

void * 
foo1_ifunc (void)
{
  return ifunc_sel (one, minus_one, zero);
}

void * foo2_ifunc (void) __asm__ ("foo2");
__asm__(".type foo2, %gnu_indirect_function");

void * 
foo2_ifunc (void)
{
  return ifunc_sel (minus_one, one, zero);
}

void * foo3_ifunc (void) __asm__ ("foo3");
__asm__(".type foo3, %gnu_indirect_function");

void * 
foo3_ifunc (void)
{
  return ifunc_sel (one, zero, minus_one);
}
