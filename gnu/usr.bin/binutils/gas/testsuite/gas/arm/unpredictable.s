        .text
        .global upredictable
unpredictable:
        .word   0x004f00b1      @ strheq  r0, [pc], #-1
        .word   0x005fffff      @ ldrsheq pc, [pc], #-255
        .word   0x007fffff      @ ldrsheq pc, [pc, #-255]!
        .word   0x00cf00b0      @ strheq  r0, [pc], #0
        .word   0x00df00b0	@ ldrheq  r0, [pc], #0
        .word   0x00dfffff	@ ldrsheq pc, [pc], #255
        .word   0x00ffffff      @ ldrsheq pc, [pc, #255]
        .word   0x0000f0b0      @ strheq  pc, [r0], -r0
        .word   0x000ff0be      @ strheq  pc, [pc], -lr
        .word   0xe16fff10      @ clz     pc, r0
        .word   0xe16f0f1f      @ clz     r0, r15

        .word   0xe99f0001      @ ldmib   r15, { r0 }
        .word   0xe9910000      @ ldmib   r1, { }
        .word   0xe89f0002      @ ldmia   pc, { r1 }
        .word   0xe93f0004      @ ldmdb   r15!, { r2 }
        .word   0xe83f0008      @ ldmda   pc!, { r3 }

        .word   0xe7d0f001      @ ldrb    pc, [r0, r1]
        .word   0xe6f0f001      @ ldrbt   pc, [r0], r1
        .word   0xe190f0b1      @ ldrh    pc, [r0, r1]
        .word   0xe190f0d1      @ ldrsb   pc, [r0, r1]
        .word   0xe010f0d0      @ ldrsb   pc, [r0], -r0
        .word   0xe190f0f1      @ ldrsh   pc, [r0, r1]
        .word   0xe6b0f001      @ ldrt    pc, [r0], r1

        .word   0xe020f291      @ mla     r0, r1, r2, pc
        .word   0xe0202f91      @ mla     r0, r1, pc, r2
        .word   0xe020219f      @ mla     r0, pc, r1, r2
        .word   0xe02f2190      @ mla     pc, r0, r1, r2
				  
        .word   0xe10ff000      @ mrs     pc, cpsr	  
				  
        .word   0xe0000f91      @ mul     r0, r1, pc	  
        .word   0xe001009f      @ mul     r0, pc, r1
        .word   0xe00f0091      @ mul     pc, r1, r0
				  
        .word   0xe0e21f93      @ smlal   r1, r2, r3, pc
        .word   0xe0e2149f      @ smlal   r1, r2, pc, r4
        .word   0xe0ef1493      @ smlal   r1, pc, r3, r4
        .word   0xe0e2f493      @ smlal   pc, r2, r3, r4
        .word   0xe0e11493      @ smlal   r1, r1, r3, r4
 				  
        .word   0xe0c21f93      @ smull   r1, r2, r3, pc
        .word   0xe0c2149f      @ smull   r1, r2, pc, r4
        .word   0xe0cf1493      @ smull   r1, pc, r3, r4
        .word   0xe0c2f493      @ smull   pc, r2, r3, r4
        .word   0xe0c11493      @ smull   r1, r1, r3, r4
				  
        .word   0xe98f0004      @ stmib   r15, { r2 }
        .word   0xe88f0008      @ stmia   r15, { r3 }
        .word   0xe92f0010      @ stmdb   r15!, { r4 }
        .word   0xe82f0020      @ stmda   r15!, { r5 }

        .word   0xe180f0b1      @ strh    pc, [r0, r1]

        .word   0xe103f092      @ swp     r15, r2, [r3]
        .word   0xe103109f      @ swp     r1, r15, [r3]
        .word   0xe10f1092      @ swp     r1, r2, [r15]
        .word   0xe1031093      @ swp     r1, r3, [r3]
        .word   0xe1033092      @ swp     r3, r2, [r3]
				  
        .word   0xe143f092      @ swpb    r15, r2, [r3]
        .word   0xe143109f      @ swpb    r1, r15, [r3]
        .word   0xe14f1092      @ swpb    r1, r2, [r15]
        .word   0xe1431093      @ swpb    r1, r3, [r3]
        .word   0xe1433092      @ swpb    r3, r2, [r3]
				  
        .word   0xe0a21f93      @ umlal   r1, r2, r3, r15
        .word   0xe0a2149f      @ umlal   r1, r2, r15, r4
        .word   0xe0af1493      @ umlal   r1, r15, r3, r4
        .word   0xe0a2f493      @ umlal   r15, r2, r3, r4
        .word   0xe0a11493      @ umlal   r1, r1, r3, r4
				  
        .word   0xe0821f93      @ umull   r1, r2, r3, r15
        .word   0xe082149f      @ umull   r1, r2, r15, r4
        .word   0xe08f1493      @ umull   r1, r15, r3, r4
        .word   0xe082f493      @ umull   r15, r2, r3, r4
        .word   0xe0811493      @ umull   r1, r1, r3, r4

	nop	@ Marker to indicated end of unpredictable insns.
