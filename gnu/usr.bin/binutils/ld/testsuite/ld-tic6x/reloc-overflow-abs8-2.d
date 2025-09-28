#name: C6X relocation underflow, ABS8
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0xffffff7f
#source: reloc-overflow-abs8.s
#error: .*relocation truncated to fit: R_C6000_ABS8.*
