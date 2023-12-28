# name: Little endian bfloat16 literal directives
# source: bfloat16-directive.s
# objdump: -s --section=.data
# as: -mlittle-endian

.*: +file format .*

Contents of section \.data:
 0000 4041fc3d 0000f742 0080f7c2 ff7f807f  .*
 0010 80ff7f7f 7fff8000 80800100 01807f00  .*
 0020 7f80803f 80bf0040 00c00000           .*
