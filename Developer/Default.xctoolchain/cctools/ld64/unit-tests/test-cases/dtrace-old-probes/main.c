
#include <stdio.h>

#define DTRACE_STRINGIFY(s) #s
#define DTRACE_TOSTRING(s) DTRACE_STRINGIFY(s)

#define DTRACE_NOPS			\
	"nop"			"\n\t"	\
	"nop"			"\n\t"	\
	"nop"			"\n\t"	


#define DTRACE_LAB(p, n)		\
   "__dtrace_probe$" DTRACE_TOSTRING(%=__LINE__) DTRACE_STRINGIFY(_##p##___##n)

#if (defined __x86_64__ || defined __arm64__)
#define DTRACE_LABEL(p, n)              \
      ".section __DATA, __data\n\t"     \
      ".globl " DTRACE_LAB(p, n) "\n\t" \
      DTRACE_LAB(p, n) ":" ".quad 1f""\n\t"    \
       ".text" "\n\t"                   \
        "1:"
#else
#define DTRACE_LABEL(p, n)		\
      ".section __DATA, __data\n\t"	\
      ".globl " DTRACE_LAB(p, n) "\n\t"	\
       DTRACE_LAB(p, n) ":\n\t" ".long 1f""\n\t"	\
       ".text" "\n\t"			\
	"1:"
#endif

#define DTRACE_CALL(p,n)	\
	DTRACE_LABEL(p,n)	\
	DTRACE_NOPS

#define DTRACE_CALL0ARGS(provider, name)							\
	__asm volatile (										\
		      DTRACE_CALL(provider, name)						\
	              :										\
	              :										\
	);

int deadwood()
{
	DTRACE_CALL0ARGS(__foo__, test2)
	return 0;
}


int main() {
	int a = 1;

	while(a) {
		DTRACE_CALL0ARGS(__foo__, test1)
	}

	return 0;
}
