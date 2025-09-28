#name: Z8002 forward calr just out of range
#source: calr-opcode.s -z8002
#source: filler.s -z8002 --defsym NOPS=2049
#source: branch-target.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#error: .*: relocation truncated to fit: r_callr against `target'
