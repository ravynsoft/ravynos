#name: C6X GOT relocations, local symbols, -r
#as: -mlittle-endian
#ld: -r -melf32_tic6x_le
#source: got-reloc-local-1.s
#source: got-reloc-local-2.s
#objdump: -dr

.*: *file format elf32-tic6x-le


Disassembly of section \.text:

00000000 <\.text>:
[ 	]*0:[ 	]*0700006e[ 	 ]*ldw \.D2T2 \*\+b14\(0\),b14
[ 	]*0:[ 	]*R_C6000_DSBT_INDEX	__c6xabi_DSBT_BASE
[ 	]*4:[ 	]*0080006c[ 	]*ldw \.D2T1 \*\+b14\(0\),a1
[ 	]*4:[ 	]*R_C6000_SBR_GOT_U15_W	a
	\.\.\.
[ 	]*20:[ 	]*0700006e[ 	]*ldw \.D2T2 \*\+b14\(0\),b14
[ 	]*20: R_C6000_DSBT_INDEX	__c6xabi_DSBT_BASE
[ 	]*24:[ 	]*0080006c[ 	]*ldw \.D2T1 \*\+b14\(0\),a1
[ 	]*24:[ 	]*R_C6000_SBR_GOT_U15_W	b
	\.\.\.
