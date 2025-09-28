#name: NIOS2 gp-relative relocations
#source: gprel.s
#ld:
#objdump: -dr --prefix-addresses

# Test the %gprel macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
[0-9a-f]+ <[^>]*> movui	gp,[0-9]+
[0-9a-f]+ <[^>]*> ldw	at,-[0-9]+\(gp\)
[0-9a-f]+ <[^>]*> ldw	r2,-[0-9]+\(gp\)
[0-9a-f]+ <[^>]*> ldb	r3,-[0-9]+\(gp\)
[0-9a-f]+ <[^>]*> ldw	at,-[0-9]+\(gp\)
[0-9a-f]+ <[^>]*> ldw	r2,-[0-9]+\(gp\)
[0-9a-f]+ <[^>]*> ldb	r3,-[0-9]+\(gp\)
