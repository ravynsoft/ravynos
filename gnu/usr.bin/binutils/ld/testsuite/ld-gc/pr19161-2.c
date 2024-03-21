int __attribute__((section("my_section"))) a[2] = {0x1234, 0x5678};

extern int __start_my_section;

extern int (*p)(void);

int
dump()
{
   int* ap = &__start_my_section;
   return ap[0];
}

void 
__attribute__((constructor))
foo()
{
  p = dump;
}
