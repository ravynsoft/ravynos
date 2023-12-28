#name: C6X relocation underflow, PCR_S12
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x0fffdffc
#source: reloc-overflow-pcr-s12.s
#error: .*relocation truncated to fit: R_C6000_PCR_S12.*
