# Check that linking anno-sym.o produces an undefined reference message referring to '_start' and not 'annobin_hello.c'
#ld:  -e _start
#error_output: anno-sym.l
# The mips-irix6 target fails this test because it does not find any function symbols.  Not sure why.
#skip: *-*-irix*
