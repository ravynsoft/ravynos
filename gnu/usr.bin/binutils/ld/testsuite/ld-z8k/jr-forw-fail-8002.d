#name: Z8002 forward jr just out of range
#source: jr-opcode.s -z8002
#source: filler.s -z8002 --defsym NOPS=128
#source: branch-target.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#error: .*: relocation truncated to fit: r_jr against `target'
