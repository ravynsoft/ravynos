#name: relocatable with script
#source: simple.s
#ld: -r -T relocatable.t
#readelf: -S --wide
#xfail: hppa-*-*

#...
  \[[ 0-9]+\] \.text.*[ \t]+PROGBITS[ \t]+0+800000[ \t0-9a-f]+AX.*
#...
  \[[ 0-9]+\] \.data.*[ \t]+PROGBITS[ \t]+0+900000[ \t0-9a-f]+WA.*
#pass
