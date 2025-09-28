#objdump: -d --prefix-addresses --reloc
#as: 
#name: all_insns

# Test handling of basic instructions.

.*: +file format elf32\-xgate

Disassembly of section .text:
0+0000 <L0> adc R1, R2, R3
0+0002 <L1> bcc \*230  Abs\* 0x000000e8 <END_CODE>
0+0004 <L2> add R4, R5, R6
0+0006 <L3> addl R7, #0xe1
0+0008 <L3\+0x2> addh R7, #0x00 Abs\* 0x000000e1 <L103\+0x1>
0+000a <L4> addh R1, #0xff
0+000c <L5> addl R2, #0xff Abs\* 0x0000ffff <END_CODE\+0xff17>
0+000e <L6> addl R4, #0x44
0+0010 <L6\+0x2> addh R4, #0x1f Abs\* 0x00001f44 <END_CODE\+0x1e5c>
0+0012 <L7> and R3, R4, R5
0+0014 <L8> andl R1, #0x04
0+0016 <L8\+0x2> andh R1, #0x80 Abs\* 0x00008004 <END_CODE\+0x7f1c>
0+0018 <L9> addl R5, #0xe8
			18: R_XGATE_IMM8_LO	.text
0+001a <L9\+0x2> addh R5, #0x00 Abs\* 0x000000e8 <END_CODE>
			1a: R_XGATE_IMM8_HI	.text
0+001c <L10> andl R7, #0xe8
			1c: R_XGATE_IMM8_LO	.text
0+001e <L10\+0x2> andh R7, #0x00 Abs\* 0x000000e8 <END_CODE>
			1e: R_XGATE_IMM8_HI	.text
0+0020 <L11> andl R4, #0x01
0+0022 <L11\+0x2> andh R4, #0xff Abs\* 0x0000ff01 <END_CODE\+0xfe19>
0+0024 <L12> andl R3, #0x01
0+0026 <L13> andh R6, #0xff Abs\* 0x0000ff01 <END_CODE\+0xfe19>
0+0028 <L14> asr R0, #0x03
0+002a <L15> asr R1, R2
0+002c <L16> bcc \*188  Abs\* 0x000000e8 <END_CODE>
0+002e <L17> bcs \*186  Abs\* 0x000000e8 <END_CODE>
0+0030 <L18> beq \*184  Abs\* 0x000000e8 <END_CODE>
0+0032 <L19> bfext R3, R4, R5
0+0034 <L20> bffo R6, R7
0+0036 <L21> bfins R0, R1, R2
0+0038 <L22> bfinsi R3, R4, R5
0+003a <L23> bfinsx R6, R7, R0
0+003c <L24> bge \*172  Abs\* 0x000000e8 <END_CODE>
0+003e <L25> bgt \*170  Abs\* 0x000000e8 <END_CODE>
0+0040 <L26> bhi \*168  Abs\* 0x000000e8 <END_CODE>
0+0042 <L27> bcc \*166  Abs\* 0x000000e8 <END_CODE>
0+0044 <L28> bith R1, #0x20
0+0046 <L29> bitl R2, #0x00
0+0048 <L30> ble \*160  Abs\* 0x000000e8 <END_CODE>
0+004a <L31> bcs \*158  Abs\* 0x000000e8 <END_CODE>
0+004c <L32> bls \*156  Abs\* 0x000000e8 <END_CODE>
0+004e <L33> blt \*154  Abs\* 0x000000e8 <END_CODE>
0+0050 <L34> bmi \*152  Abs\* 0x000000e8 <END_CODE>
0+0052 <L35> bne \*150  Abs\* 0x000000e8 <END_CODE>
0+0054 <L36> bpl \*148  Abs\* 0x000000e8 <END_CODE>
0+0056 <L37> bra \*146  Abs\* 0x000000e8 <END_CODE>
	...
0+005a <L39> bvc \*142  Abs\* 0x000000e8 <END_CODE>
0+005c <L40> bvs \*140  Abs\* 0x000000e8 <END_CODE>
0+005e <L41> sub R0, R1, R2
0+0060 <L42> cmpl R3, #0xff
0+0062 <L43> xnor R4, R0, R5
0+0064 <L44> sbc R0, R6, R7
0+0066 <L45> cmpl R1, #0xff Abs\* 0x0000ffdd <END_CODE\+0xfef5>
0+0068 <L45\+0x2> cpch R1, #0xff
0+006a <L46> cpch R2, #0xff Abs\* 0x0000ffff <END_CODE\+0xff17>
0+006c <L47> csem #0x4
0+006e <L48> csem R5
0+0070 <L49> csl R6, #0x0b
0+0072 <L50> csl R7, R0
0+0074 <L51> csr R1, #0x02
0+0076 <L52> csr R2, R3
0+0078 <L53> jal R4
0+007a <L54> ldb R5, \(R6, #0x14\)
0+007c <L55> ldb R7, \(R0, R1\+\)
0+007e <L56> ldb R7, \(R0, \-R1\)
0+0080 <L57> ldb R0, \(R0, R0\)
0+0082 <L58> ldh R1, #0xff
0+0084 <L59> ldl R2, #0xff Abs\* 0x0000ffff <END_CODE\+0xff17>
0+0086 <L60> ldl R3, #0xe8
			86: R_XGATE_IMM8_LO	.text
0+0088 <L60\+0x2> ldh R3, #0x00 Abs\* 0x000000e8 <END_CODE>
			88: R_XGATE_IMM8_HI	.text
0+008a <L61> ldw R4, \(R5, #0x14\)
0+008c <L62> ldw R5, \(R6, R7\+\)
0+008e <L63> ldw R5, \(R6, \-R7\)
0+0090 <L64> ldw R1, \(R2, R4\)
0+0092 <L65> lsl R1, #0x04
0+0094 <L66> lsl R2, R3
0+0096 <L67> lsr R4, #0x05
0+0098 <L68> lsr R5, R6
0+009a <L69> or R6, R0, R7
0+009c <L70> sub R1, R0, R2
0+009e <L71> nop
0+00a0 <L72> or R1, R2, R3
0+00a2 <L73> orh R4, #0xff
0+00a4 <L74> orl R5, #0xff
0+00a6 <L75> par R6
0+00a8 <L76> rol R7, #0x06
0+00aa <L77> rol R1, R2
0+00ac <L78> ror R3, #0x05
0+00ae <L79> ror R4, R5
0+00b0 <L80> rts
0+00b2 <L81> sbc R1, R2, R3
0+00b4 <L82> ssem #0x4
0+00b6 <L83> ssem R1
0+00b8 <L84> sex R2
0+00ba <L85> sif
0+00bc <L86> sif R4
0+00be <L87> stb R5, \(R6, #0x5\)
0+00c0 <L88> stb R0, \(R0, R0\+\)
0+00c2 <L89> stb R0, \(R0, \-R0\)
0+00c4 <L90> stb R2, \(R0, R0\)
0+00c6 <L91> stw R1, \(R2, #0x10\)
0+00c8 <L92> stw R1, \(R2, R3\+\)
0+00ca <L93> stw R1, \(R2, \-R3\)
0+00cc <L94> stw R2, \(R3, R4\)
0+00ce <L95> sub R3, R4, R6
0+00d0 <L96> subl R4, #0xff
0+00d2 <L96\+0x2> subh R4, #0xff Abs\* 0x0000ffff <END_CODE\+0xff17>
0+00d4 <L97> subh R5, #0xff
0+00d6 <L98> subl R6, #0xff Abs\* 0x0000ffff <END_CODE\+0xff17>
0+00d8 <L99> tfr R7, PC
0+00da <L100> tfr R7, CCR
0+00dc <L101> tfr CCR, R7
0+00de <L102> sub R0, R1, R0
0+00e0 <L103> xnor R1, R2, R3
0+00e2 <L104> xnorh R4, #0xff
0+00e4 <L105> xnorl R5, #0xff
0+00e6 <L106> xnor R3, R0, R3