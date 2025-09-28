#as:
#objdump: -d

#...
.*loop.*a2,.*
.*rsr.lend.*a2
.*wsr.lbeg.*a2
.*l32r.*a2,.*
.*nop
.*wsr.lend.*a2
.*isync
.*rsr.lcount.*a2
.*addi.*a2, a2, 1
#...
