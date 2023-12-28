#source: dso-1b.s
#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: --shared -m crislinux --version-script $srcdir/$subdir/hidedsofns2468 --hash-style=sysv
#objdump: -T

# Like libdso-1, but export the function as expfn@@TST2 and another
# function as expobj@@TST2.

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
#...
00000[12].[02468ace] g    DF .text	0+2[ 	]+TST2[	 ]+expobj
00000[12].[02468ace] g    DF .text	0+2[ 	]+TST2[	 ]+expfn
#pass
