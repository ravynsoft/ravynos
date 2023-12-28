 .section ".opd","aw",@progbits
 .p2align 3
 .globl _start
_start:
 .quad .L_start,.TOC.@tocbase,0

 .text
.L_start:
 addis 3,2,PrettyStackTraceHead@got@tlsld@ha
 addi 29,3,PrettyStackTraceHead@got@tlsld@l
 mr 3,29
 bl __tls_get_addr(PrettyStackTraceHead@tlsld)
 nop
 addis 3,3,PrettyStackTraceHead@dtprel@ha
 ld 3,PrettyStackTraceHead@dtprel@l(3)
 nop

 addi 29,2,PrettyStackTraceHead@got@tlsld
 mr 3,29
 bl __tls_get_addr(PrettyStackTraceHead@tlsld)
 nop
 ld 3,PrettyStackTraceHead@dtprel(3)
 nop
 nop
 nop

 addis 3,2,PrettyStackTraceHead@got@tlsgd@ha
 addi 29,3,PrettyStackTraceHead@got@tlsgd@l
 mr 3,29
 bl __tls_get_addr(PrettyStackTraceHead@tlsgd)
 nop
 ld 3,0(3)
 nop
 nop

 addi 29,2,PrettyStackTraceHead@got@tlsgd
 mr 3,29
 bl __tls_get_addr(PrettyStackTraceHead@tlsgd)
 nop
 ld 3,0(3)
 nop
 nop
 nop

 .section ".tbss","awT",@nobits
 .align 3
PrettyStackTraceHead:
 .space 8
