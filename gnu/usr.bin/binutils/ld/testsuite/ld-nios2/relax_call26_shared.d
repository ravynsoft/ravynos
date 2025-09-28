#name: NIOS2 relax_call26_shared
#ld: --relax -Trelax_call26_shared.ld
#source: relax_call26.s
#objdump: -dr --prefix-addresses 
# Test relaxation of call26 relocations via linker stubs

.*: +file format elf32-littlenios2

Disassembly of section text0:
00000000 <_start> call	00000010 <func0>
00000004 <[^>]*> call	00000014 <func1>
00000008 <[^>]*> call	00000030 <[^>]*>
0000000c <[^>]*> jmpi	00000024 <[^>]*>
00000010 <func0> ret
00000014 <func1> nop
00000018 <[^>]*> nop
0000001c <[^>]*> call	00000030 <[^>]*>
00000020 <[^>]*> ret
00000024 <[^>]*> movhi	at,16384
00000028 <[^>]*> addi	at,at,16
0000002c <[^>]*> jmp	at
00000030 <[^>]*> movhi	at,16384
00000034 <[^>]*> addi	at,at,0
00000038 <[^>]*> jmp	at

Disassembly of section text2:
40000000 <func2a> nop
40000004 <[^>]*> nop
40000008 <[^>]*> nop
4000000c <[^>]*> ret
40000010 <func2b> nop
