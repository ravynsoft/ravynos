#name: Error when 430X ISA, large memory model and lower data region object attributes conflict with options
#source: attr-430x-large-lower.s
#as: -mdata-region=none -mcpu=msp430 -ml
#error_output: attr-430x-large-lower-bad.l
