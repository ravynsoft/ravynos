#ld: --oformat binary -T pr21529.ld -e main
#objdump: -s -b binary
#xfail: aarch64*-*-* arm*-*-* avr-*-* ia64-*-* m68hc1*-*-* nds32*-*-* riscv*-*-* score-*-* v850-*-* loongarch*-*-*
# Skip targets which can't change output format to binary.

#pass
