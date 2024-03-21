#name: Error when 430X ISA, large memory model and any data region object attributes conflict with options
#source: attr-430x-large-any.s
#as: -mcpu=msp430 -ml
#error_output: attr-430x-large-any-bad.l
