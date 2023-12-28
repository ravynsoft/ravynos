#name: NIOS2 relax_section
#as: -relax-section
#ld: --relax -Trelax_jmp.ld
#source: relax_section.s
#objdump: -dr --prefix-addresses 

# Test relaxation of section

.*: +file format elf32-littlenios2

Disassembly of section text1:
00008000 <[^>]*> bne	r2,r3,00008010 <[^>]*>
00008004 <[^>]*> nextpc	at
00008008 <[^>]*> addi	at,at,32764
0000800c <[^>]*> jmp	at
00008010 <[^>]*> bge	r2,r3,00008024 <[^>]*>
00008014 <[^>]*> nextpc	at
00008018 <[^>]*> addi	at,at,32767
0000801c <[^>]*> addi	at,at,9
00008020 <[^>]*> jmp	at
00008024 <[^>]*> bne	r2,r3,00008030 <in_range>
00008028 <[^>]*> nop
0000802c <[^>]*> nop
00008030 <in_range> nop
#...
00010000 <[^>]*> br	00008030 <in_range>
00010004 <just_out_of_range> nop
00010008 <[^>]*> nop
0001000c <[^>]*> nop
00010010 <[^>]*> nop
00010014 <[^>]*> nop
00010018 <[^>]*> nop
0001001c <[^>]*> nop
00010020 <farther_out_of_range> nop
#pass
