#source: mbind1.s
#ld: -shared -z common-page-size=0x1000 -z max-page-size=0x200000
#readelf: -S -l --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

#...
  \[[ 0-9]+\] \.mbind\.text[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 AXD  0   0 4096
  \[[ 0-9]+\] \.mbind\.text[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 AXD  0   3 4096
  \[[ 0-9]+\] \.mbind\.rodata[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00  AD  0   2 4096
#...
  \[[ 0-9]+\] \.mbind\.data[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 WAD  0   0 4096
  \[[ 0-9]+\] \.mbind\.data[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 WAD  0   3 4096
#...
  \[[ 0-9]+\] \.mbind\.bss[ 	]+NOBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 WAD  0   0 4096
  \[[ 0-9]+\] \.mbind\.bss[ 	]+NOBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 WAD  0   3 4096
#...
Program Headers:
#...
  GNU_MBIND\+0[ \t]+0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ R E 0x1000
  GNU_MBIND\+0x3[ \t]+0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ R E 0x1000
  GNU_MBIND\+0x2[ \t]+0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ R   0x1000
  GNU_MBIND\+0[ \t]+0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW  0x1000
  GNU_MBIND\+0x3[ \t]+0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW  0x1000
  GNU_MBIND\+0[ \t]+0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW  0x1000
  GNU_MBIND\+0x3[ \t]+0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ 0x[0-9a-f]+ RW  0x1000
#...
 Section to Segment mapping:
#...
   [0-9]+     .mbind.text +
   [0-9]+     .mbind.text +
   [0-9]+     .mbind.rodata +
   [0-9]+     .mbind.data +
   [0-9]+     .mbind.data +
   [0-9]+     .mbind.bss +
   [0-9]+     .mbind.bss +
#pass
