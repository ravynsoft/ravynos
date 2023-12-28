.syntax unified

.arm

@ LDRD
ldrd r0,r1,[r0,r1]			@ unpredictable
ldrd r0,r1,[r1,r0]			@ ditto
ldrd r0,r1,[r0,r2]!			@ ditto
ldrd r0,r1,[r1,r2]!			@ ditto

@ STRD

strd r0,r1,[r0,r2]!			@ ditto
strd r0,r1,[r1,r2]!			@ ditto
