 .weak a,b
 .data
 .dc.a a
 .text
 .global _start
_start:
 bl b
 nop
