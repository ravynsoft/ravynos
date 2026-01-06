#include <string.h>

int a_value = 10;

int a(const char* l, const char* r)
{
  a_value = *l;
  return (*l == *r);
}

