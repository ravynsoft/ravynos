#name: Z8001 backward djnz just out of range
#source: branch-target.s -z8001
#source: branch-target2.s -z8001
#source: filler.s -z8001 --defsym NOPS=125
#source: djnz-opcode.s -z8001
#source: dbjnz-opcode.s -z8001
#ld: -T reloc.ld -mz8001 -e 0
#error: .*: relocation truncated to fit: r_disp7 against `target'
