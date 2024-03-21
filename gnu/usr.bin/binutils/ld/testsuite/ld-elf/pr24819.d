#ld: -r
#readelf: -S --wide
#xfail: [uses_genelf]

#...
  \[[ 0-9]+\] \.data\.foo[ \t]+PROGBITS[ \t0-9a-f]+WAG .*
#...
  \[[ 0-9]+\] \.data\.foo[ \t]+PROGBITS[ \t0-9a-f]+WA .*
#pass
