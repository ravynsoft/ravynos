#PROG: elfedit
#elfedit: --output-type exec
#source: empty.s
#readelf: -h
#name: Update ELF header 2
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi

#...
ELF Header:
  Magic:   7f 45 4c 46 .*
#...
  Version:[ \t]+1 \(current\)
#...
  Type:[ \t]+EXEC \(Executable file\)
#...
