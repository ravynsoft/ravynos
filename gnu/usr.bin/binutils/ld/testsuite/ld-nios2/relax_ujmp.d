#name: NIOS2 relax_ujmp
#as: -relax-all
#ld: --relax -Trelax_jmp.ld
#source: relax_ujmp.s
#objdump: -dr --prefix-addresses 

# Test relaxation of unconditional jumps

.*: +file format elf32-littlenios2

Disassembly of section text2:
00000000 <[^>]*> br	00008000 <[^>]*>
00000004 <[^>]*> movhi	at,1
00000008 <[^>]*> ori	at,at,16
0000000c <[^>]*> jmp	at
00000010 <[^>]*> br	0000001c <sym>
00000014 <[^>]*> nop
00000018 <[^>]*> nop
0000001c <sym> nop

Disassembly of section text1:
00008000 <[^>]*> br	00010000 <on_border>
00008004 <[^>]*> movhi	at,1
00008008 <[^>]*> ori	at,at,16
0000800c <[^>]*> jmp	at
#...
00010000 <on_border> br	00010010 <in_range>
00010004 <[^>]*> nop
00010008 <[^>]*> nop
0001000c <[^>]*> nop
00010010 <in_range> nop
#pass
