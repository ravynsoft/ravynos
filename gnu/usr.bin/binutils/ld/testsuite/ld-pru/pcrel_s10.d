#name: PRU R_PRU_S10_PCREL
#source: pcrel_s10.s
#source: pcrel_s10_label.s
#ld:
#objdump: -dr --prefix-addresses

# Test the relative quick branch relocations.
.*: +file format elf32-pru

Disassembly of section .text:
[0-9a-f]+ <[^>]*> qba	[0-9a-f]+ <ext_label>
[0-9a-f]+ <[^>]*> qba	[0-9a-f]+ <ext_label\+0x10>
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> nop
