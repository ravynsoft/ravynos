#name: --gc-sections with __start_SECTIONNAME
#source: dummy.s
#ld: --gc-sections -e main tmpdir/pr19161-1.o tmpdir/pr19161-2.o
#nm: --format=bsd
#xfail: epiphany-*-* frv-*-* iq2000-*-* lm32-*-* m32c-*-*
#xfail: mips64vr-*-* msp430-*-* powerpc*-*-eabivle rl78-*-* rx-*-*

#...
0*[1-9a-f]+[0-9a-f]*[ 	](d|D)[ 	]_*__start_my_section
#...
