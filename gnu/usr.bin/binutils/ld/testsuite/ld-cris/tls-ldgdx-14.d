#source: tls128.s
#source: tls-ld-5.s
#source: tls-gd-1.s
#source: tls-ldgd-14.s
#source: tls-hx1x2.s
#as: --pic --no-underscore --em=criself
#ld: --shared -m crislinux --hash-style=sysv tmpdir/tls-dso-xz-1.so
#objdump: -s -h -t -T -R -r -p

# Check that we have proper NPTL/TLS markings and GOT for two
# R_CRIS_16_GOT_GD and two R_CRIS_16_DTPRELs against different
# variables in a DSO, GD symbols defined elsewhere.

.*:     file format elf32-cris

Program Header:
#...
     TLS off .*
         filesz 0x0+88 memsz 0x0+88 flags r--

Dynamic Section:
  NEEDED               tmpdir/tls-dso-xz-1.so
#...
private flags = 0:
#...
  7 .got .*
                  CONTENTS.*
SYMBOL TABLE:
#...
0+         \*UND\*	0+ x
#...
0+         \*UND\*	0+ z
#...
DYNAMIC SYMBOL TABLE:
#...
0+      D  \*UND\*	0+ x
#...
0+      D  \*UND\*	0+ z
#...
DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
000022d0 R_CRIS_DTPMOD     \*ABS\*
000022d8 R_CRIS_DTP        x
000022e0 R_CRIS_DTP        z

Contents of section \.hash:
#...
Contents of section \.text:
 01b4 5fae8000 5fbe8400 5fae1400 5fae1c00  .*
Contents of section .tdata:
#...
Contents of section \.got:
 22c4 4c220+ 0+ 0+ 0+  .*
 22d4 0+ 0+ 0+ 0+  .*
 22e4 0+                             .*
