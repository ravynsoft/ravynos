#source: emit-relocs-528-overflow.s
#ld: -T relocs.ld -e0 --emit-relocs
#objdump: -dr
#error: .*\(.text\+0x\d+\): relocation truncated to fit: R_AARCH64_TLSLD_ADD_DTPREL_HI12 against symbol `v2' .*
