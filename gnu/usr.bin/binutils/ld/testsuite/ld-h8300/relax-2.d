# name: H8300 Relaxation Test 2
# ld: --relax
# objdump: -d --no-show-raw-insn

.*:     file format .*

Disassembly of section .text:

00000100 <_start>:
 100:	mov.b	@0x64:8,r0l
 102:	mov.b	r0l,@0x64:8
 104:	mov.b	@0x4320:16,r0l
 108:	mov.b	r0l,@0x4320:16
 10c:	mov.w	@0xff64:16,r0
 110:	mov.w	r0,@0xff64:16
 114:	mov.w	@0x4320:16,r0
 118:	mov.w	r0,@0x4320:16
 11c:	mov.l	@0xff64:16,er0
 122:	mov.l	er0,@0xff64:16
 128:	mov.l	@0x4320:16,er0
 12e:	mov.l	er0,@0x4320:16
