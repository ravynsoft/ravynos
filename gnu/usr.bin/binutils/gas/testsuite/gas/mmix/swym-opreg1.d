# objdump: -dr
.*:     file format elf64-mmix

Disassembly of section \.text:

0+ <Main>:
   0:	fd000005 	swym 0,0,5
   4:	fd000005 	swym 0,0,5
   8:	fd020005 	swym 2,0,5
   c:	fd020005 	swym 2,0,5
  10:	fd010203 	swym 1,2,3
  14:	fd010203 	swym 1,2,3
  18:	fd010203 	swym 1,2,3
  1c:	fd010203 	swym 1,2,3
  20:	00000005 	trap 0,0,5
  24:	00000005 	trap 0,0,5
  28:	00020005 	trap 2,0,5
  2c:	00020005 	trap 2,0,5
  30:	00010203 	trap 1,2,3
  34:	00010203 	trap 1,2,3
  38:	00010203 	trap 1,2,3
  3c:	00010203 	trap 1,2,3
  40:	ff0b1621 	trip 11,22,33
  44:	fd00007b 	swym 0,0,123
  48:	fd7bb26e 	swym 123,178,110
  4c:	ff12d687 	trip 18,214,135
  50:	007b002d 	trap 123,0,45
  54:	007bb26e 	trap 123,178,110
  58:	fffeff01 	trip 254,255,1
  5c:	fd63ff00 	swym 99,255,0

