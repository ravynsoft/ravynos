#name: C6X relocation underflow, ABS_S16
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0xffff7fff
#source: reloc-overflow-abs-s16.s
#error: .*relocation truncated to fit: R_C6000_ABS_S16.*
