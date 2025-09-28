#source: code-model.s
#as: -march=rv64i -mabi=lp64 --defsym __medany__=1 --defsym __undefweak__=1
#ld: -Tcode-model-01.ld -melf64lriscv --relax
#error: .*relocation truncated to fit: R_RISCV_GOT_HI20 against undefined symbol `symbolW'
