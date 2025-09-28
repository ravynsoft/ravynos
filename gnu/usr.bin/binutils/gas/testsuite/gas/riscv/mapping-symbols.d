#as: -misa-spec=20191213
#source: mapping.s
#objdump: --syms --special-syms

.*file format.*riscv.*

SYMBOL TABLE:
0+00 l    d  .text	0+00 .text
0+00 l    d  .data	0+00 .data
0+00 l    d  .bss	0+00 .bss
0+00 l    d  .text.cross.section.A	0+00 .text.cross.section.A
0+00 l       .text.cross.section.A	0+00 \$xrv32i2p1_c2p0
0+00 l    d  .text.corss.section.B	0+00 .text.corss.section.B
0+00 l       .text.corss.section.B	0+00 \$xrv32i2p1_c2p0
0+02 l       .text.corss.section.B	0+00 \$xrv32i2p1
0+00 l    d  .text.data	0+00 .text.data
0+00 l       .text.data	0+00 \$d
0+08 l       .text.data	0+00 \$xrv32i2p1_c2p0
0+0c l       .text.data	0+00 \$d
0+00 l    d  .text.odd.align.start.insn	0+00 .text.odd.align.start.insn
0+00 l       .text.odd.align.start.insn	0+00 \$xrv32i2p1_c2p0
0+02 l       .text.odd.align.start.insn	0+00 \$d
0+08 l       .text.odd.align.start.insn	0+00 \$xrv32i2p1
0+00 l    d  .text.odd.align.start.data	0+00 .text.odd.align.start.data
0+00 l       .text.odd.align.start.data	0+00 \$d
0+00 l    d  .text.zero.fill.first	0+00 .text.zero.fill.first
0+00 l       .text.zero.fill.first	0+00 \$xrv32i2p1_c2p0
0+00 l    d  .text.zero.fill.last	0+00 .text.zero.fill.last
0+00 l       .text.zero.fill.last	0+00 \$xrv32i2p1_c2p0
0+02 l       .text.zero.fill.last	0+00 \$x
0+00 l    d  .text.zero.fill.align.A	0+00 .text.zero.fill.align.A
0+00 l       .text.zero.fill.align.A	0+00 \$xrv32i2p1_c2p0
0+00 l    d  .text.zero.fill.align.B	0+00 .text.zero.fill.align.B
0+00 l       .text.zero.fill.align.B	0+00 \$xrv32i2p1
0+00 l    d  .text.last.section	0+00 .text.last.section
0+00 l       .text.last.section	0+00 \$xrv32i2p1
0+04 l       .text.last.section	0+00 \$d
0+00 l    d  .text.section.padding	0+00 .text.section.padding
0+00 l       .text.section.padding	0+00 \$xrv32i2p1_c2p0
0+04 l       .text.section.padding	0+00 \$xrv32i2p1_a2p1_c2p0
0+06 l       .text.section.padding	0+00 \$d
0+00 l    d  .text.relax.align	0+00 .text.relax.align
0+00 l       .text.relax.align	0+00 \$xrv32i2p1_c2p0
0+08 l       .text.relax.align	0+00 \$xrv32i2p1
0+0a l       .text.section.padding	0+00 \$x
0+03 l       .text.odd.align.start.insn	0+00 \$d
0+04 l       .text.odd.align.start.insn	0+00 \$x
0+01 l       .text.odd.align.start.data	0+00 \$d
0+02 l       .text.odd.align.start.data	0+00 \$xrv32i2p1_c2p0
0+00 l    d  .riscv.attributes	0+00 .riscv.attributes
0+00 g       .text.cross.section.A	0+00 funcA
0+00 g       .text.corss.section.B	0+00 funcB
