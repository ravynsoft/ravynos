#objdump: -dr
#as: -32
#name: SAVE/RESTORE instructions
#source: save.s

.*: +file format .*mips.*

Disassembly of section .text:
00000000 <func>:
   0:[ 	]+7000205f[ 	]+save[ 	]+8
   4:[ 	]+7000309f[ 	]+save[ 	]+16,ra
   8:[ 	]+700028df[ 	]+save[ 	]+24,s0
   c:[ 	]+7000251f[ 	]+save[ 	]+32,s1
  10:[ 	]+70002d5f[ 	]+save[ 	]+40,s0-s1
  14:[ 	]+7000399f[ 	]+save[ 	]+48,ra,s0
  18:[ 	]+700035df[ 	]+save[ 	]+56,ra,s1
  1c:[ 	]+70003e1f[ 	]+save[ 	]+64,ra,s0-s1
  20:[ 	]+70003e5f[ 	]+save[ 	]+72,ra,s0-s1
  24:[ 	]+70003e9f[ 	]+save[ 	]+80,ra,s0-s1
  28:[ 	]+70003edf[ 	]+save[ 	]+88,ra,s0-s1
  2c:[ 	]+70083c1f[ 	]+save[ 	]+128,ra,s0-s1
  30:[ 	]+7008205f[ 	]+save[ 	]+136
  34:[ 	]+7008309f[ 	]+save[ 	]+144,ra
  38:[ 	]+70082cdf[ 	]+save[ 	]+152,s0-s1
  3c:[ 	]+7080221f[ 	]+save[ 	]+64,s2
  40:[ 	]+7300225f[ 	]+save[ 	]+72,s2-s7
  44:[ 	]+7380229f[ 	]+save[ 	]+80,s2-s8
  48:[ 	]+73802edf[ 	]+save[ 	]+88,s0-s8
  4c:[ 	]+7000a21f[ 	]+save[ 	]+64,a3
  50:[ 	]+7009201f[ 	]+save[ 	]+128,a2-a3
  54:[ 	]+7015a01f[ 	]+save[ 	]+256,a0-a3
  58:[ 	]+7012201f[ 	]+save[ 	]+a0,256
  5c:[ 	]+700c201f[ 	]+save[ 	]+a0-a1,128
  60:[ 	]+7007221f[ 	]+save[ 	]+a0-a3,64
  64:[ 	]+700aa01f[ 	]+save[ 	]+a0,128,a3
  68:[ 	]+700ba01f[ 	]+save[ 	]+a0,128,a1-a3
  6c:[ 	]+700d201f[ 	]+save[ 	]+a0-a1,128,a2-a3
  70:[ 	]+700ea01f[ 	]+save[ 	]+a0-a2,128,a3
  74:[ 	]+738d3c1f[ 	]+save[ 	]+a0-a1,128,ra,s0-s8,a2-a3
  78:[ 	]+70081c1f[ 	]+restore[ 	]+128,ra,s0-s1
  7c:[ 	]+7008105f[ 	]+restore[ 	]+136,ra
  80:[ 	]+7080021f[ 	]+restore[ 	]+64,s2
  84:[ 	]+738d1c1f[ 	]+restore[ 	]+a0-a1,128,ra,s0-s8,a2-a3
	\.\.\.
