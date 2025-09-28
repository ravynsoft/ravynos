#as: --pic --no-underscore --em=criself
#ld: --shared -m crislinux --hash-style=sysv
#source: tls-ie-10.s
#source: tls128g.s
#source: tls-x.s
#objdump: -s -t -R -p -T

# DSO with a single R_CRIS_32_GOT_TPREL.  Check that we have proper
# NPTL/TLS markings and GOT.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+ paddr 0x0+ align 2\*\*13
         filesz 0x0+158 memsz 0x0+158 flags r-x
    LOAD off    0x0+158 vaddr 0x0+2158 paddr 0x0+2158 align 2\*\*13
         filesz 0x0+10c memsz 0x0+10c flags rw-
 DYNAMIC off    0x0+1dc vaddr 0x0+21dc paddr 0x0+21dc align 2\*\*2
         filesz 0x0+78 memsz 0x0+78 flags rw-
     TLS off    0x0+158 vaddr 0x0+2158 paddr 0x0+2158 align 2\*\*2
         filesz 0x0+84 memsz 0x0+84 flags r--

Dynamic Section:
  HASH                 0x0+b4
  STRTAB               0x0+12c
  SYMTAB               0x0+dc
  STRSZ                0x0+15
  SYMENT               0x0+10
  RELA                 0x0+144
  RELASZ               0x0+c
  RELAENT              0x0+c
  FLAGS                0x0+10
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
0+2260 R_CRIS_32_TPREL   x

Contents of section \.hash:
#...
Contents of section \.text:
 0150 6fae0c00 00000000                    .*
#...
Contents of section \.got:
 2254 dc210+ 0+ 0+ 0+  .*
