#PROG: elfedit
#elfedit: --output-osabi FenixOS
#source: empty.s
#readelf: -h
#name: Update ELF header 3
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi

#...
ELF Header:
  Magic:   7f 45 4c 46 .*
#...
  Version:[ \t]+1 \(current\)
#...
  OS/ABI:[ \t]+FenixOS
#...
