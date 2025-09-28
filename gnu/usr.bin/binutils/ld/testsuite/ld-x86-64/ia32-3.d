#source: dummy.s
#as: --32
#ld: -m elf_i386 tmpdir/start32.o tmpdir/foo64.o
#error: .*i386:x86-64.* architecture of input file `tmpdir/foo64.o' is incompatible with i386.* output
