#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS CP2 64-bit move instructions
#as: -32
#source: cp2-64\.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 0000 7d3c 	dmtc2	zero,\$0
[0-9a-f]+ <[^>]*> 0001 7d3c 	dmtc2	zero,\$1
[0-9a-f]+ <[^>]*> 0002 7d3c 	dmtc2	zero,\$2
[0-9a-f]+ <[^>]*> 0003 7d3c 	dmtc2	zero,\$3
[0-9a-f]+ <[^>]*> 0004 7d3c 	dmtc2	zero,\$4
[0-9a-f]+ <[^>]*> 0005 7d3c 	dmtc2	zero,\$5
[0-9a-f]+ <[^>]*> 0006 7d3c 	dmtc2	zero,\$6
[0-9a-f]+ <[^>]*> 0007 7d3c 	dmtc2	zero,\$7
[0-9a-f]+ <[^>]*> 0008 7d3c 	dmtc2	zero,\$8
[0-9a-f]+ <[^>]*> 0009 7d3c 	dmtc2	zero,\$9
[0-9a-f]+ <[^>]*> 000a 7d3c 	dmtc2	zero,\$10
[0-9a-f]+ <[^>]*> 000b 7d3c 	dmtc2	zero,\$11
[0-9a-f]+ <[^>]*> 000c 7d3c 	dmtc2	zero,\$12
[0-9a-f]+ <[^>]*> 000d 7d3c 	dmtc2	zero,\$13
[0-9a-f]+ <[^>]*> 000e 7d3c 	dmtc2	zero,\$14
[0-9a-f]+ <[^>]*> 000f 7d3c 	dmtc2	zero,\$15
[0-9a-f]+ <[^>]*> 0010 7d3c 	dmtc2	zero,\$16
[0-9a-f]+ <[^>]*> 0011 7d3c 	dmtc2	zero,\$17
[0-9a-f]+ <[^>]*> 0012 7d3c 	dmtc2	zero,\$18
[0-9a-f]+ <[^>]*> 0013 7d3c 	dmtc2	zero,\$19
[0-9a-f]+ <[^>]*> 0014 7d3c 	dmtc2	zero,\$20
[0-9a-f]+ <[^>]*> 0015 7d3c 	dmtc2	zero,\$21
[0-9a-f]+ <[^>]*> 0016 7d3c 	dmtc2	zero,\$22
[0-9a-f]+ <[^>]*> 0017 7d3c 	dmtc2	zero,\$23
[0-9a-f]+ <[^>]*> 0018 7d3c 	dmtc2	zero,\$24
[0-9a-f]+ <[^>]*> 0019 7d3c 	dmtc2	zero,\$25
[0-9a-f]+ <[^>]*> 001a 7d3c 	dmtc2	zero,\$26
[0-9a-f]+ <[^>]*> 001b 7d3c 	dmtc2	zero,\$27
[0-9a-f]+ <[^>]*> 001c 7d3c 	dmtc2	zero,\$28
[0-9a-f]+ <[^>]*> 001d 7d3c 	dmtc2	zero,\$29
[0-9a-f]+ <[^>]*> 001e 7d3c 	dmtc2	zero,\$30
[0-9a-f]+ <[^>]*> 001f 7d3c 	dmtc2	zero,\$31
[0-9a-f]+ <[^>]*> 0000 6d3c 	dmfc2	zero,\$0
[0-9a-f]+ <[^>]*> 0001 6d3c 	dmfc2	zero,\$1
[0-9a-f]+ <[^>]*> 0002 6d3c 	dmfc2	zero,\$2
[0-9a-f]+ <[^>]*> 0003 6d3c 	dmfc2	zero,\$3
[0-9a-f]+ <[^>]*> 0004 6d3c 	dmfc2	zero,\$4
[0-9a-f]+ <[^>]*> 0005 6d3c 	dmfc2	zero,\$5
[0-9a-f]+ <[^>]*> 0006 6d3c 	dmfc2	zero,\$6
[0-9a-f]+ <[^>]*> 0007 6d3c 	dmfc2	zero,\$7
[0-9a-f]+ <[^>]*> 0008 6d3c 	dmfc2	zero,\$8
[0-9a-f]+ <[^>]*> 0009 6d3c 	dmfc2	zero,\$9
[0-9a-f]+ <[^>]*> 000a 6d3c 	dmfc2	zero,\$10
[0-9a-f]+ <[^>]*> 000b 6d3c 	dmfc2	zero,\$11
[0-9a-f]+ <[^>]*> 000c 6d3c 	dmfc2	zero,\$12
[0-9a-f]+ <[^>]*> 000d 6d3c 	dmfc2	zero,\$13
[0-9a-f]+ <[^>]*> 000e 6d3c 	dmfc2	zero,\$14
[0-9a-f]+ <[^>]*> 000f 6d3c 	dmfc2	zero,\$15
[0-9a-f]+ <[^>]*> 0010 6d3c 	dmfc2	zero,\$16
[0-9a-f]+ <[^>]*> 0011 6d3c 	dmfc2	zero,\$17
[0-9a-f]+ <[^>]*> 0012 6d3c 	dmfc2	zero,\$18
[0-9a-f]+ <[^>]*> 0013 6d3c 	dmfc2	zero,\$19
[0-9a-f]+ <[^>]*> 0014 6d3c 	dmfc2	zero,\$20
[0-9a-f]+ <[^>]*> 0015 6d3c 	dmfc2	zero,\$21
[0-9a-f]+ <[^>]*> 0016 6d3c 	dmfc2	zero,\$22
[0-9a-f]+ <[^>]*> 0017 6d3c 	dmfc2	zero,\$23
[0-9a-f]+ <[^>]*> 0018 6d3c 	dmfc2	zero,\$24
[0-9a-f]+ <[^>]*> 0019 6d3c 	dmfc2	zero,\$25
[0-9a-f]+ <[^>]*> 001a 6d3c 	dmfc2	zero,\$26
[0-9a-f]+ <[^>]*> 001b 6d3c 	dmfc2	zero,\$27
[0-9a-f]+ <[^>]*> 001c 6d3c 	dmfc2	zero,\$28
[0-9a-f]+ <[^>]*> 001d 6d3c 	dmfc2	zero,\$29
[0-9a-f]+ <[^>]*> 001e 6d3c 	dmfc2	zero,\$30
[0-9a-f]+ <[^>]*> 001f 6d3c 	dmfc2	zero,\$31
	\.\.\.
