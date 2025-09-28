#as: -EL -I$srcdir/$subdir --generate-missing-build-notes=no
#objdump: --syms --special-syms -d
#name: ARM Mapping Symbols for .arm/.thumb
# This test is only valid on EABI based ports.
#target: *-*-*eabi* *-*-linux-* *-*-elf *-*-nacl*
#source: mapdir.s


.*: +file format .*arm.*

SYMBOL TABLE:
0+00 l    d  .text	00000000 .text
0+00 l    d  .data	00000000 .data
0+00 l    d  .bss	00000000 .bss
0+00 l    d  .fini_array	00000000 .fini_array
0+00 l       .fini_array	00000000 \$d
0+00 l     O .fini_array	00000000 __do_global_dtors_aux_fini_array_entry
0+00 l    d  .code	00000000 .code
0+00 l       .code	00000000 \$a
0+00 l    d  .tcode	00000000 .tcode
0+00 l       .tcode	00000000 \$t
0+00 l    d  .ARM.attributes	00000000 .ARM.attributes
0+00         \*UND\*	00000000 __do_global_dtors_aux



Disassembly of section .code:

00000000 <.code>:
   0:	e1a00000 	nop			@ \(mov r0, r0\)

Disassembly of section .tcode:

00000000 <.tcode>:
   0:	46c0      	nop			@ \(mov r8, r8\)
