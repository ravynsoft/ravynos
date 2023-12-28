#name: sections 26 (.persistent)
#source: section26.s
#readelf: -S --wide
# The h8300 port issues a warning message for
# new sections created without atrributes.
#xfail: h8300-*

#...
  \[..\] .persistent[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00  WA .*
#...
  \[..\] .persistent.foo[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00  WA .*
#...
  \[..\] .persistent.bss[ 	]+NOBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00  WA .*
#...
  \[..\] .gnu.linkonce.p.bar[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00  WA .*
#pass
