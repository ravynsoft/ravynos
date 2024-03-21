#ld: -Ttext=0x60
#readelf: -S --wide
#xfail: d10v-* m68hc1*-* msp*-* pru-*-* s12z-*-* visium-* xgate-* xstormy*-*
# the above targets use memory regions that don't allow 0x60 for .text

#...
  \[[ 0-9]+\] \.text[ \t]+PROGBITS[ \t]+0*60[ \t]+.*
#pass
