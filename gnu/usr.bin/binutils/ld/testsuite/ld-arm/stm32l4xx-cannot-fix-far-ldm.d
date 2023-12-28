#source: stm32l4xx-cannot-fix-far-ldm.s
#as:-EL -mcpu=cortex-m4 -mfpu=fpv4-sp-d16
#ld:-EL --fix-stm32l4xx-629360 -Ttext=0x80000
#objdump: -dr --prefix-addresses --show-raw-insn
#name: STM32L4XX erratum : LDM cannot be patched when LDM is too far from veneer section
#warning: .*cannot create STM32L4XX veneer; jump out of range by 24 bytes; cannot encode branch instruction.*

# Test the `LDM*' instructions when too far from the veneer section
# They cannot, thus should not, be patched

.*: +file format .*arm.*

Disassembly of section \.text:
00080000 <__stm32l4xx_veneer_0> 4607[[:space:]]+mov[[:space:]]+r7, r0
00080002 <__stm32l4xx_veneer_0\+0x2> e8b7 007e[[:space:]]+ldmia\.w[[:space:]]+r7\!, {r1, r2, r3, r4, r5, r6}
00080006 <__stm32l4xx_veneer_0\+0x6> e897 0380[[:space:]]+ldmia\.w[[:space:]]+r7, {r7, r8, r9}
0008000a <__stm32l4xx_veneer_0\+0xa> f3ff 978b[[:space:]]+b\.w[[:space:]]+0107ff24 <__stm32l4xx_veneer_0_r>
0008000e <__stm32l4xx_veneer_0\+0xe> de00[[:space:]]+udf[[:space:]]+#0
	\.\.\.
	\.\.\.
0107ff20 <_start\+0xffff00> f400 906e[[:space:]]+b\.w[[:space:]]+00080000 <__stm32l4xx_veneer_0>
	\.\.\.
01080024 <__stm32l4xx_veneer_0_r\+0x100> e899 03fe[[:space:]]+ldmia\.w[[:space:]]+r9, {r1, r2, r3, r4, r5, r6, r7, r8, r9}
01080028 <__stm32l4xx_veneer_1_r> bf00[[:space:]]+nop

