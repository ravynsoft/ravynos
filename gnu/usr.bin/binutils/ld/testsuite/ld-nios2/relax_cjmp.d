#name: NIOS2 relax_cjmp
#as: -relax-all
#ld: --relax -Trelax_jmp.ld
#source: relax_cjmp.s
#objdump: -dr --prefix-addresses 

# Test relaxation of conditional jumps

.*: +file format elf32-littlenios2

Disassembly of section text2:
00000000 <[^>]*> bge	r2,r3,00008000 <[^>]*>
00000004 <[^>]*> bge	r2,r3,00000014 <[^>]*>
00000008 <[^>]*> movhi	at,1
0000000c <[^>]*> ori	at,at,24
00000010 <[^>]*> jmp	at
00000014 <[^>]*> bge	r3,r2,00000020 <sym>
00000018 <[^>]*> nop
0000001c <[^>]*> nop
00000020 <sym> nop

Disassembly of section text1:
00008000 <[^>]*> beq	r2,r3,00010000 <on_border>
00008004 <[^>]*> bne	r2,r3,00008014 <[^>]*>
00008008 <[^>]*> movhi	at,1
0000800c <[^>]*> ori	at,at,24
00008010 <[^>]*> jmp	at
00008014 <[^>]*> nop
00008018 <[^>]*> nop
#...
00010000 <on_border> bne	r2,r3,00010018 <in_range>
00010004 <[^>]*> nop
00010008 <[^>]*> nop
0001000c <[^>]*> nop
00010010 <[^>]*> nop
00010014 <[^>]*> nop
00010018 <in_range> nop
#pass
