#skip: aarch64_be-*-*
#ld: -e0
#PROG: objcopy
#objcopy: -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .rel* -j .rela* -j .reloc --target=efi-app-aarch64
#objdump: -h -f
#name: Check if efi app format is recognized

.*:     file format pei-aarch64-little
architecture: aarch64, flags 0x00000132:
EXEC_P, HAS_SYMS, HAS_LOCALS, D_PAGED
start address 0x0000000000000000

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 \.text         00000030  0[^ ]+  0[^ ]+  0[^ ]+  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
