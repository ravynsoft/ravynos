#source: emit-relocs-272-overflow.s
#ld: -T relocs.ld --defsym tempy=0x1000000000000 --defsym tempy1=-0x1000000000001 -e0 --emit-relocs
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_MOVW_SABS_G2 against symbol `tempy' .*
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_MOVW_SABS_G2 against symbol `tempy1' .*
