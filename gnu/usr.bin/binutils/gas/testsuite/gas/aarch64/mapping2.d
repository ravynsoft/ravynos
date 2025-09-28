#objdump: --syms --special-syms
#as: --generate-missing-build-notes=no
#name: AArch64 Mapping Symbols Test 2
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

.*: +file format.*aarch64.*

SYMBOL TABLE:
[0]+00 l    d  .text	0[0]+00 .text
[0]+00 l    d  .data	0[0]+00 .data
[0]+00 l    d  .bss	0[0]+00 .bss
[0]+00 l       .text	0[0]+00 \$x
[0]+04 l       .text	0[0]+00 foo
[0]+00 l    d  .comment	0[0]+00 .comment
[0]+00 g     F .text	0[0]+0c main


