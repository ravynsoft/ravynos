#name: 32 bit PC relative reloc
#as: -mmcu=avr51 -gdwarf-sections -g
#objdump: -r
#source: per-function-debugline.s
#target: avr-*-*

.*:     file format elf32-avr

RELOCATION RECORDS FOR \[.text.main\]:
#...


RELOCATION RECORDS FOR \[.debug_line\]:
OFFSET +TYPE +VALUE
00000000 R_AVR_32_PCREL    .Ldebug_line_end-0x00000004


RELOCATION RECORDS FOR \[.debug_line.text.main\]:
#...
