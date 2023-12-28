#source: emit-relocs-271-overflow.s
#ld: -T relocs.ld --defsym tempy=0x100000000 --defsym tempy1=-0x100000001 -e0 --emit-relocs
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_MOVW_SABS_G1 against symbol `tempy' .*
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_MOVW_SABS_G1 against symbol `tempy1' .*
