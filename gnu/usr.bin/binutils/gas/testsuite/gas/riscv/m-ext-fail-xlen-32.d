#as: -march=rv32im -defsym rv64=1
#source: m-ext.s
#objdump: -d
#error_output: m-ext-fail-xlen-32.l
