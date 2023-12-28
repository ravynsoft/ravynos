#source: dummy.s
#as: --32 -march=iamcu
#ld: -m elf_iamcu tmpdir/startiamcu.o tmpdir/foo32.o
#error: .*i386(:.+)? architecture of input file `tmpdir/foo32.o' is incompatible with iamcu.* output
