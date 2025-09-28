#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS MIPS32 instructions
#source: mips32.s
#as: -32

# Check MIPS32 instruction assembly (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0022 4b3c 	clo	at,v0
[0-9a-f]+ <[^>]*> 0064 5b3c 	clz	v1,a0
[0-9a-f]+ <[^>]*> 00c5 cb3c 	madd	a1,a2
[0-9a-f]+ <[^>]*> 0107 db3c 	maddu	a3,t0
[0-9a-f]+ <[^>]*> 0149 eb3c 	msub	t1,t2
[0-9a-f]+ <[^>]*> 018b fb3c 	msubu	t3,t4
[0-9a-f]+ <[^>]*> 01ee 6a10 	mul	t5,t6,t7
[0-9a-f]+ <[^>]*> 6090 2000 	pref	0x4,0\(s0\)
[0-9a-f]+ <[^>]*> 6091 27ff 	pref	0x4,2047\(s1\)
[0-9a-f]+ <[^>]*> 6092 2800 	pref	0x4,-2048\(s2\)
[0-9a-f]+ <[^>]*> 0000 0800 	ssnop
[0-9a-f]+ <[^>]*> 20a1 6000 	cache	0x5,0\(at\)
[0-9a-f]+ <[^>]*> 20a2 67ff 	cache	0x5,2047\(v0\)
[0-9a-f]+ <[^>]*> 20a3 6800 	cache	0x5,-2048\(v1\)
[0-9a-f]+ <[^>]*> 5020 8000 	li	at,0x8000
[0-9a-f]+ <[^>]*> 0081 0950 	addu	at,at,a0
[0-9a-f]+ <[^>]*> 20a1 6000 	cache	0x5,0\(at\)
[0-9a-f]+ <[^>]*> 3020 8000 	li	at,-32768
[0-9a-f]+ <[^>]*> 00a1 0950 	addu	at,at,a1
[0-9a-f]+ <[^>]*> 20a1 6fff 	cache	0x5,-1\(at\)
[0-9a-f]+ <[^>]*> 5020 8000 	li	at,0x8000
[0-9a-f]+ <[^>]*> 20a1 6000 	cache	0x5,0\(at\)
[0-9a-f]+ <[^>]*> 3020 8000 	li	at,-32768
[0-9a-f]+ <[^>]*> 20a1 6fff 	cache	0x5,-1\(at\)
[0-9a-f]+ <[^>]*> 0000 f37c 	eret
[0-9a-f]+ <[^>]*> 0000 037c 	tlbp
[0-9a-f]+ <[^>]*> 0000 137c 	tlbr
[0-9a-f]+ <[^>]*> 0000 237c 	tlbwi
[0-9a-f]+ <[^>]*> 0000 337c 	tlbwr
[0-9a-f]+ <[^>]*> 0000 937c 	wait
[0-9a-f]+ <[^>]*> 0000 937c 	wait
[0-9a-f]+ <[^>]*> 0345 937c 	wait	0x345
[0-9a-f]+ <[^>]*> 4680      	break
[0-9a-f]+ <[^>]*> 4680      	break
[0-9a-f]+ <[^>]*> 0345 0007 	break	0x345
[0-9a-f]+ <[^>]*> 0048 d147 	break	0x48,0x345
[0-9a-f]+ <[^>]*> 46c0      	sdbbp
[0-9a-f]+ <[^>]*> 46c0      	sdbbp
[0-9a-f]+ <[^>]*> 0345 db7c 	sdbbp	0x345
	\.\.\.
