#objdump: -h
#name: section exclude
#skip: sh-*

# Test that 'e' works in .section directives.

.*: .*

Sections:
Idx Name          *Size      *VMA       *LMA       *File off  *Algn
  0 \.text         0*0000000  0*0000000  0*0000000  0*0000000  2\*\*[24]
.*
  1 \.data         0*0000000  0*0000000  0*0000000  0*0000000  2\*\*[24]
.*
  2 \.bss          0*0000000  0*0000000  0*0000000  0*0000000  2\*\*[24]
.*
  3 \.foo1         0*0000004  0*0000000  0*0000000  .*  2\*\*[24]
.*CONTENTS, EXCLUDE.*
