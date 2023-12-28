#readelf: -Wa

# Sanity check; the .section line caused a gas SEGV.
# See PR gas/14521.

#...
 +\[ .\] \.text +PROGBITS +0+ +0+48 +0+ +0+ +AXG +.*
#...
COMDAT group section \[    1\] `\.group' \[\.foo\] contains 1 sections:
 +\[Index\] +Name
 +\[ +.\] +\.text
#pass
