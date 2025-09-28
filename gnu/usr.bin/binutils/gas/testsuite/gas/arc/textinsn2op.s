# Insn 3 .extInstruction test
	.extInstruction myinsn, 0x07, 0x2d, SUFFIX_FLAG, SYNTAX_2OP

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
