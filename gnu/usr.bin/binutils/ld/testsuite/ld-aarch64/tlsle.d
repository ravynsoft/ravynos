#source: tlsle.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -e0
#objdump: -dr

.*: .*

Disassembly of section .text:

0+10000 <.text>:
 +10000:	d2a00000 	movz	x0, #0x0, lsl #16
 +10004:	f2800200 	movk	x0, #0x10
 +10008:	d2a00000 	movz	x0, #0x0, lsl #16
 +1000c:	f28fffe0 	movk	x0, #0x7fff
 +10010:	d2a00000 	movz	x0, #0x0, lsl #16
 +10014:	f2900000 	movk	x0, #0x8000
 +10018:	d2a00000 	movz	x0, #0x0, lsl #16
 +1001c:	f29fffe0 	movk	x0, #0xffff
 +10020:	d2a00020 	mov	x0, #0x10000               	// #65536
 +10024:	f2800000 	movk	x0, #0x0
 +10028:	d2afffe0 	mov	x0, #0x7fff0000            	// #2147418112
 +1002c:	f29fffe0 	movk	x0, #0xffff
 +10030:	d2b00000 	mov	x0, #0x80000000            	// #2147483648
 +10034:	f2800000 	movk	x0, #0x0
 +10038:	d2bfffe0 	mov	x0, #0xffff0000            	// #4294901760
 +1003c:	f2800000 	movk	x0, #0x0
 +10040:	d2bfffe0 	mov	x0, #0xffff0000            	// #4294901760
 +10044:	f29fffe0 	movk	x0, #0xffff
 +10048:	d2c00000 	movz	x0, #0x0, lsl #32
 +1004c:	f2a00000 	movk	x0, #0x0, lsl #16
 +10050:	f2800200 	movk	x0, #0x10
 +10054:	d2c00000 	movz	x0, #0x0, lsl #32
 +10058:	f2a00000 	movk	x0, #0x0, lsl #16
 +1005c:	f28fffe0 	movk	x0, #0x7fff
 +10060:	d2c00000 	movz	x0, #0x0, lsl #32
 +10064:	f2a00000 	movk	x0, #0x0, lsl #16
 +10068:	f2900000 	movk	x0, #0x8000
 +1006c:	d2c00000 	movz	x0, #0x0, lsl #32
 +10070:	f2a00000 	movk	x0, #0x0, lsl #16
 +10074:	f29fffe0 	movk	x0, #0xffff
 +10078:	d2c00000 	movz	x0, #0x0, lsl #32
 +1007c:	f2a00020 	movk	x0, #0x1, lsl #16
 +10080:	f2800000 	movk	x0, #0x0
 +10084:	d2c00000 	movz	x0, #0x0, lsl #32
 +10088:	f2afffe0 	movk	x0, #0x7fff, lsl #16
 +1008c:	f29fffe0 	movk	x0, #0xffff
 +10090:	d2c00000 	movz	x0, #0x0, lsl #32
 +10094:	f2b00000 	movk	x0, #0x8000, lsl #16
 +10098:	f2800000 	movk	x0, #0x0
 +1009c:	d2c00000 	movz	x0, #0x0, lsl #32
 +100a0:	f2bfffe0 	movk	x0, #0xffff, lsl #16
 +100a4:	f2800000 	movk	x0, #0x0
 +100a8:	d2c00000 	movz	x0, #0x0, lsl #32
 +100ac:	f2bfffe0 	movk	x0, #0xffff, lsl #16
 +100b0:	f29fffe0 	movk	x0, #0xffff
 +100b4:	d2c00020 	mov	x0, #0x100000000           	// #4294967296
 +100b8:	f2a00000 	movk	x0, #0x0, lsl #16
 +100bc:	f2824660 	movk	x0, #0x1233
 +100c0:	d2c24680 	mov	x0, #0x123400000000        	// #20014547599360
 +100c4:	f2aacf00 	movk	x0, #0x5678, lsl #16
 +100c8:	f2921560 	movk	x0, #0x90ab
 +100cc:	d2c24680 	mov	x0, #0x123400000000        	// #20014547599360
 +100d0:	f2bfffe0 	movk	x0, #0xffff, lsl #16
 +100d4:	f2800000 	movk	x0, #0x0
 +100d8:	d2c24680 	mov	x0, #0x123400000000        	// #20014547599360
 +100dc:	f2bfffe0 	movk	x0, #0xffff, lsl #16
 +100e0:	f29fffc0 	movk	x0, #0xfffe
 +100e4:	d2d00000 	mov	x0, #0x800000000000        	// #140737488355328
 +100e8:	f2a00000 	movk	x0, #0x0, lsl #16
 +100ec:	f2800020 	movk	x0, #0x1
 +100f0:	d2dfffe0 	mov	x0, #0xffff00000000        	// #281470681743360
 +100f4:	f2bfffe0 	movk	x0, #0xffff, lsl #16
 +100f8:	f29fffe0 	movk	x0, #0xffff
 +100fc:	d65f03c0 	ret
