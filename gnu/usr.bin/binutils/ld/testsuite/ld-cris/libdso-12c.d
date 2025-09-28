#source: expdyn1.s
#source: dsov32-1.s
#source: dsov32-2.s
#source: dso-1.s
#as: --pic --no-underscore --march=v32 --em=criself
#ld: --shared -m crislinux --version-script $srcdir/$subdir/hidedsofns2468 --hash-style=sysv
#objdump: -s -T

# Like libdso-12b.d, but dsofn is defined and the two called functions
# are forced local using a linker script.  There should just be the
# GOT relocation for expobj in the DSO.

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
#...
0+1ae g[ 	]+DF \.text	0+2  Base[ 	]+expfn
0+2268 g[ 	]+DO \.data	0+4  Base[ 	]+expobj
#...
0+1b0 g[ 	]+DF \.text	0+8  Base[ 	]+dsofn3
#...
Contents of section \.rela\.dyn:
 01a0 64220000 0a040000 00000000           .*
Contents of section \.text:
 01ac b005b005 bfbe1c00 0000b005 7f0da020  .*
 01bc 00005f0d 0c00bfbe f6ffffff b0050000  .*
 01cc b0050000                             .*
Contents of section .dynamic:
 21d0 04000000 94000000 05000000 34010000  .*
 21e0 06000000 c4000000 0a000000 26000000  .*
 21f0 0b000000 10000000 07000000 a0010000  .*
 2200 08000000 0c000000 09000000 0c000000  .*
 2210 fcffff6f 68010000 fdffff6f 02000000  .*
 2220 f0ffff6f 5a010000 00000000 00000000  .*
 2230 00000000 00000000 00000000 00000000  .*
 2240 00000000 00000000 00000000 00000000  .*
 2250 00000000 00000000                    .*
Contents of section \.got:
 2258 d0210000 00000000 00000000 00000000  .*
Contents of section \.data:
 2268 00000000                             .*
