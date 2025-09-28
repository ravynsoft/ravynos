#objdump: --syms --special-syms
#as:  --generate-missing-build-notes=no
#name: ARM Mapping Symbols Test 2
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

.*: +file format.*arm.*

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l       .text	00000000 \$t
00000002 l       .text	00000000 foo
00000000 l    d  .note	00000000 .note
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000008 main


