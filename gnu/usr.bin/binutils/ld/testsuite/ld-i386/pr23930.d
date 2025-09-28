#source: ../ld-x86-64/pr23930a.s
#source: ../ld-x86-64/pr23930b.s
#as: --32
#ld: -m elf_i386 -z separate-code -z norelro -T ../ld-x86-64/pr23930-32.t
#objdump: --disassemble=main

#...
[a-f0-9]+ <main>:
[a-f0-9]+:	31 c0                	xor    %eax,%eax
[a-f0-9]+:	c3                   	ret
#pass
