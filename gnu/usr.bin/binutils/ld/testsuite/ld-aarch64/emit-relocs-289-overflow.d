#source: emit-relocs-289-overflow.s
#ld: -T relocs.ld --defsym tempy=0x1100000000 -e0 --emit-relocs
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_MOVW_PREL_G1 against symbol `tempy' .*
