#name: Z8002 backward relative load just out of range
#source: branch-target.s -z8002
#source: 0filler.s -z8002 --defsym BYTES=32764
#source: ldr-opcode.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#error: .*: relocation truncated to fit: r_rel16 against `target'
