#source: dso-1.s
#as: --pic --no-underscore --march=v32 --em=criself
#ld: --shared -m crislinux --hash-style=sysv
#objdump: -p -h

# Sanity check; just an empty GOT.

.*:     file format elf32-cris

Program Header:
    LOAD off    0x0+ vaddr 0x0+ paddr 0x0+ align 2\*\*13
         filesz 0x0+d4 memsz 0x0+d4 flags r-x
    LOAD off    0x0+d4 vaddr 0x0+20d4 paddr 0x0+20d4 align 2\*\*13
         filesz 0x0+64 memsz 0x0+64 flags rw-
 DYNAMIC off    0x0+d4 vaddr 0x0+20d4 paddr 0x0+20d4 align 2\*\*2
         filesz 0x0+58 memsz 0x0+58 flags rw-
Dynamic Section:
  HASH.*0x0*94
  STRTAB.*0x0*c8
  SYMTAB.*0x0*a8
  STRSZ.*0x0*7
  SYMENT.*0x0*10
private flags = 2: \[v32\]
Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 \.hash         0+14  0+94  0+94  0+94  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  1 \.dynsym       0+20  0+a8  0+a8  0+a8  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  2 \.dynstr       0+7  0+c8  0+c8  0+c8  2\*\*0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 \.text         0+4  0+d0  0+d0  0+d0  2\*\*1
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  4 \.dynamic      0+58  0+20d4  0+20d4  0+d4  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA
  5 \.got          0+c  0+212c  0+212c  0+12c  2\*\*2
                  CONTENTS, ALLOC, LOAD, DATA
