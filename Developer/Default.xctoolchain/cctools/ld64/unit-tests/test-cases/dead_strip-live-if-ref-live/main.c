#include <stdio.h>

int aa = 10;
int bb = 20;
int cc = 30;


int main()
{
  printf("%p %p\n", &aa, &cc);
	return 0;
}


struct MetaData {
  void*		addr;
  unsigned long	size;
  const char*	name;
};


#define META_DATA(__x)							  \
  __attribute__((used, section("__DATA,__meta,regular,live_support")))	  \
  static struct MetaData __x##Info = { &__x, sizeof(__x), #__x };


META_DATA(aa);
META_DATA(bb);
META_DATA(cc);
