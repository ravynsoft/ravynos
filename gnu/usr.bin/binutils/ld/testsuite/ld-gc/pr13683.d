#name: --gc-sections with --defsym
#source: dummy.s
#ld: --gc-sections -e main --defsym foo=foo2 tmpdir/pr13683.o
#nm: --format=bsd
#xfail: iq2000-*-* lm32-*-* epiphany-*-* mips64vr-*-* frv-*-* m32c-*-* rl78-*-* rx-*-* sh-*-* powerpc*-*-eabivle msp430-*-*

# Note - look for both "foo" and "foo2" being defined, non-zero function symbols

#...
0*[1-9a-f]+[0-9a-f]*[ 	](T|D)[ 	]_*foo
#...
0*[1-9a-f]+[0-9a-f]*[ 	](T|D)[ 	]_*foo2
#...
