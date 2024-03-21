#name: C6X relocation overflow, ABS_S16
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x8000
#source: reloc-overflow-abs-s16.s
#error: .*relocation truncated to fit: R_C6000_ABS_S16.*
