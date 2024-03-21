#source: section-match-1.s
#ld: -T section-match-1.t
#objdump: -s
# This test uses arbitrary section names, which are not supported by some
# file formats.  Also these section names must be present in the
# output, not translated into some other name, eg .text
#target: [is_elf_format]
#xfail: h8300-*-*

.*:     file format .*

#...
Contents of section \.secA:
 [0-9a-f]* (01)?0+(01)? .*
Contents of section \.secC:
 [0-9a-f]* (02)?0+(02)? .*
#pass
