# relax-align.o has 3 andi(nop) insns.
# relax-align has 2 andi insns, ld relax delete andi insns.
# the last pcaddi 16 bytes align.
  .text
L1:
  la.local $a0, L1
  la.local $a0, L1
  .align 4
  la.local $a0, L1
