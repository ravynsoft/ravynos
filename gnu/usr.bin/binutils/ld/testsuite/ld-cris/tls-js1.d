#source: dsov32-1.s
#source: tls-ld-4.s
#source: dsov32-2.s
#source: expdyn1.s
#source: tls-hx.s
#source: dso-1.s
#as: --pic --no-underscore --em=criself --march=v32
#ld: --shared -m crislinux --hash-style=sysv
#readelf: -a

# DSO with a R_CRIS_16_DTPREL and a R_CRIS_32_PLT_PCREL.  The .got.plt
# byte index (a) and .rela.plt item index (b) are in sync as b=a/4-3
# *except* when there's a R_CRIS_DTPMOD, because while the relocated
# contents goes in .got.plt, the relocation goes in .rela.got, not
# .rela.plt.  And, it'd cover 8 bytes in .got.plt, not 4 bytes.
# Making sure .rela.plt has the right contents; no R_CRIS_NONE entries.

#...
  .* .got[ 	]+PROGBITS[ 	]+0+22e4 0+2e4 0+20 04  WA  0   0  4
#...
Relocation section '\.rela\.dyn' at offset .* contains 2 entries:
 Offset     Info    Type            Sym\.Value  Sym\. Name \+ Addend
000022f0  0000001e R_CRIS_DTPMOD +0
00002300  0000040a R_CRIS_GLOB_DAT   00002304   expobj \+ 0

Relocation section '\.rela\.plt' at offset .* contains 2 entries:
 Offset     Info    Type            Sym\.Value  Sym\. Name \+ Addend
000022f8  0000020b R_CRIS_JUMP_SLOT  00000232   dsofn4 \+ 0
000022fc  0000080b R_CRIS_JUMP_SLOT  0000024a   dsofn \+ 0

The decoding of unwind sections for machine type Axis Communications 32-bit embedded processor is not currently supported.
#pass
