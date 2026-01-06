
#include "foo.h"
#include "bar.h"

void* pfoo4 = &foo4;
void* pfoo2 = &foo2;

void* pbar2 = &bar2;
void* pbar1 = &bar1;	// not weak

int main (void)
{
   return 0;
}

