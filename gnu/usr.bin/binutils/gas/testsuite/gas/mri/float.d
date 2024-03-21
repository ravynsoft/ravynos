#name: MRI floating point constants
#as: -M
#objdump: -s

.*:     file format .*

Contents of section \.text:
 0+00 (12345678 9abcdef0 3f800000 41200000)|(f0debc9a 78563412 0000803f 00002041) .*
 0+10 (41200000 42c80000)|(000020 410000c842) .*
