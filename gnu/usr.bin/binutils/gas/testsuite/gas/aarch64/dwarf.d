#readelf: -s --debug-dump=aranges
#as: -g --generate-missing-build-notes=no
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

Symbol table '.symtab' contains 11 entries:
   Num:[ ]+Value[ ]+Size[ ]+Type[ ]+Bind[ ]+Vis[ ]+Ndx[ ]+Name
     0: 0+     0 NOTYPE  LOCAL  DEFAULT  UND[ ]+
     1: 0+     0 SECTION LOCAL  DEFAULT    1.*
     2: 0+     0 SECTION LOCAL  DEFAULT    2.*
     3: 0+     0 SECTION LOCAL  DEFAULT    3.*
     4: 0+     0 NOTYPE  LOCAL  DEFAULT    1 \$x
     5: 0+     0 SECTION LOCAL  DEFAULT    6.*
     6: 0+     0 SECTION LOCAL  DEFAULT    8.*
     7: 0+     0 SECTION LOCAL  DEFAULT    4.*
     8: 0+     0 SECTION LOCAL  DEFAULT   11.*
     9: 0+     0 SECTION LOCAL  DEFAULT    9.*
    10: 0+     8 FUNC    GLOBAL DEFAULT    1 testfunc
Contents of the .debug_aranges section:

  Length:                   (44|28)
  Version:                  2
  Offset into .debug_info:  (0x)?0
  Pointer Size:             (8|4)
  Segment Size:             0

    Address[ ]+Length
    0+ 0+8 ?
    0+ 0+ ?
