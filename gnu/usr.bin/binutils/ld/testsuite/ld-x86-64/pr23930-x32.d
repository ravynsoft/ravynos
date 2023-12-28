#source: pr23930a.s
#source: pr23930b.s
#as: --x32
#ld: -m elf32_x86_64 -z separate-code -z norelro -T pr23930-32.t
#objdump: --disassemble=main

#...
[a-f0-9]+ <main>:
[a-f0-9]+:	31 c0                	xor    %eax,%eax
[a-f0-9]+:	c3                   	ret
#pass
