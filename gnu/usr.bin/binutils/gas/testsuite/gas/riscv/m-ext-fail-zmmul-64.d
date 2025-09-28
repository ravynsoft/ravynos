#as: -march=rv64i_zmmul -defsym rv64=1
#source: m-ext.s
#objdump: -d
#error_output: m-ext-fail-zmmul-64.l
