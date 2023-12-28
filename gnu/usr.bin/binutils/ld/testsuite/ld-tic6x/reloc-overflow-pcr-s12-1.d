#name: C6X relocation overflow, PCR_S12
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tgeneric.ld --defsym s=0x10002000
#source: reloc-overflow-pcr-s12.s
#error: .*relocation truncated to fit: R_C6000_PCR_S12.*
