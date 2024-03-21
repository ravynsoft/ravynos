#objdump: -s -j .text -j "\$TEXT\$"
#name: Generate NOPs in an architecture neutral manner

.*: +file format .*

Contents of section (\.text|\$TEXT\$):
 [^ ]* .*
#pass
