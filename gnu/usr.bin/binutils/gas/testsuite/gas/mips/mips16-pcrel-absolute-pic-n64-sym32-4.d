#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative reference to absolute expression 4 (PIC, n64, sym32)
#as: -64 -msym32 -call_shared
#source: mips16-pcrel-absolute-4.s
#dump: mips16-pcrel-absolute-4.d
