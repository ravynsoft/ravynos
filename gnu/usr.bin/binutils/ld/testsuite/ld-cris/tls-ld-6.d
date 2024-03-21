#as: --pic --no-underscore --em=criself -I$srcdir/$subdir
#ld: --shared -m crislinux --hash-style=sysv
#source: tls128g.s
#source: tls-ld-6.s
#source: tls-hx.s
#objdump: -s -t -R -p

# DSO with a single R_CRIS_32_DTPREL against a hidden symbol.  Check
# that we have proper NPTL/TLS markings and GOT.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+ paddr 0x0+ align 2\*\*13
         filesz 0x0+138 memsz 0x0+138 flags r-x
    LOAD off    0x0+138 vaddr 0x0+2138 paddr 0x0+2138 align 2\*\*13
         filesz 0x0+108 memsz 0x0+108 flags rw-
 DYNAMIC off    0x0+1bc vaddr 0x0+21bc paddr 0x0+21bc align 2\*\*2
         filesz 0x0+70 memsz 0x0+70 flags rw-
     TLS off    0x0+138 vaddr 0x0+2138 paddr 0x0+2138 align 2\*\*2
         filesz 0x0+84 memsz 0x0+84 flags r--

Dynamic Section:
  HASH                 0x0+b4
  STRTAB               0x0+110
  SYMTAB               0x0+d0
  STRSZ                0x0+11
  SYMENT               0x0+10
  RELA                 0x0+124
  RELASZ               0x0+c
  RELAENT              0x0+c
private flags = 0:

SYMBOL TABLE:
#...
0+80 l       \.tdata	0+4 x
#...

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
0+2238 R_CRIS_DTPMOD     \*ABS\*

Contents of section \.hash:
#...
Contents of section \.text:
 0130 6fae8000 00000000              .*
#...
Contents of section \.got:
 222c bc210+ 0+ 0+ 0+  .*
 223c 0+                             .*
