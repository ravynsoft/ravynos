#source: emit-relocs-287-overflow.s
#ld: -T relocs.ld --defsym tempy=0x20000 --defsym tempy2=0x0 -e0 --emit-relocs
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_MOVW_PREL_G0 against symbol `tempy' .*
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_MOVW_PREL_G0 against symbol `tempy2' .*
