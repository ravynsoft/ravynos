#objdump: -d -mm9s12xg --prefix-addresses --reloc
#as: -mm9s12xg
#name: XGATE instruction set and all modes

.*:     file format elf32-m68hc12


Disassembly of section .text:
00000000 <label1> adc R1, R2, R3
00000002 <label2> add R4, R6, R1
00000004 <label3> addl R7, #0x34
00000006 <label3\+0x2> addh R7, #0x12
00000008 <label3\+0x4> addl R4, #0xa5
0000000a <label3\+0x6> addh R4, #0x32
0000000c <label3\+0x8> and R7, R6, R5
0000000e <label3\+0xa> andl R2, #0x32
00000010 <label3\+0xc> andh R2, #0x54
00000012 <label3\+0xe> andl R1, #0xa5
00000014 <label3\+0x10> andh R1, #0x32
00000016 <label3\+0x12> asr R2, #0x3
00000018 <label3\+0x14> asr R3, R4
0000001a <label3\+0x16> bcc 0x00000000 <label1>
0000001c <label3\+0x18> bcs 0x00000002 <label2>
0000001e <label3\+0x1a> beq 0x00000004 <label3>
00000020 <label3\+0x1c> bfext R1, R2, R3
00000022 <label3\+0x1e> bffo R4, R5
00000024 <label3\+0x20> bfins R6, R7, R1
00000026 <label3\+0x22> bfinsi R2, R4, R6
00000028 <label3\+0x24> bfinsx R3, R5, R7
0000002a <label3\+0x26> bge 0x00000000 <label1>
0000002c <label3\+0x28> bgt 0x00000002 <label2>
0000002e <label3\+0x2a> bhi 0x00000004 <label3>
00000030 <label3\+0x2c> bcc 0x00000000 <label1>
00000032 <label3\+0x2e> bith R2, #0x55
00000034 <label3\+0x30> bitl R3, #0xaa
00000036 <label3\+0x32> ble 0x00000002 <label2>
00000038 <label3\+0x34> bcs 0x00000004 <label3>
0000003a <label3\+0x36> bls 0x00000000 <label1>
0000003c <label3\+0x38> blt 0x00000002 <label2>
0000003e <label3\+0x3a> bmi 0x00000004 <label3>
00000040 <label3\+0x3c> bne 0x00000000 <label1>
00000042 <label3\+0x3e> bpl 0x00000002 <label2>
00000044 <label3\+0x40> bra 0x00000004 <label3>
00000046 <label3\+0x42> brk
00000048 <label3\+0x44> bvc 0x00000000 <label1>
0000004a <label3\+0x46> bvs 0x00000002 <label2>
0000004c <label3\+0x48> sub R0, R1, R2
0000004e <label3\+0x4a> cmpl R3, #0x34
00000050 <label3\+0x4c> cpch R3, #0x12
00000052 <label3\+0x4e> cmpl R4, #0x32
00000054 <label3\+0x50> xnor R4, R0, R5
00000056 <label3\+0x52> xnor R6, R0, R6
00000058 <label3\+0x54> sbc R0, R7, R5
0000005a <label3\+0x56> cpch R6, #0xa5
0000005c <label3\+0x58> csem #0x2
0000005e <label3\+0x5a> csem R1
00000060 <label3\+0x5c> csl R2, #0x1
00000062 <label3\+0x5e> csl R3, R4
00000064 <label3\+0x60> csr R5, #0x4
00000066 <label3\+0x62> csr R6, R7
00000068 <label3\+0x64> jal R1
0000006a <label3\+0x66> ldb R2, \(R3, #0x4\)
0000006c <label3\+0x68> ldb R3, \(R0, R2\)
0000006e <label3\+0x6a> ldb R4, \(R5, R6\+\)
00000070 <label3\+0x6c> ldb R5, \(R6, -R7\)
00000072 <label3\+0x6e> ldh R6, #0x35
00000074 <label3\+0x70> ldl R7, #0x46
00000076 <label3\+0x72> ldw R1, \(R2, #0x1d\)
00000078 <label3\+0x74> ldw R2, \(R3, R0\)
0000007a <label3\+0x76> ldw R3, \(R4, R5\+\)
0000007c <label3\+0x78> ldw R4, \(R5, -R6\)
0000007e <label3\+0x7a> ldl R6, #0x34
00000080 <label3\+0x7c> ldh R6, #0x12
00000082 <label3\+0x7e> lsl R7, #0x2
00000084 <label3\+0x80> lsl R2, R1
00000086 <label3\+0x82> lsr R5, #0x3
00000088 <label3\+0x84> lsl R6, R3
0000008a <label3\+0x86> or R7, R0, R6
0000008c <label3\+0x88> sub R2, R0, R3
0000008e <label3\+0x8a> sub R4, R0, R4
00000090 <label3\+0x8c> nop
00000092 <label3\+0x8e> or R2, R3, R4
00000094 <label3\+0x90> orl R5, #0x56
00000096 <label3\+0x92> orh R5, #0x12
00000098 <label3\+0x94> orh R6, #0x08
0000009a <label3\+0x96> orl R4, #0xf0
0000009c <label3\+0x98> par R1
0000009e <label3\+0x9a> rol R2, #0x5
000000a0 <label3\+0x9c> rol R3, R4
000000a2 <label3\+0x9e> ror R3, #0x6
000000a4 <label3\+0xa0> ror R5, R4
000000a6 <label3\+0xa2> rts
000000a8 <label3\+0xa4> sbc R7, R1, R2
000000aa <label3\+0xa6> sex R1
000000ac <label3\+0xa8> sif
000000ae <label3\+0xaa> sif R2
000000b0 <label3\+0xac> ssem #0x5
000000b2 <label3\+0xae> ssem R3
000000b4 <label3\+0xb0> stb R2, \(R4, #0xf\)
000000b6 <label3\+0xb2> stb R3, \(R5, R6\)
000000b8 <label3\+0xb4> stb R0, \(R7, R1\+\)
000000ba <label3\+0xb6> stb R1, \(R2, -R3\)
000000bc <label3\+0xb8> stw R7, \(R6, #0x1e\)
000000be <label3\+0xba> stw R6, \(R5, R0\)
000000c0 <label3\+0xbc> stw R5, \(R4, R3\+\)
000000c2 <label3\+0xbe> stw R4, \(R3, -R2\)
000000c4 <label3\+0xc0> sub R7, R6, R5
000000c6 <label3\+0xc2> subl R4, #0x34
000000c8 <label3\+0xc4> subh R4, #0x12
000000ca <label3\+0xc6> subh R5, #0x44
000000cc <label3\+0xc8> subl R4, #0x55
000000ce <label3\+0xca> tfr R2, CCR
000000d0 <label3\+0xcc> tfr CCR, R3
000000d2 <label3\+0xce> tfr R5, PC
000000d4 <label3\+0xd0> sub R0, R2, R0
000000d6 <label3\+0xd2> xnor R4, R6, R2
000000d8 <label3\+0xd4> xnorl R3, #0x32
000000da <label3\+0xd6> xnorh R3, #0x54
000000dc <label3\+0xd8> xnorh R2, #0x32
000000de <label3\+0xda> xnorl R1, #0x54
