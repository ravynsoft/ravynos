#source: pr23633.s
#PROG: objcopy
#objcopy: --set-section-alignment .text=16
#objdump: --section-headers
#target: [is_elf_format]
#xfail: rx-*-*

#...
.*\.text.*2\*\*4
#pass
