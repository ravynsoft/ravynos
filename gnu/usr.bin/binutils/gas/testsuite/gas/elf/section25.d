#name: sections 25 (.noinit)
#source: section25.s
#readelf: -S --wide
# The h8300 port issues a warning message for
# new sections created without atrributes.
#xfail: h8300-*

#...
  \[..\] .noinit[ 	]+NOBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00  WA .*
#...
  \[..\] .noinit.foo[ 	]+NOBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00  WA .*
#...
  \[..\] .gnu.linkonce.n.bar[ 	]+NOBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00  WA .*
#pass
