#include "protected-func-1.h"

protected_func_type protected_func_1a_ptr = protected_func_1a;

__attribute__ ((visibility("protected")))
int
protected_func_1a (void)
{
  return 1;
}

__attribute__ ((visibility("protected")))
int
protected_func_1b (void)
{
  return 2;
}

protected_func_type
protected_func_1a_p (void)
{
  return protected_func_1a;
}

protected_func_type
protected_func_1b_p (void)
{
  return protected_func_1b;
}
