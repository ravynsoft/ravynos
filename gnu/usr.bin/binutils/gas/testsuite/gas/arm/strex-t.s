.syntax unified
.thumb
	strexh r0, r1, [r2]
	strexh r0, r1, [r2, #+0x00]
	strexh r0, r1, [r13]

	strexb r0, r1, [r2]
	strexb r0, r1, [r2, #+0x00]
	strexb r0, r1, [r13]

