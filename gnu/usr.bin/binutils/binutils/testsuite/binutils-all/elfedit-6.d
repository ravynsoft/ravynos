#PROG: elfedit
#elfedit: --output-abiversion 20
#source: empty.s
#readelf: -h
#name: Update ELF header 6
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi

#...
ELF Header:
  Magic:   7f 45 4c 46 .*
#...
  Version:[ \t]+1 \(current\)
#...
  ABI Version:[ \t]+20
#...
