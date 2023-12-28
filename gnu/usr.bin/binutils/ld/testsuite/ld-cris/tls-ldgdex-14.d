#source: start1.s
#source: tls128.s
#source: tls-ld-5.s
#source: tls-gd-1.s
#source: tls-ldgd-14.s
#source: tls-hx1x2.s
#as: --pic --no-underscore --em=criself
#ld: -m crislinux tmpdir/tls-dso-xz-1.so --hash-style=sysv
#objdump: -s -h -t -T -R -r -p

# Check that we have proper NPTL/TLS markings and GOT for two
# R_CRIS_16_GOT_GD and two R_CRIS_16_DTPRELs against different
# variables, for an executable, GD symbols defined elsewhere.

.*:     file format elf32-cris

Program Header:
#...
     TLS off  .*
         filesz 0x0+88 memsz 0x0+88 flags r--

Dynamic Section:
  NEEDED               tmpdir/tls-dso-xz-1.so
#...
private flags = 0:
#...
  8 .got .*
                  CONTENTS.*
SYMBOL TABLE:
#...
0+         \*UND\*	0+ x
#...
0+         \*UND\*	0+ z
#...
DYNAMIC SYMBOL TABLE:
0+      D  \*UND\*	0+ x
0+      D  \*UND\*	0+ z
#...
DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
000822b0 R_CRIS_DTP        x
000822b8 R_CRIS_DTP        z

Contents of section .interp:
#...
Contents of section \.text:
 80180 41b20000 5fae8000 5fbe8400 5fae1400  .*
 80190 5fae1c00                             .*
Contents of section \.tdata:
#...
Contents of section \.got:
 8229c 1c220800 0+ 0+ 010+  .*
 822ac 0+ 0+ 0+ 0+  .*
 822bc 0+                             .*
