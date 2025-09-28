#source: code-model.s
#as: -march=rv64i -mabi=lp64 --defsym __medlow__=1
#ld: -Tcode-model-01.ld -melf64lriscv --no-relax
#error: .*relocation truncated to fit: R_RISCV_HI20 against `symbolL'
