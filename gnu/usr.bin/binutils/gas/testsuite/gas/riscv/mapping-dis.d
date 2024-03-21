#as:
#source: mapping.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text.cross.section.A:

0+000 <funcA>:
[ 	]+[0-9a-f]+:[ 	]+4505[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+bffd[ 	]+j[ 	]+0 <funcA>

Disassembly of section .text.corss.section.B:

0+000 <funcB>:
[ 	]+[0-9a-f]+:[ 	]+4509[ 	]+li[ 	]+a0,2
[ 	]+[0-9a-f]+:[ 	]+fffff06f[ 	]+j[ 	]+0 <funcB>

Disassembly of section .text.data:

0+000 <.text.data>:
[ 	]+[0-9a-f]+:[ 	]+00000000[ 	]+.word[ 	]+0x00000000
[ 	]+[0-9a-f]+:[ 	]+00000001[ 	]+.word[ 	]+0x00000001
[ 	]+[0-9a-f]+:[ 	]+4505[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+4509[ 	]+li[ 	]+a0,2
[ 	]+[0-9a-f]+:[ 	]+05000302[ 	]+.word[ 	]+0x05000302

Disassembly of section .text.odd.align.start.insn:

0+000 <.text.odd.align.start.insn>:
[ 	]+[0-9a-f]+:[ 	]+4505[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+01[ 	]+.byte[ 	]+0x01
[ 	]+[0-9a-f]+:[ 	]+00[ 	]+.byte[ 	]+0x00
[ 	]+[0-9a-f]+:[ 	]+00000013[ 	]+nop
[ 	]+[0-9a-f]+:[ 	]+00200513[ 	]+li[ 	]+a0,2
[ 	]+[0-9a-f]+:[ 	]+00000013[ 	]+nop

Disassembly of section .text.odd.align.start.data:

0+000 <.text.odd.align.start.data>:
[ 	]+[0-9a-f]+:[ 	]+01[ 	]+.byte[ 	]+0x01
[ 	]+[0-9a-f]+:[ 	]+00[ 	]+.byte[ 	]+0x00
[ 	]+[0-9a-f]+:[ 	]+0001[ 	]+nop
[ 	]+[0-9a-f]+:[ 	]+4505[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+0001[ 	]+nop

Disassembly of section .text.zero.fill.first:

0+000 <.text.zero.fill.first>:
[ 	]+[0-9a-f]+:[ 	]+4505[ 	]+li[ 	]+a0,1

Disassembly of section .text.zero.fill.last:

0+000 <.text.zero.fill.last>:
[ 	]+[0-9a-f]+:[ 	]+4505[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+4509[ 	]+li[ 	]+a0,2

Disassembly of section .text.zero.fill.align.A:

0+000 <.text.zero.fill.align.A>:
[ 	]+[0-9a-f]+:[ 	]+4505[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+4509[ 	]+li[ 	]+a0,2

Disassembly of section .text.zero.fill.align.B:

0+000 <.text.zero.fill.align.B>:
[ 	]+[0-9a-f]+:[ 	]+00100513[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+00200513[ 	]+li[ 	]+a0,2

Disassembly of section .text.last.section:

0+000 <.text.last.section>:
[ 	]+[0-9a-f]+:[ 	]+00100513[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+00000001[ 	]+.word[ 	]+0x00000001

Disassembly of section .text.section.padding:

0+000 <.text.section.padding>:
[ 	]+[0-9a-f]+:[ 	]+4505[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+0001[ 	]+nop
[ 	]+[0-9a-f]+:[ 	]+4509[ 	]+li[ 	]+a0,2
[ 	]+[0-9a-f]+:[ 	]+00000001[ 	]+.word[ 	]+0x00000001
[ 	]+[0-9a-f]+:[ 	]+0001[ 	]+nop

Disassembly of section .text.relax.align:

0+000 <.text.relax.align>:
[ 	]+[0-9a-f]+:[ 	]+0001[ 	]+nop
[ 	]+[0-9a-f]+:[ 	]+4505[ 	]+li[ 	]+a0,1
[ 	]+[0-9a-f]+:[ 	]+00000013[ 	]+nop
[ 	]+[0-9a-f]+:[ 	]+00200513[ 	]+li[ 	]+a0,2
[ 	]+[0-9a-f]+:[ 	]+00000013[ 	]+nop
