#as: --gdwarf-5
#name: DWARF5 .file 0 (no directory)
#readelf: -wl

#...
 The Directory Table \(offset 0x.*, lines 4, columns 1\):
  Entry	Name
#...
  1	\(indirect line string, offset: 0x.*\): master directory
  2	\(indirect line string, offset: 0x.*\): secondary directory
  3	\(indirect line string, offset: 0x.*\): /tmp

 The File Name Table \(offset 0x.*, lines 3, columns 3\):
  Entry	Dir	MD5				Name
  0	1 (0x)?0	\(indirect line string, offset: 0x.*\): master source file
  1	2 (0x)?0	\(indirect line string, offset: 0x.*\): secondary source file
  2	3 0x95828e8bc4f7404dbf7526fb7bd0f192	\(indirect line string, offset: 0x.*\): foo.c
#pass


