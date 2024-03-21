#name: C6X unknown attribute merging 2
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-unknown-55-a.s
#source: attr-unknown-55-a.s
#error: .*error: unknown mandatory EABI object attribute 55
