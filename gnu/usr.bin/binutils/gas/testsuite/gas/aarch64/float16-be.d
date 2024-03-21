# name: Big endian float16 literals (IEEE 754 format)
# source: float16.s
# objdump: -s --section=.data
# as: -mbig-endian

.*: +file format .*

Contents of section \.data:
 0000 4a002fdf 1c197bff 000103ff 04003c00.*
 0010 3c017fff 7c00fc00 00008000 bc00bbe7.*
 0020 fbff4200 4a00603e.*
