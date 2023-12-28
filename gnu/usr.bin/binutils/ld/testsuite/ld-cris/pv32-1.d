#source: pv32.s
#as: --march=v32 --no-underscore --em=criself
#ld: -e here -m crislinux tmpdir/libdso-12.so --hash-style=sysv
#objdump: -s -T

# Trivial test of linking a program to a v32 DSO.

.*:     file format elf32-cris

DYNAMIC SYMBOL TABLE:
0+801ca      DF \*UND\*	0+ expfn
0+822d0 g    DO \.bss	0+4 expobj
0+801e4      DF \*UND\*	0+ dsofn3
0+80210 g    DF \.text	0+8 dsofn

Contents of section \.interp:
 800d4 2f6c6962 2f6c642e 736f2e31 00        .*
#...
Contents of section \.rela\.dyn:
 8018c d0220800 09020000 00000000           .*
Contents of section \.rela\.plt:
 80198 c8220800 0b010000 00000000 cc220800  .*
 801a8 0b030000 00000000                    .*
Contents of section \.plt:
 801b0 84e26ffe c0220800 7e7a3f7a 04f26ffa  .*
 801c0 bf09b005 00000000 00006ffe c8220800  .*
 801d0 6ffabf09 b0053f7e 00000000 bf0ed4ff  .*
 801e0 ffffb005 6ffecc22 08006ffa bf09b005  .*
 801f0 3f7e0c00 0000bf0e baffffff b005      .*
Contents of section \.text:
 801fe b005bfbe caffffff b005bfbe dcffffff  .*
 8020e b0056fae d0220800 b0050000           .*
Contents of section \.dynamic:
#...
Contents of section \.got:
 822bc 1c220800 00000000 00000000 d6010800  .*
 822cc f0010800                             .*
