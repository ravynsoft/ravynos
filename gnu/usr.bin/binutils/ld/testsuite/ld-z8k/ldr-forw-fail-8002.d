#name: Z8002 forward relative load just out of range
#source: ldr-opcode.s -z8002
#source: 0filler.s -z8002 --defsym BYTES=32768
#source: branch-target.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#error: .*: relocation truncated to fit: r_rel16 against `target'
