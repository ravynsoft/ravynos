#source: dummy.s
#as: --32
#ld: -m elf_i386 tmpdir/start32.o tmpdir/foo32.o
#readelf: -h

ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 \(current\)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC \(Executable file\)
  Machine:                           Intel 80386
  Version:                           0x1
#pass
