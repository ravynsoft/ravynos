#name: Z8001 forward relative byte load just out of range
#source: ldrb-opcode2.s -z8001
#source: 0filler.s -z8001 --defsym BYTES=32766
#source: branch-target.s -z8001
#ld: -T reloc.ld -mz8001 -e 0
#error: .*: relocation truncated to fit: r_rel16 against `target'
