
typedef float __m128 __attribute__ ((__vector_size__ (16), __may_alias__));
typedef __SIZE_TYPE__ size_t;

long s1 = 0;
__m128 r;
__m128 * volatile raddr = &r;

int main (int argc, const char **argv)
{
  return 15 & (int)(size_t)raddr;
}

void __main (void)
{
  __asm__ (".section .drectve\n"
	   "  .ascii \" -aligncomm:_r,4\"\n"
	   "  .ascii \" -aligncomm:r,4\"\n"
	   "  .text");
}

#if defined (__CYGWIN__) || defined (__MINGW32__)
void _alloca (void)
{
}
#endif
