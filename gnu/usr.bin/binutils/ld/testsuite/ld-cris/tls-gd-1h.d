#as: --pic --no-underscore --em=criself
#ld: --shared -m crislinux --hash-style=sysv
#source: tls-gd-1.s
#source: tls128g.s
#source: tls-hx.s
#objdump: -s -t -R -p

# DSO with a single R_CRIS_16_GOT_GD against a hidden symbol.  Check
# that we have proper NPTL/TLS markings and GOT.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+ paddr 0x0+ align 2\*\*13
         filesz 0x0+134 memsz 0x0+134 flags r-x
    LOAD off    0x0+134 vaddr 0x0+2134 paddr 0x0+2134 align 2\*\*13
         filesz 0x0+108 memsz 0x0+108 flags rw-
 DYNAMIC off    0x0+1b8 vaddr 0x0+21b8 paddr 0x0+21b8 align 2\*\*2
         filesz 0x0+70 memsz 0x0+70 flags rw-
     TLS off    0x0+134 vaddr 0x0+2134 paddr 0x0+2134 align 2\*\*2
         filesz 0x0+84 memsz 0x0+84 flags r--

Dynamic Section:
  HASH                 0x0+b4
  STRTAB               0x0+110
  SYMTAB               0x0+d0
  STRSZ                0x0+12
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
0+2234 R_CRIS_DTP        \*ABS\*\+0x0+80

Contents of section \.hash:
#...
Contents of section \.text:
 0130 5fae0c00                             .*
#...
Contents of section \.got:
 2228 b8210+ 0+ 0+ 0+  .*
 2238 0+                             .*
