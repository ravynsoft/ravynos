#source: dummy.s
#as: --64
#ld: -m elf_x86_64 tmpdir/start64.o tmpdir/foox32.o
#error: .*i386:x64-32(:.+)? architecture of input file `tmpdir/foox32.o' is incompatible with i386:x86-64.* output
