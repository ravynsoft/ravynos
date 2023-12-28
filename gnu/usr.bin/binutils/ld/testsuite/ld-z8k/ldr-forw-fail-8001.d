#name: Z8001 forward relative load just out of range
#source: ldr-opcode.s -z8001
#source: 0filler.s -z8001 --defsym BYTES=32768
#source: branch-target.s -z8001
#ld: -T reloc.ld -mz8001 -e 0
#error: .*: relocation truncated to fit: r_rel16 against `target'
