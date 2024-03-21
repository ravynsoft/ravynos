#name: AVR, large .debug_line table
#as: -mlink-relax -mmcu=avrxmega2
#objdump: --dwarf=decodedline
#source: large-debug-line-table.s
#target: avr-*-*

.*:     file format elf32-avr

Contents of the \.debug_line section:

CU: large-debug-line-table\.c:
File +name +Line +number +Starting +address +View +Stmt
large-debug-line-table.c +1 +0 +x
#...
