.syntax unified

.thumb

@ strexh

strexh r0, r1, #0x04
strexh r0, r1, [r2], #0x04
strexh r0, r1, [r2, #+0x00]!
strexh r0, r1, [r2, r3]
strexh r0, r0, [r1]
strexh r0, r1, [r2, #-0x04]
strexh r0, r1, [r15]
strexh r0, r13, [r1]
strexh r0, r15, [r1]
strexh r13, r0, [r1]
strexh r15, r0, [r1]

@ strexb

strexb r0, r1, #0x04
strexb r0, r1, [r2], #0x04
strexb r0, r1, [r2, #+0x00]!
strexb r0, r1, [r2, r3]
strexb r0, r0, [r1]
strexb r0, r1, [r2, #-0x04]
strexb r0, r1, [r15]
strexb r0, r13, [r1]
strexb r0, r15, [r1]
strexb r13, r0, [r1]
strexb r15, r0, [r1]

