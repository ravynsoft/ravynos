#name: aarch64-emit-relocs-309-up-bad
#source: emit-relocs-309.s
#as:
#ld: -Ttext 0x0 --section-start .got=0x100001
#error: .*relocation truncated to fit: R_AARCH64_GOT_LD_PREL19.*
