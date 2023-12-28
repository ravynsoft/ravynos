# name: Little endian float16 literals (IEEE 754 & Alternative)
# source: float16.s
# objdump: -s --section=.data
# as: -mlittle-endian

.*: +file format .*arm.*

Contents of section \.data:
 0000 004adf2f 191cff7b 0100ff03 0004003c.*
 0010 013cff7f 007c00fc 00000080 00bce7bb.*
 0020 fffb0042 004a3e60 5638ff7f ffff0472.*
