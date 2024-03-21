#include "protected-data-1.h"

int protected_data_1a __attribute__ ((visibility("protected"))) = 1;
int protected_data_1b __attribute__ ((visibility("protected"))) = 2;

int *
protected_data_1a_p (void)
{
  return &protected_data_1a;
}

int *
protected_data_1b_p (void)
{
  return &protected_data_1b;
}

void
set_protected_data_1a (int i)
{
  protected_data_1a = i;
}

void
set_protected_data_1b (int i)
{
  protected_data_1b = i;
}

int
check_protected_data_1a (int i)
{
  return protected_data_1a == i ? 0 : 1;
}

int
check_protected_data_1b (int i)
{
  return protected_data_1b == i ? 0 : 1;
}
