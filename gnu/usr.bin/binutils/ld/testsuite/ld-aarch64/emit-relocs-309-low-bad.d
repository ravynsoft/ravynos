#name: aarch64-emit-relocs-309-low-bad
#source: emit-relocs-309.s
#as:
#ld: -Ttext 0xFFFFFD --section-start .got=0x0
#error: .*relocation truncated to fit: R_AARCH64_GOT_LD_PREL19.*
