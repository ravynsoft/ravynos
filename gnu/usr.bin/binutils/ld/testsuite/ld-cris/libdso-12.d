#source: expdyn1.s
#source: dsov32-1.s
#source: dsov32-2.s
#as: --pic --no-underscore --march=v32 --em=criself
#ld: --shared -m crislinux -z nocombreloc --hash-style=sysv
#objdump: -s -T

# Check for common DSO contents; load of GOT register, branch to
# function PLT, undefined symbol, GOT reloc.

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
#...
0+1f2 g    DF \.text	0+12 dsofn4
0+1e8 g    DF \.text	0+2 expfn
0+22b0 g    DO \.data	0+4 expobj
#...
0+1ea g    DF \.text	0+8 dsofn3
#...
0+      D  \*UND\*	0+ dsofn
#...
Contents of section \.rela\.got:
 0174 ac220000 0a040000 00000000           .*
Contents of section \.rela\.plt:
 0180 a4220000 0b020000 00000000 a8220000  .*
 0190 0b070000 00000000                    .*
Contents of section \.plt:
 0198 84e20401 7e7a3f7a 04f26ffa bf09b005  .*
 01a8 00000000 00000000 00006f0d 0c000000  .*
 01b8 6ffabf09 b0053f7e 00000000 bf0ed4ff  .*
 01c8 ffffb005 6f0d1000 00006ffa bf09b005  .*
 01d8 3f7e0c00 0000bf0e baffffff b005      .*
Contents of section \.text:
 01e6 b005b005 bfbee2ff ffffb005 7f0da620  .*
 01f6 00005f0d 1400bfbe b6ffffff b0050000  .*
Contents of section \.dynamic:
 2208 04000000 94000000 05000000 48010000  .*
 2218 06000000 c8000000 0a000000 29000000  .*
 2228 0b000000 10000000 03000000 98220000  .*
 2238 02000000 18000000 14000000 07000000  .*
 2248 17000000 80010000 07000000 74010000  .*
 2258 08000000 0c000000 09000000 0c000000  .*
 2268 00000000 00000000 00000000 00000000  .*
 2278 00000000 00000000 00000000 00000000  .*
 2288 00000000 00000000 00000000 00000000  .*
Contents of section \.got:
 2298 08220000 00000000 00000000 be010000  .*
 22a8 d8010000 00000000                    .*
Contents of section \.data:
 22b0 00000000                             .*
