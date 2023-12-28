#objdump: -r
#source: mixed-mode-reloc.s
#name: x86 mixed mode relocs (64-bit object)
#as: --generate-missing-build-notes=no

.*: +file format .*x86-64.*

RELOCATION RECORDS FOR \[.text\]:
OFFSET[ 	]+TYPE[ 	]+VALUE[ 	]*
[0-9a-f]+[ 	]+R_X86_64_GOT32[ 	]+xtrn
[0-9a-f]+[ 	]+R_X86_64_PLT32[ 	]+xtrn-0x0*4
[0-9a-f]+[ 	]+R_X86_64_GOT32[ 	]+xtrn
[0-9a-f]+[ 	]+R_X86_64_PLT32[ 	]+xtrn-0x0*4
[0-9a-f]+[ 	]+R_X86_64_GOT32[ 	]+xtrn
[0-9a-f]+[ 	]+R_X86_64_PLT32[ 	]+xtrn-0x0*4
