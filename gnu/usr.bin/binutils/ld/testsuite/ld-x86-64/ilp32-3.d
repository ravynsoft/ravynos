#source: dummy.s
#as: --x32
#ld: -m elf32_x86_64 tmpdir/startx32.o tmpdir/foo64.o
#error: .*i386:x86-64(:.+)? architecture of input file `tmpdir/foo64.o' is incompatible with i386:x64-32.* output
