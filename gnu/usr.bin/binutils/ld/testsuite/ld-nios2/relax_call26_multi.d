#name: NIOS2 relax_call26_multi
#ld: --relax -Trelax_call26_multi.ld
#source: relax_call26.s
#objdump: -dr --prefix-addresses 
# Test relaxation of call26 relocations via linker stubs

.*: +file format elf32-littlenios2

Disassembly of section text0:
00000000 <_start> call	00000010 <func0>
00000004 <[^>]*> call	0000002c <func1>
00000008 <[^>]*> call	00000020 <[^>]*>
0000000c <[^>]*> jmpi	00000014 <[^>]*>
00000010 <func0> ret
00000014 <[^>]*> movhi	at,16384
00000018 <[^>]*> addi	at,at,16
0000001c <[^>]*> jmp	at
00000020 <[^>]*> movhi	at,16384
00000024 <[^>]*> addi	at,at,0
00000028 <[^>]*> jmp	at

Disassembly of section text1:
0000002c <func1> nop
00000030 <[^>]*> nop
00000034 <[^>]*> call	0000003c <[^>]*>
00000038 <[^>]*> ret
0000003c <[^>]*> movhi	at,16384
00000040 <[^>]*> addi	at,at,0
00000044 <[^>]*> jmp	at

Disassembly of section text2:
40000000 <func2a> nop
40000004 <[^>]*> nop
40000008 <[^>]*> nop
4000000c <[^>]*> ret
40000010 <func2b> nop
