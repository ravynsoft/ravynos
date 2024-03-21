 .text
 .globl _start
_start:
 addis 3,31,PrettyStackTraceHead@got@tlsld@ha
 addi 29,3,PrettyStackTraceHead@got@tlsld@l
 mr 3,29
 bl __tls_get_addr(PrettyStackTraceHead@tlsld)
 addis 3,3,PrettyStackTraceHead@dtprel@ha
 lwz 3,PrettyStackTraceHead@dtprel@l(3)
 nop
 nop

 addi 29,31,PrettyStackTraceHead@got@tlsld
 mr 3,29
 bl __tls_get_addr(PrettyStackTraceHead@tlsld)
 lwz 3,PrettyStackTraceHead@dtprel(3)
 nop
 nop
 nop
 nop

 addis 3,31,PrettyStackTraceHead@got@tlsgd@ha
 addi 29,3,PrettyStackTraceHead@got@tlsgd@l
 mr 3,29
 bl __tls_get_addr(PrettyStackTraceHead@tlsgd)
 lwz 3,0(3)
 nop
 nop
 nop

 addi 29,31,PrettyStackTraceHead@got@tlsgd
 mr 3,29
 bl __tls_get_addr(PrettyStackTraceHead@tlsgd)
 lwz 3,0(3)
 nop
 nop
 nop
 nop

 .section ".tbss","awT",@nobits
 .align 2
PrettyStackTraceHead:
 .space 4
