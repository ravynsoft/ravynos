#name: C6X relocation underflow, PCR_S7
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x0ffffefc
#source: reloc-overflow-pcr-s7.s
#error: .*relocation truncated to fit: R_C6000_PCR_S7.*
