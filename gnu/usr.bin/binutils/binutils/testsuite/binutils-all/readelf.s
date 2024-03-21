There are .* section headers, starting at offset .*:

Section Headers:
 +\[Nr\] Name +Type +Addr +Off +Size +ES Flg Lk Inf Al
 +\[ 0\] +NULL +00000000 000000 000000 00 +0 +0 +0
# On the normal MIPS systems, sections must be aligned to 16 byte
# boundaries. On IA64, text sections are aligned to 16 byte boundaries.
 +\[ 1\] .* +PROGBITS +00000000 0000(34|38|40) 0000(08|10) 00 +AX +0 +0 +(.|..)
 +\[ 2\] .rel.* +REL. +0+ 0+.* 0000.. 0. +I +.+ +1 +4
# MIPS targets put .rela.text here.
#...
 +\[ .\] .* +PROGBITS +00000000 0000(3c|40|44|48|50) 0000(04|10) 00 +WA +0 +0 +(.|..)
 +\[ .\] .* +NOBITS +00000000 0000(40|44|48|4c|60) 000000 00 +WA +0 +0 +(.|..)
# ARM targets put .ARM.attributes here.
# MIPS targets put .reginfo, .mdebug, .MIPS.abiflags and .gnu.attributes here.
# v850 targets put .call_table_data and .call_table_text here.
# riscv targets put .riscv.attributes here.
#...
 +\[..\] .symtab +SYMTAB +00000000 0+.* 0+.* 10 +.. +.+ +4
 +\[..\] .strtab +STRTAB +00000000 0+.* 0+.* 00 .* +0 +0 +1
 +\[..\] .shstrtab +STRTAB +00000000 0+.* 0+.* 00 .* +0 +0 +.
Key to Flags:
#...
