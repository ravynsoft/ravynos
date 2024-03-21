#PROG: elfedit
#elfedit: --output-mach iamcu
#source: empty.s
#as: --32
#readelf: -h
#name: Update ELF header 5
#target: x86_64-*-* i386-*-*

#...
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 \(current\)
#...
  Machine:                           Intel MCU
#...
