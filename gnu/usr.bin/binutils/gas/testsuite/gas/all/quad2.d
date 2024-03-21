#objdump : -s -j .data -j "\$DATA\$"
#name : .quad binary-not tests

.*: .*

Contents of section (\.data|\$DATA\$):
 0000 (ffffffff 7fffffff ffffffff 00000000|ffffff7f ffffffff 00000000 ffffffff) .*
#pass
