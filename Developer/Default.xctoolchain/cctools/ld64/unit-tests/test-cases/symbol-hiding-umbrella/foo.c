
void foo() {}

#define SYMBOL_NOT_HERE_IN_10_6(sym) \
                 extern const char sym##_tmp __asm("$ld$hide$os10.6$_" #sym ); const char sym##_tmp = 0;
				
SYMBOL_NOT_HERE_IN_10_6(bar)
