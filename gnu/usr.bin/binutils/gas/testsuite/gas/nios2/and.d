#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 and

# Test the and macro.

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 2108703a 	and	r4,r4,r4
0+0004 <[^>]*> 211fffcc 	andi	r4,r4,32767
0+0008 <[^>]*> 2120000c 	andi	r4,r4,32768
0+000c <[^>]*> 213fffcc 	andi	r4,r4,65535
0+0010 <[^>]*> 2100000c 	andi	r4,r4,0
0+0014 <[^>]*> 211fffec 	andhi	r4,r4,32767
0+0018 <[^>]*> 2120002c 	andhi	r4,r4,32768
0+001c <[^>]*> 213fffec 	andhi	r4,r4,65535
0+0020 <[^>]*> 2100002c 	andhi	r4,r4,0
