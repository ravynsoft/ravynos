#name: Z8001 backward jr just out of range
#source: branch-target.s -z8001
#source: filler.s -z8001 --defsym NOPS=127
#source: jr-opcode.s -z8001
#ld: -T reloc.ld -mz8001 -e 0
#error: .*: relocation truncated to fit: r_jr against `target'
