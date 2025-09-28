#PROG: elfedit
#elfedit: --output-mach l1om
#source: empty.s
#readelf: -h
#name: Update ELF header 1
#target: x86_64-*-*
#notarget: x86_64-*-gnux32

#...
ELF Header:
  Magic:   7f 45 4c 46 0(1|2) 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF(32|64)
  Data:                              2's complement, little endian
  Version:                           1 \(current\)
#...
  Machine:                           Intel L1OM
#...
