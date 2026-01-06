
int foo()
{
	return 0;
}

			
#if __arm__ 
	#define INSTALL_NAME_4_0(sym) \
                 extern const char install_name_4_0 __asm("$ld$install_name$os4.0$" #sym ); const char install_name_4_0 = 0;
				
	INSTALL_NAME_4_0(/usr/lib/libfoo.dylib)
#else
	#define INSTALL_NAME_10_5(sym) \
                 extern const char install_name_10_5 __asm("$ld$install_name$os10.5$" #sym ); const char install_name_10_5 = 0;
				
	INSTALL_NAME_10_5(/usr/lib/libfoo.dylib)
#endif
