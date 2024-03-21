# Check that expressions that generate relocations work when the symbol is a constant.
#name: PR27217
#objdump: -rd
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+000 <.*>:
[ 	]+0:[ 	]+90000000[ 	]+adrp[ 	]+x0, [0-9]*[ 	]+<.*>
[ 	]+0:[ 	]+R_AARCH64(|_P32)_ADR_PREL_PG_HI21[ 	]+\*ABS\*\+0x12345678
[ 	]+4:[ 	]+91000000[ 	]+add[ 	]+x0, x0, #0x0
[ 	]+4:[ 	]+R_AARCH64(|_P32)_ADD_ABS_LO12_NC[ 	]+\*ABS\*\+0x12345678
[ 	]+8:[ 	]+d65f03c0[ 	]+ret
#pass
