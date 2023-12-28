#name: Z8002 backward djnz just out of range
#source: branch-target.s -z8002
#source: branch-target2.s -z8002
#source: filler.s -z8002 --defsym NOPS=125
#source: djnz-opcode.s -z8002
#source: dbjnz-opcode.s -z8002
#ld: -T reloc.ld -mz8002 -e 0
#error: .*: relocation truncated to fit: r_disp7 against `target'
