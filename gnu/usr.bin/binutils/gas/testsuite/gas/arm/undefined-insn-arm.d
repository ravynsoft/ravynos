#name: Undefined binary printing in arm mode
#skip: *-*-pe *-*-vxworks
#source: undefined-insn.s
#objdump: -D -b binary -m armv7e-m
#...
