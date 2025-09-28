#name: Z8001 backward calr just out of range
#source: branch-target.s -z8001
#source: filler.s -z8001 --defsym NOPS=2046
#source: calr-opcode.s -z8001
#ld: -T reloc.ld -mz8001 -e 0
#error: .*: relocation truncated to fit: r_callr against `target'
