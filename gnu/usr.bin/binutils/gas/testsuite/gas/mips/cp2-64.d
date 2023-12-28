#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP2 64-bit move instructions
#as: -32

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 48a00000 	dmtc2	zero,\$0
[0-9a-f]+ <[^>]*> 48a00800 	dmtc2	zero,\$1
[0-9a-f]+ <[^>]*> 48a01000 	dmtc2	zero,\$2
[0-9a-f]+ <[^>]*> 48a01800 	dmtc2	zero,\$3
[0-9a-f]+ <[^>]*> 48a02000 	dmtc2	zero,\$4
[0-9a-f]+ <[^>]*> 48a02800 	dmtc2	zero,\$5
[0-9a-f]+ <[^>]*> 48a03000 	dmtc2	zero,\$6
[0-9a-f]+ <[^>]*> 48a03800 	dmtc2	zero,\$7
[0-9a-f]+ <[^>]*> 48a04000 	dmtc2	zero,\$8
[0-9a-f]+ <[^>]*> 48a04800 	dmtc2	zero,\$9
[0-9a-f]+ <[^>]*> 48a05000 	dmtc2	zero,\$10
[0-9a-f]+ <[^>]*> 48a05800 	dmtc2	zero,\$11
[0-9a-f]+ <[^>]*> 48a06000 	dmtc2	zero,\$12
[0-9a-f]+ <[^>]*> 48a06800 	dmtc2	zero,\$13
[0-9a-f]+ <[^>]*> 48a07000 	dmtc2	zero,\$14
[0-9a-f]+ <[^>]*> 48a07800 	dmtc2	zero,\$15
[0-9a-f]+ <[^>]*> 48a08000 	dmtc2	zero,\$16
[0-9a-f]+ <[^>]*> 48a08800 	dmtc2	zero,\$17
[0-9a-f]+ <[^>]*> 48a09000 	dmtc2	zero,\$18
[0-9a-f]+ <[^>]*> 48a09800 	dmtc2	zero,\$19
[0-9a-f]+ <[^>]*> 48a0a000 	dmtc2	zero,\$20
[0-9a-f]+ <[^>]*> 48a0a800 	dmtc2	zero,\$21
[0-9a-f]+ <[^>]*> 48a0b000 	dmtc2	zero,\$22
[0-9a-f]+ <[^>]*> 48a0b800 	dmtc2	zero,\$23
[0-9a-f]+ <[^>]*> 48a0c000 	dmtc2	zero,\$24
[0-9a-f]+ <[^>]*> 48a0c800 	dmtc2	zero,\$25
[0-9a-f]+ <[^>]*> 48a0d000 	dmtc2	zero,\$26
[0-9a-f]+ <[^>]*> 48a0d800 	dmtc2	zero,\$27
[0-9a-f]+ <[^>]*> 48a0e000 	dmtc2	zero,\$28
[0-9a-f]+ <[^>]*> 48a0e800 	dmtc2	zero,\$29
[0-9a-f]+ <[^>]*> 48a0f000 	dmtc2	zero,\$30
[0-9a-f]+ <[^>]*> 48a0f800 	dmtc2	zero,\$31
[0-9a-f]+ <[^>]*> 48200000 	dmfc2	zero,\$0
[0-9a-f]+ <[^>]*> 48200800 	dmfc2	zero,\$1
[0-9a-f]+ <[^>]*> 48201000 	dmfc2	zero,\$2
[0-9a-f]+ <[^>]*> 48201800 	dmfc2	zero,\$3
[0-9a-f]+ <[^>]*> 48202000 	dmfc2	zero,\$4
[0-9a-f]+ <[^>]*> 48202800 	dmfc2	zero,\$5
[0-9a-f]+ <[^>]*> 48203000 	dmfc2	zero,\$6
[0-9a-f]+ <[^>]*> 48203800 	dmfc2	zero,\$7
[0-9a-f]+ <[^>]*> 48204000 	dmfc2	zero,\$8
[0-9a-f]+ <[^>]*> 48204800 	dmfc2	zero,\$9
[0-9a-f]+ <[^>]*> 48205000 	dmfc2	zero,\$10
[0-9a-f]+ <[^>]*> 48205800 	dmfc2	zero,\$11
[0-9a-f]+ <[^>]*> 48206000 	dmfc2	zero,\$12
[0-9a-f]+ <[^>]*> 48206800 	dmfc2	zero,\$13
[0-9a-f]+ <[^>]*> 48207000 	dmfc2	zero,\$14
[0-9a-f]+ <[^>]*> 48207800 	dmfc2	zero,\$15
[0-9a-f]+ <[^>]*> 48208000 	dmfc2	zero,\$16
[0-9a-f]+ <[^>]*> 48208800 	dmfc2	zero,\$17
[0-9a-f]+ <[^>]*> 48209000 	dmfc2	zero,\$18
[0-9a-f]+ <[^>]*> 48209800 	dmfc2	zero,\$19
[0-9a-f]+ <[^>]*> 4820a000 	dmfc2	zero,\$20
[0-9a-f]+ <[^>]*> 4820a800 	dmfc2	zero,\$21
[0-9a-f]+ <[^>]*> 4820b000 	dmfc2	zero,\$22
[0-9a-f]+ <[^>]*> 4820b800 	dmfc2	zero,\$23
[0-9a-f]+ <[^>]*> 4820c000 	dmfc2	zero,\$24
[0-9a-f]+ <[^>]*> 4820c800 	dmfc2	zero,\$25
[0-9a-f]+ <[^>]*> 4820d000 	dmfc2	zero,\$26
[0-9a-f]+ <[^>]*> 4820d800 	dmfc2	zero,\$27
[0-9a-f]+ <[^>]*> 4820e000 	dmfc2	zero,\$28
[0-9a-f]+ <[^>]*> 4820e800 	dmfc2	zero,\$29
[0-9a-f]+ <[^>]*> 4820f000 	dmfc2	zero,\$30
[0-9a-f]+ <[^>]*> 4820f800 	dmfc2	zero,\$31
	\.\.\.
