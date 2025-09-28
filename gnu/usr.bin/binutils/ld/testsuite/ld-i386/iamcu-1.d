#source: dummy.s
#as: --32 -march=iamcu
#ld: -m elf_iamcu tmpdir/startiamcu.o tmpdir/fooiamcu.o
#readelf: -h

ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 \(current\)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC \(Executable file\)
  Machine:                           Intel MCU
  Version:                           0x1
#pass
