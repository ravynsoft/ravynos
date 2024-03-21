#as: --EB
#source: call.s
#objdump: -dr
#name: eBPF CALL instruction, big endian

.*: +file format .*bpf.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
   0:	85 00 00 00 00 00 00 00 	call 0
   8:	85 00 00 00 00 00 00 01 	call 1
  10:	85 00 00 00 ff ff ff fe 	call -2
  18:	85 00 00 00 00 00 00 0a 	call 10
  20:	85 01 00 00 00 00 00 00 	call 0
[0-9a-f]+ <foo>:
  28:	85 01 00 00 ff ff ff ff 	call -1
  30:	85 01 00 00 ff ff ff fe 	call -2
  38:	85 01 00 00 ff ff ff fd 	call -3