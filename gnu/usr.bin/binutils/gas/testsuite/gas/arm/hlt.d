#objdump: -d
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
[^:]+:\s+ba80      	hlt	0x0000
[^:]+:\s+ba8f      	hlt	0x000f
[^:]+:\s+e1000070 	hlt	0x0000
[^:]+:\s+e100007f 	hlt	0x000f
[^:]+:\s+ba80      	hlt	0x0000
[^:]+:\s+ba8f      	hlt	0x000f
[^:]+:\s+e1000070 	hlt	0x0000
[^:]+:\s+e100007f 	hlt	0x000f
[^:]+:\s+ba80      	hlt	0x0000
[^:]+:\s+ba8f      	hlt	0x000f
[^:]+:\s+e1000070 	hlt	0x0000
[^:]+:\s+e100007f 	hlt	0x000f
[^:]+:\s+ba80      	hlt	0x0000
[^:]+:\s+ba8f      	hlt	0x000f
[^:]+:\s+e1000070 	hlt	0x0000
[^:]+:\s+e100007f 	hlt	0x000f
[^:]+:\s+ba80      	hlt	0x0000
[^:]+:\s+ba8f      	hlt	0x000f
[^:]+:\s+e1000070 	hlt	0x0000
[^:]+:\s+e100007f 	hlt	0x000f
[^:]+:\s+e1000070 	hlt	0x0000
[^:]+:\s+e100007f 	hlt	0x000f
[^:]+:\s+e1000070 	hlt	0x0000
[^:]+:\s+e100007f 	hlt	0x000f
[^:]+:\s+e1000070 	hlt	0x0000
[^:]+:\s+e100007f 	hlt	0x000f
