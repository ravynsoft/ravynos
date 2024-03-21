#objdump: -t
#as:  --generate-missing-build-notes=no
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

SYMBOL TABLE:
0+ l    d  \.text	0+ \.text
0+ l    d  \.data	0+ \.data
0+ l    d  \.bss	0+ \.bss
0+ l       \.text	0+ 0x80 foo
