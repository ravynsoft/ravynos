#as: --gdwarf-5
#name: DWARF5 .file 0 (directory and relative file)
#readelf: -wl

#...
 The Directory Table \(offset 0x.*, lines 1, columns 1\):
  Entry	Name
  0	\(indirect line string, offset: 0x.*\): /example

 The File Name Table \(offset 0x.*, lines 2, columns 2\):
  Entry	Dir	Name
  0	0	\(indirect line string, offset: 0x.*\): test.c
  1	0	\(indirect line string, offset: 0x.*\): test.c
#pass
