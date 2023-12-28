#source: emit-relocs-291-overflow.s
#ld: -T relocs.ld --defsym tempy=0x10000000000000 -e0 --emit-relocs
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_MOVW_PREL_G2 against symbol `tempy' .*
