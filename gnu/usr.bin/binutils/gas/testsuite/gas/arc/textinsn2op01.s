# Insn 2op .extInstruction test
	.extInstruction myinsn, 0x07, 0x30, SUFFIX_FLAG|SUFFIX_COND, SYNTAX_2OP|OP1_IMM_IMPLIED

        myinsn r0,r1
        myinsn fp,sp

        myinsn r0,0
        myinsn r1,-1
        myinsn 0,r2
        myinsn r4,255
        myinsn r6,-256

        myinsn r8,256
        myinsn r9,-257
        myinsn r11,0x42424242

        myinsn r0,foo

        myinsn.f r0,r1
        myinsn.f r2,1
        myinsn.f 0,r4
        myinsn.f r5,512

	myinsn.ne.f 0,r4
	myinsn.c.f  0xdeadbeef, 0xdeadbeef
	myinsn.nc.f 0xdeadbeef, 0x02
