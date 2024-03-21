#objdump: -dr
#as: -32 -I$srcdir/$subdir
#name: SAVE/RESTORE instruction subset disassembly
#source: save-sub.s

.*: +file format .*mips.*

Disassembly of section .text:
00000000 <func>:
   0:[ 	]+6481[ 	]+.short[ 	]0x6481
   2:[ 	]+64c2[ 	]+.short[ 	]0x64c2
   4:[ 	]+64a3[ 	]+.short[ 	]0x64a3
   6:[ 	]+6494[ 	]+.short[ 	]0x6494
   8:[ 	]+64b5[ 	]+.short[ 	]0x64b5
   a:[ 	]+64e6[ 	]+.short[ 	]0x64e6
   c:[ 	]+64d7[ 	]+.short[ 	]0x64d7
   e:[ 	]+64f8[ 	]+.short[ 	]0x64f8
  10:[ 	]+64f9[ 	]+.short[ 	]0x64f9
  12:[ 	]+64fa[ 	]+.short[ 	]0x64fa
  14:[ 	]+64fb[ 	]+.short[ 	]0x64fb
  16:[ 	]+64f0[ 	]+.short[ 	]0x64f0
  18:[ 	]+f010[ 	]+extend[ 	]0x10
  1a:[ 	]+6481[ 	]+.short[ 	]0x6481
  1c:[ 	]+f010[ 	]+extend[ 	]0x10
  1e:[ 	]+64c2[ 	]+.short[ 	]0x64c2
  20:[ 	]+f010[ 	]+extend[ 	]0x10
  22:[ 	]+64b3[ 	]+.short[ 	]0x64b3
  24:[ 	]+f100[ 	]+extend[ 	]0x100
  26:[ 	]+6488[ 	]+.short[ 	]0x6488
  28:[ 	]+f600[ 	]+extend[ 	]0x600
  2a:[ 	]+6489[ 	]+.short[ 	]0x6489
  2c:[ 	]+f700[ 	]+extend[ 	]0x700
  2e:[ 	]+648a[ 	]+.short[ 	]0x648a
  30:[ 	]+f700[ 	]+extend[ 	]0x700
  32:[ 	]+64bb[ 	]+.short[ 	]0x64bb
  34:[ 	]+f001[ 	]+extend[ 	]0x1
  36:[ 	]+6488[ 	]+.short[ 	]0x6488
  38:[ 	]+f012[ 	]+extend[ 	]0x12
  3a:[ 	]+6480[ 	]+.short[ 	]0x6480
  3c:[ 	]+f02b[ 	]+extend[ 	]0x2b
  3e:[ 	]+6480[ 	]+.short[ 	]0x6480
  40:[ 	]+f024[ 	]+extend[ 	]0x24
  42:[ 	]+6480[ 	]+.short[ 	]0x6480
  44:[ 	]+f018[ 	]+extend[ 	]0x18
  46:[ 	]+6480[ 	]+.short[ 	]0x6480
  48:[ 	]+f00e[ 	]+extend[ 	]0xe
  4a:[ 	]+6488[ 	]+.short[ 	]0x6488
  4c:[ 	]+f015[ 	]+extend[ 	]0x15
  4e:[ 	]+6480[ 	]+.short[ 	]0x6480
  50:[ 	]+f017[ 	]+extend[ 	]0x17
  52:[ 	]+6480[ 	]+.short[ 	]0x6480
  54:[ 	]+f01a[ 	]+extend[ 	]0x1a
  56:[ 	]+6480[ 	]+.short[ 	]0x6480
  58:[ 	]+f01d[ 	]+extend[ 	]0x1d
  5a:[ 	]+6480[ 	]+.short[ 	]0x6480
  5c:[ 	]+f71a[ 	]+extend[ 	]0x71a
  5e:[ 	]+64f0[ 	]+.short[ 	]0x64f0
  60:[ 	]+6470[ 	]+.short[ 	]0x6470
  62:[ 	]+f010[ 	]+extend[ 	]0x10
  64:[ 	]+6441[ 	]+.short[ 	]0x6441
  66:[ 	]+f100[ 	]+extend[ 	]0x100
  68:[ 	]+6408[ 	]+.short[ 	]0x6408
  6a:[ 	]+f71a[ 	]+extend[ 	]0x71a
  6c:[ 	]+6470[ 	]+.short[ 	]0x6470
	\.\.\.
