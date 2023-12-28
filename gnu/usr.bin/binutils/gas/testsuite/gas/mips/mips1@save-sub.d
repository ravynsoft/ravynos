#objdump: -dr
#as: -32 -I$srcdir/$subdir
#name: SAVE/RESTORE instruction subset disassembly
#source: save-sub.s

.*: +file format .*mips.*

Disassembly of section .text:
00000000 <func>:
   0:[ 	]+7000205f[ 	]+.word[ 	]0x7000205f
   4:[ 	]+7000309f[ 	]+.word[ 	]0x7000309f
   8:[ 	]+700028df[ 	]+.word[ 	]0x700028df
   c:[ 	]+7000251f[ 	]+.word[ 	]0x7000251f
  10:[ 	]+70002d5f[ 	]+.word[ 	]0x70002d5f
  14:[ 	]+7000399f[ 	]+.word[ 	]0x7000399f
  18:[ 	]+700035df[ 	]+.word[ 	]0x700035df
  1c:[ 	]+70003e1f[ 	]+.word[ 	]0x70003e1f
  20:[ 	]+70003e5f[ 	]+.word[ 	]0x70003e5f
  24:[ 	]+70003e9f[ 	]+.word[ 	]0x70003e9f
  28:[ 	]+70003edf[ 	]+.word[ 	]0x70003edf
  2c:[ 	]+70083c1f[ 	]+.word[ 	]0x70083c1f
  30:[ 	]+7008205f[ 	]+.word[ 	]0x7008205f
  34:[ 	]+7008309f[ 	]+.word[ 	]0x7008309f
  38:[ 	]+70082cdf[ 	]+.word[ 	]0x70082cdf
  3c:[ 	]+7080221f[ 	]+.word[ 	]0x7080221f
  40:[ 	]+7300225f[ 	]+.word[ 	]0x7300225f
  44:[ 	]+7380229f[ 	]+.word[ 	]0x7380229f
  48:[ 	]+73802edf[ 	]+.word[ 	]0x73802edf
  4c:[ 	]+7000a21f[ 	]+.word[ 	]0x7000a21f
  50:[ 	]+7009201f[ 	]+.word[ 	]0x7009201f
  54:[ 	]+7015a01f[ 	]+.word[ 	]0x7015a01f
  58:[ 	]+7012201f[ 	]+.word[ 	]0x7012201f
  5c:[ 	]+700c201f[ 	]+.word[ 	]0x700c201f
  60:[ 	]+7007221f[ 	]+.word[ 	]0x7007221f
  64:[ 	]+700aa01f[ 	]+.word[ 	]0x700aa01f
  68:[ 	]+700ba01f[ 	]+.word[ 	]0x700ba01f
  6c:[ 	]+700d201f[ 	]+.word[ 	]0x700d201f
  70:[ 	]+700ea01f[ 	]+.word[ 	]0x700ea01f
  74:[ 	]+738d3c1f[ 	]+.word[ 	]0x738d3c1f
  78:[ 	]+70081c1f[ 	]+.word[ 	]0x70081c1f
  7c:[ 	]+7008105f[ 	]+.word[ 	]0x7008105f
  80:[ 	]+7080021f[ 	]+.word[ 	]0x7080021f
  84:[ 	]+738d1c1f[ 	]+.word[ 	]0x738d1c1f
	\.\.\.
