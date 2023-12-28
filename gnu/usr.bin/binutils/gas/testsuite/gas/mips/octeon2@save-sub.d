#objdump: -dr
#as: -32 -I$srcdir/$subdir
#name: SAVE/RESTORE instruction subset disassembly
#source: save-sub.s

.*: +file format .*mips.*

Disassembly of section .text:
00000000 <func>:
   0:[ 	]+7000205f[ 	]+udi15[ 	]+zero,zero,a0,0x1
   4:[ 	]+7000309f[ 	]+lai[ 	]+a2,\(zero\)
   8:[ 	]+700028df[ 	]+laid[ 	]+a1,\(zero\)
   c:[ 	]+7000251f[ 	]+udi15[ 	]+zero,zero,a0,0x14
  10:[ 	]+70002d5f[ 	]+udi15[ 	]+zero,zero,a1,0x15
  14:[ 	]+7000399f[ 	]+lad[ 	]+a3,\(zero\)
  18:[ 	]+700035df[ 	]+lawd[ 	]+a2,\(zero\),zero
  1c:[ 	]+70003e1f[ 	]+udi15[ 	]+zero,zero,a3,0x18
  20:[ 	]+70003e5f[ 	]+udi15[ 	]+zero,zero,a3,0x19
  24:[ 	]+70003e9f[ 	]+udi15[ 	]+zero,zero,a3,0x1a
  28:[ 	]+70003edf[ 	]+udi15[ 	]+zero,zero,a3,0x1b
  2c:[ 	]+70083c1f[ 	]+udi15[ 	]+zero,t0,a3,0x10
  30:[ 	]+7008205f[ 	]+udi15[ 	]+zero,t0,a0,0x1
  34:[ 	]+7008309f[ 	]+udi15[ 	]+zero,t0,a2,0x2
  38:[ 	]+70082cdf[ 	]+laad[ 	]+a1,\(zero\),t0
  3c:[ 	]+7080221f[ 	]+udi15[ 	]+a0,zero,a0,0x8
  40:[ 	]+7300225f[ 	]+udi15[ 	]+t8,zero,a0,0x9
  44:[ 	]+7380229f[ 	]+las[ 	]+a0,\(gp\)
  48:[ 	]+73802edf[ 	]+udi15[ 	]+gp,zero,a1,0x1b
  4c:[ 	]+7000a21f[ 	]+udi15[ 	]+zero,zero,s4,0x8
  50:[ 	]+7009201f[ 	]+udi15[ 	]+zero,t1,a0,0x0
  54:[ 	]+7015a01f[ 	]+udi15[ 	]+zero,s5,s4,0x0
  58:[ 	]+7012201f[ 	]+udi15[ 	]+zero,s2,a0,0x0
  5c:[ 	]+700c201f[ 	]+udi15[ 	]+zero,t4,a0,0x0
  60:[ 	]+7007221f[ 	]+udi15[ 	]+zero,a3,a0,0x8
  64:[ 	]+700aa01f[ 	]+udi15[ 	]+zero,t2,s4,0x0
  68:[ 	]+700ba01f[ 	]+udi15[ 	]+zero,t3,s4,0x0
  6c:[ 	]+700d201f[ 	]+udi15[ 	]+zero,t5,a0,0x0
  70:[ 	]+700ea01f[ 	]+udi15[ 	]+zero,t6,s4,0x0
  74:[ 	]+738d3c1f[ 	]+udi15[ 	]+gp,t5,a3,0x10
  78:[ 	]+70081c1f[ 	]+udi15[ 	]+zero,t0,v1,0x10
  7c:[ 	]+7008105f[ 	]+udi15[ 	]+zero,t0,v0,0x1
  80:[ 	]+7080021f[ 	]+udi15[ 	]+a0,zero,zero,0x8
  84:[ 	]+738d1c1f[ 	]+udi15[ 	]+gp,t5,v1,0x10
	\.\.\.
