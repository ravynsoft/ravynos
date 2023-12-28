#as: --pic --no-underscore --em=criself
#ld: --shared -m crislinux --hash-style=sysv
#source: tls-gd-2.s
#source: tls128g.s
#source: tls-x.s
#objdump: -s -t -R -p -T

# DSO with a single R_CRIS_32_GOT_GD.  Check that we have proper
# NPTL/TLS markings and GOT.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+ paddr 0x0+ align 2\*\*13
         filesz 0x0+154 memsz 0x0+154 flags r-x
    LOAD off    0x0+154 vaddr 0x0+2154 paddr 0x0+2154 align 2\*\*13
         filesz 0x0+108 memsz 0x0+108 flags rw-
 DYNAMIC off    0x0+1d8 vaddr 0x0+21d8 paddr 0x0+21d8 align 2\*\*2
         filesz 0x0+70 memsz 0x0+70 flags rw-
     TLS off    0x0+154 vaddr 0x0+2154 paddr 0x0+2154 align 2\*\*2
         filesz 0x0+84 memsz 0x0+84 flags r--

Dynamic Section:
  HASH                 0x0+b4
  STRTAB               0x0+12c
  SYMTAB               0x0+dc
  STRSZ                0x0+14
  SYMENT               0x0+10
  RELA                 0x0+140
  RELASZ               0x0+c
  RELAENT              0x0+c
private flags = 0:

SYMBOL TABLE:
#...
0+80 g       \.tdata	0+4 x
#...
DYNAMIC SYMBOL TABLE:
#...
0+80 g    D  \.tdata	0+4 x
#...

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
0+2254 R_CRIS_DTP        x

Contents of section \.hash:
#...
Contents of section \.text:
 014c 6fae0c00 00000000              .*
#...
Contents of section \.got:
 2248 d8210+ 0+ 0+ 0+  .*
 2258 0+                             .*
