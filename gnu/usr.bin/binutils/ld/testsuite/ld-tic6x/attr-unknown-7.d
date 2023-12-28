#name: C6X unknown attribute merging 7
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: attr-unknown-1024-1.s
#source: attr-unknown-1024-1.s
#error: .*error: unknown mandatory EABI object attribute 1024
