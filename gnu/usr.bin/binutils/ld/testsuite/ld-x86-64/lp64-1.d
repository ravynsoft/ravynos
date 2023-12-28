#source: dummy.s
#as: --64
#ld: -m elf_x86_64 tmpdir/start64.o tmpdir/foo64.o
#readelf: -h

ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 \(current\)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC \(Executable file\)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
#pass
