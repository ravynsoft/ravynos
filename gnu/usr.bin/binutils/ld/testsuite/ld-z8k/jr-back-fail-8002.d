#name: Z8002 backward jr just out of range
#source: branch-target.s -z8002
#source: filler.s -z8002 --defsym NOPS=127
#source: jr-opcode.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#error: .*: relocation truncated to fit: r_jr against `target'
