#name: Z8002 forward relative byte load just out of range
#source: ldrb-opcode2.s -z8002
#source: 0filler.s -z8002 --defsym BYTES=32766
#source: branch-target.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#error: .*: relocation truncated to fit: r_rel16 against `target'
