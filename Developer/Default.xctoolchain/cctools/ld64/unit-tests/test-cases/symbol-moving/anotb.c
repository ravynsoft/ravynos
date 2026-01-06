

#define SYMBOL_IS_HERE_IN_10_4(sym) \
                 extern const char sym##_tmp __asm("$ld$add$os10.4$_" #sym ); const char sym##_tmp = 0;

#define SYMBOL_IS_HERE_IN_10_5(sym) \
                 extern const char sym##_tmp __asm("$ld$add$os10.5$_" #sym ); const char sym##_tmp = 0;

#define SYMBOL_NOT_HERE_IN_10_4(sym) \
                 extern const char sym##_tmp __asm("$ld$hide$os10.4$_" #sym ); const char sym##_tmp = 0;
				
#define SYMBOL_NOT_HERE_IN_10_5(sym) \
                 extern const char sym##_tmp __asm("$ld$hide$os10.5$_" #sym ); const char sym##_tmp = 0;
				
				
//			10.4		10.5
// aaa		libbar		libfoo
// bbb		libfoo		libbar
//

// bbb is new here in 10.5.  It was elsewhere in 10.4
SYMBOL_NOT_HERE_IN_10_4(bbb)

// aaa was here in 10.4 and move elsewhere 
SYMBOL_IS_HERE_IN_10_4(aaa)

