#as: -a32
#source: xcoff-function-1.s
#objdump: -t
#name: XCOFF function test 1 (32-bit)

.*

SYMBOL TABLE:
.*
.*
\[  2\].* .text
AUX val     8 .*
\[  4\].* .foo
AUX .* ttlsiz 0x8 .*
AUX .* typ 2 .* clss 0 .*
\[  7\].* .bar
AUX .* ttlsiz 0x4 .*
AUX .* typ 2 .* clss 0 .*


