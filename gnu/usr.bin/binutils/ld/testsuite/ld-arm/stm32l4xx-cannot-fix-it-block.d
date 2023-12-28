#source: stm32l4xx-cannot-fix-it-block.s
#as:-EL -mcpu=cortex-m4 -mfpu=fpv4-sp-d16
#ld:-EL --fix-stm32l4xx-629360 -Ttext=0x8000
#objdump: -dr --prefix-addresses --show-raw-insn
#name: STM32L4XX erratum : LDM cannot be patched when not last in IT block
#warning: .*multiple load detected in non-last IT block instruction.*

# Test the `LDM*' instructions when non-last in IT block
# They cannot, thus should not, be patched

.*: +file format .*arm.*

Disassembly of section \.text:
00008000 \<_start\> bf04[[:space:]]+itt[[:space:]]+eq
00008002 \<_start\+0x2\> e899 03fe[[:space:]]+ldmiaeq\.w[[:space:]]+r9, {r1, r2, r3, r4, r5, r6, r7, r8, r9}
00008006 \<_start\+0x6\> f3af 8000[[:space:]]+nopeq\.w
