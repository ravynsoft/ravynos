#name: NIOS2 relax_callr
#as: -relax-all
#ld: --relax -Trelax_callr.ld
#source: relax_callr.s
#objdump: -dr --prefix-addresses 
# Test relaxation of callr

.*: +file format elf32-littlenios2

Disassembly of section text1:
00000000 <[^>]*> movhi	at,2048
00000004 <[^>]*> ori	at,at,0
00000008 <[^>]*> callr	at
0000000c <[^>]*> movhi	at,2048
00000010 <[^>]*> ori	at,at,20
00000014 <[^>]*> callr	at

Disassembly of section text2:
08000000 <func> nop
08000004 <[^>]*> br	08000014 <func1>
08000008 <[^>]*> nop
0800000c <[^>]*> nop
08000010 <[^>]*> nop
08000014 <func1> nop
