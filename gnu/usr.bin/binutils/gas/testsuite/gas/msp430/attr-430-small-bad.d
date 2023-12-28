#name: Error when 430 ISA and small memory model object attributes conflict with options
#source: attr-430-small.s
#as: -mdata-region=none -ml
#error_output: attr-430-small-bad.l
