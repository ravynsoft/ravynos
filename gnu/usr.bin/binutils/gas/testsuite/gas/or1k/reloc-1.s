	l.movhi	r3,hi(x)
	l.ori	r3,r4,hi(x)
	l.addi	r3,r4,hi(x)
	l.lwz	r3,hi(x)(r4)

	l.movhi	r3,lo(x)
	l.ori	r3,r4,lo(x)
	l.addi	r3,r4,lo(x)
	l.lwz	r3,lo(x)(r4)
	l.lws	r3,lo(x)(r4)
	l.lhz	r3,lo(x)(r4)
	l.lhs	r3,lo(x)(r4)
	l.lbz	r3,lo(x)(r4)
	l.lbs	r3,lo(x)(r4)
	l.lwa	r3,lo(x)(r4)
	l.sw	lo(x)(r4),r3
	l.sh	lo(x)(r4),r3
	l.sb	lo(x)(r4),r3
	l.swa	lo(x)(r4),r3

	l.movhi	r3,ha(x)
	l.ori	r3,r4,ha(x)
	l.addi	r3,r4,ha(x)

	l.ori	r3,r0,got(x)
	l.addi	r3,r4,got(x)
	l.lwz	r3,got(x)(r4)

	l.movhi	r3,gotpchi(_GLOBAL_OFFSET_TABLE_-4)
	l.ori	r3,r3,gotpclo(_GLOBAL_OFFSET_TABLE_-8)

	l.movhi	r3,gotoffhi(x)
	l.ori	r3,r3,gotofflo(x)
	l.movhi	r3,gotoffha(x)
	l.lwz	r3,gotofflo(x)(r3)
	l.sw	gotofflo(x)(r3),r3

	l.movhi	r3,tlsgdhi(x)
	l.ori	r3,r3,tlsgdlo(x)

	l.movhi	r3,tlsldmhi(x)
	l.ori	r3,r3,tlsldmlo(x)

	l.movhi	r3,dtpoffhi(x)
	l.ori	r3,r3,dtpofflo(x)

	l.movhi	r3,gottpoffhi(x)
	l.ori	r3,r3,gottpofflo(x)
	l.movhi	r3,gottpoffha(x)
	l.lwz	r3,gottpofflo(x)(r3)

	l.movhi	r3,tpoffhi(x)
	l.ori	r3,r3,tpofflo(x)
	l.movhi	r3,tpoffha(x)
	l.lwz	r3,tpofflo(x)(r3)
	l.sw	tpofflo(x)(r3),r3

	l.j	plta(x)
	l.jal	plta(x)
	l.bf	plta(x)
	l.bnf	plta(x)

	l.adrp	r3,got(x)
	l.adrp	r3,tlsgd(x)
	l.adrp	r3,tlsldm(x)
	l.adrp	r3,gottp(x)

	l.ori	r4,r3,po(x)
	l.ori	r4,r3,gotpo(x)
	l.ori	r4,r3,tlsgdpo(x)
	l.ori	r4,r3,tlsldmpo(x)
	l.ori	r4,r3,gottppo(x)

	l.lbz	r5,po(x)(r3)
	l.lbz	r5,gotpo(x)(r3)
	l.sb	po(x)(r3),r6

	l.movhi	r4,gotha(x)
	l.ori	r3,r4,gotha(x)
	l.addi	r3,r4,gotha(x)
