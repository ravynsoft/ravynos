#name: PRU R_PRU_U8_PCREL
#source: pcrel_u8.s
#source: pcrel_u8_label.s
#ld:
#objdump: -dr --prefix-addresses

# Test the relative quick branch relocations.
.*: +file format elf32-pru

Disassembly of section .text:
[0-9a-f]+ <[^>]*> loop	[0-9a-f]+ <end_loop>, 5
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> nop
[0-9a-f]+ <[^>]*> nop
