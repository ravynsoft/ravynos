#as: --gdwarf-5
#name: DWARF5 .file 0 (directory and absolute file)
#readelf: -wl

#...
 The Directory Table \(offset 0x.*, lines 2, columns 1\):
  Entry	Name
  0	\(indirect line string, offset: 0x.*\): /current/directory
  1	\(indirect line string, offset: 0x.*\): /full/path

 The File Name Table \(offset 0x.*, lines 2, columns 2\):
  Entry	Dir	Name
  0	1	\(indirect line string, offset: 0x.*\): test.c
  1	1	\(indirect line string, offset: 0x.*\): test.c
#pass
