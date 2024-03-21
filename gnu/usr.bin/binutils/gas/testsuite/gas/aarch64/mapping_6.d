#objdump: --syms --special-syms
#as: --generate-missing-build-notes=no
#name: AArch64 Mapping Symbols Test 6
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*: +file format.*aarch64.*

SYMBOL TABLE:
[0]+00 l    d  .text	[0]+00 .text
[0]+00 l    d  .data	[0]+00 .data
[0]+00 l    d  .bss	[0]+00 .bss
[0]+00 l       .text	[0]+00 \$x
[0]+04 l       .text	[0]+00 \$d
[0]+10 l       .text	[0]+00 \$d
[0]+09 l       .text	[0]+00 \$d
[0]+0c l       .text	[0]+00 \$x
