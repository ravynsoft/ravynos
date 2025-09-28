#objdump: --syms --special-syms
#as: --generate-missing-build-notes=no
#name: AArch64 Mapping Symbols
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

# Test the generation of AArch64 ELF Mapping Symbols

.*: +file format.*aarch64.*

SYMBOL TABLE:
0+00 l    d  .text	0+0 (|.text)
0+00 l    d  .data	0+0 (|.data)
0+00 l    d  .bss	0+0 (|.bss)
0+00 l       .text	0+0 \$x
0+00 l    d  foo	0+0 (|foo)
0+00 l       foo	0+0 \$x
#Maybe section symbol for .ARM.attributes
#...
0+00 g       .text	0+0 mapping
0+08 g       .text	0+0 another_mapping
