#name: Z8002 forward dbjnz just out of range
#source: dbjnz-opcode.s -z8002
#source: filler.s -z8002 --defsym NOPS=1
#source: branch-target2.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#error: .*: relocation truncated to fit: r_disp7 against `target2'
