#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP0 control register move instructions
#as: -32
#source: cp0c.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 40c00000 	ctc0	zero,\$0
[0-9a-f]+ <[^>]*> 40c00800 	ctc0	zero,\$1
[0-9a-f]+ <[^>]*> 40c01000 	ctc0	zero,\$2
[0-9a-f]+ <[^>]*> 40c01800 	ctc0	zero,\$3
[0-9a-f]+ <[^>]*> 40c02000 	ctc0	zero,\$4
[0-9a-f]+ <[^>]*> 40c02800 	ctc0	zero,\$5
[0-9a-f]+ <[^>]*> 40c03000 	ctc0	zero,\$6
[0-9a-f]+ <[^>]*> 40c03800 	ctc0	zero,\$7
[0-9a-f]+ <[^>]*> 40c04000 	ctc0	zero,\$8
[0-9a-f]+ <[^>]*> 40c04800 	ctc0	zero,\$9
[0-9a-f]+ <[^>]*> 40c05000 	ctc0	zero,\$10
[0-9a-f]+ <[^>]*> 40c05800 	ctc0	zero,\$11
[0-9a-f]+ <[^>]*> 40c06000 	ctc0	zero,\$12
[0-9a-f]+ <[^>]*> 40c06800 	ctc0	zero,\$13
[0-9a-f]+ <[^>]*> 40c07000 	ctc0	zero,\$14
[0-9a-f]+ <[^>]*> 40c07800 	ctc0	zero,\$15
[0-9a-f]+ <[^>]*> 40c08000 	ctc0	zero,\$16
[0-9a-f]+ <[^>]*> 40c08800 	ctc0	zero,\$17
[0-9a-f]+ <[^>]*> 40c09000 	ctc0	zero,\$18
[0-9a-f]+ <[^>]*> 40c09800 	ctc0	zero,\$19
[0-9a-f]+ <[^>]*> 40c0a000 	ctc0	zero,\$20
[0-9a-f]+ <[^>]*> 40c0a800 	ctc0	zero,\$21
[0-9a-f]+ <[^>]*> 40c0b000 	ctc0	zero,\$22
[0-9a-f]+ <[^>]*> 40c0b800 	ctc0	zero,\$23
[0-9a-f]+ <[^>]*> 40c0c000 	ctc0	zero,\$24
[0-9a-f]+ <[^>]*> 40c0c800 	ctc0	zero,\$25
[0-9a-f]+ <[^>]*> 40c0d000 	ctc0	zero,\$26
[0-9a-f]+ <[^>]*> 40c0d800 	ctc0	zero,\$27
[0-9a-f]+ <[^>]*> 40c0e000 	ctc0	zero,\$28
[0-9a-f]+ <[^>]*> 40c0e800 	ctc0	zero,\$29
[0-9a-f]+ <[^>]*> 40c0f000 	ctc0	zero,\$30
[0-9a-f]+ <[^>]*> 40c0f800 	ctc0	zero,\$31
[0-9a-f]+ <[^>]*> 40400000 	cfc0	zero,\$0
[0-9a-f]+ <[^>]*> 40400800 	cfc0	zero,\$1
[0-9a-f]+ <[^>]*> 40401000 	cfc0	zero,\$2
[0-9a-f]+ <[^>]*> 40401800 	cfc0	zero,\$3
[0-9a-f]+ <[^>]*> 40402000 	cfc0	zero,\$4
[0-9a-f]+ <[^>]*> 40402800 	cfc0	zero,\$5
[0-9a-f]+ <[^>]*> 40403000 	cfc0	zero,\$6
[0-9a-f]+ <[^>]*> 40403800 	cfc0	zero,\$7
[0-9a-f]+ <[^>]*> 40404000 	cfc0	zero,\$8
[0-9a-f]+ <[^>]*> 40404800 	cfc0	zero,\$9
[0-9a-f]+ <[^>]*> 40405000 	cfc0	zero,\$10
[0-9a-f]+ <[^>]*> 40405800 	cfc0	zero,\$11
[0-9a-f]+ <[^>]*> 40406000 	cfc0	zero,\$12
[0-9a-f]+ <[^>]*> 40406800 	cfc0	zero,\$13
[0-9a-f]+ <[^>]*> 40407000 	cfc0	zero,\$14
[0-9a-f]+ <[^>]*> 40407800 	cfc0	zero,\$15
[0-9a-f]+ <[^>]*> 40408000 	cfc0	zero,\$16
[0-9a-f]+ <[^>]*> 40408800 	cfc0	zero,\$17
[0-9a-f]+ <[^>]*> 40409000 	cfc0	zero,\$18
[0-9a-f]+ <[^>]*> 40409800 	cfc0	zero,\$19
[0-9a-f]+ <[^>]*> 4040a000 	cfc0	zero,\$20
[0-9a-f]+ <[^>]*> 4040a800 	cfc0	zero,\$21
[0-9a-f]+ <[^>]*> 4040b000 	cfc0	zero,\$22
[0-9a-f]+ <[^>]*> 4040b800 	cfc0	zero,\$23
[0-9a-f]+ <[^>]*> 4040c000 	cfc0	zero,\$24
[0-9a-f]+ <[^>]*> 4040c800 	cfc0	zero,\$25
[0-9a-f]+ <[^>]*> 4040d000 	cfc0	zero,\$26
[0-9a-f]+ <[^>]*> 4040d800 	cfc0	zero,\$27
[0-9a-f]+ <[^>]*> 4040e000 	cfc0	zero,\$28
[0-9a-f]+ <[^>]*> 4040e800 	cfc0	zero,\$29
[0-9a-f]+ <[^>]*> 4040f000 	cfc0	zero,\$30
[0-9a-f]+ <[^>]*> 4040f800 	cfc0	zero,\$31
	\.\.\.
