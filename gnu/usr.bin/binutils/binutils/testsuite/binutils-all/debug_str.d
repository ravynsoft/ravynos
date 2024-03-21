#PROG: objcopy
#source: debug_str.s
#objdump: -h
#name: Uncompressed .debug_str section starting with ZLIB

.*ebug_str.copy.o:     file format .*
#...
  . .debug_str    0+01.  0+0  0+0  0+0..  2..0
#...
