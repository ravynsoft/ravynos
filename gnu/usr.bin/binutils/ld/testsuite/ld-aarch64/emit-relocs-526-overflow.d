#source: emit-relocs-526-overflow.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_TLSLD_MOVW_DTPREL_G0 against symbol `v2' .*
