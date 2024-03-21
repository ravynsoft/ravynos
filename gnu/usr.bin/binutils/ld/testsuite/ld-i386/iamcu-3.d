#source: dummy.s
#as: --32
#ld: -m elf_i386 tmpdir/startiamcu.o tmpdir/foo32.o
#error: iamcu.* architecture of input file `tmpdir/startiamcu.o' is incompatible with .*i386(:.+)? output
