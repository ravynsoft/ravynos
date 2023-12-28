#source: start1.s
#source: tls128.s
#source: tls-ie-8e.s
#as: --no-underscore --em=criself
#ld: -m crislinux --hash-style=sysv
#ld_after_inputfiles: tmpdir/tls-dso-xz-1.so
#objdump: -s -h -t -T -R -r -p

# Check that we have proper NPTL/TLS markings and GOT for a
# R_CRIS_32_IE, for an executable, symbol defined elsewhere.

.*:     file format elf32-cris

Program Header:
#...
     TLS off   .*
         filesz 0x0+80 memsz 0x0+80 flags r--

Dynamic Section:
  NEEDED               tmpdir/tls-dso-xz-1.so
#...
private flags = 0:
#...
  8 .got[ 	]+0+10 .*
                  CONTENTS, ALLOC, LOAD, DATA
SYMBOL TABLE:
#...
0+         \*UND\*	0+ x
#...
DYNAMIC SYMBOL TABLE:
0+      D  \*UND\*	0+ x
#...
DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
0+82278 R_CRIS_32_TPREL   x

Contents of section .interp:
#...
Contents of section \.text:
 80160 41b20000 6fae7822 08000000           .*
Contents of section \.tdata:
#...
Contents of section \.got:
 8226c ec210800 00000000 00000000 00000000  .*
