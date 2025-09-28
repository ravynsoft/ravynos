#as: -march=rv64ifc -gdwarf-2
#name: Dwarf line number info for .insn
#objdump: -WL -w
#source: insn.s

.*:[ 	]+file format .*

Contents of the .debug_line section:

CU: .*/insn.s:
File name +Line number +Starting address.*
insn.s +2 +0.*
insn.s +3 +0x4.*
insn.s +4 +0x8.*
insn.s +5 +0xc.*
insn.s +6 +0x10.*
insn.s +7 +0x14.*
insn.s +8 +0x18.*
insn.s +9 +0x1c.*
insn.s +10 +0x20.*
insn.s +11 +0x24.*
insn.s +13 +0x28.*
insn.s +14 +0x2a.*
insn.s +15 +0x2c.*
insn.s +16 +0x2e.*
insn.s +17 +0x30.*
insn.s +18 +0x32.*
insn.s +19 +0x34.*
insn.s +20 +0x36.*
insn.s +22 +0x38.*
insn.s +23 +0x3c.*
insn.s +24 +0x40.*
insn.s +25 +0x44.*
insn.s +26 +0x48.*
insn.s +27 +0x4c.*
insn.s +28 +0x50.*
insn.s +29 +0x54.*
insn.s +30 +0x58.*
insn.s +31 +0x5c.*
insn.s +33 +0x60.*
insn.s +34 +0x62.*
insn.s +35 +0x64.*
insn.s +36 +0x66.*
insn.s +37 +0x68.*
insn.s +38 +0x6a.*
insn.s +39 +0x6c.*
insn.s +40 +0x6e.*
insn.s +41 +0x70.*
insn.s +43 +0x72.*
insn.s +44 +0x76.*
insn.s +45 +0x7a.*
insn.s +46 +0x7e.*
insn.s +47 +0x82.*
insn.s +48 +0x86.*
insn.s +49 +0x8a.*
insn.s +50 +0x8e.*
insn.s +51 +0x92.*
insn.s +52 +0x96.*
insn.s +53 +0x9a.*
insn.s +54 +0x9e.*
insn.s +55 +0xa2.*
insn.s +57 +0xa6.*
insn.s +59 +0xaa.*
insn.s +60 +0xac.*
insn.s +61 +0xb0.*
insn.s +62 +0xb6.*
insn.s +63 +0xbe.*
insn.s +64 +0xc8.*
insn.s +65 +0xd4.*
insn.s +66 +0xea.*
insn.s +67 +0xec.*
insn.s +68 +0xf0.*
insn.s +69 +0xf6.*
insn.s +70 +0xfe.*
insn.s +71 +0x108.*
insn.s +72 +0x114.*
insn.s +74 +0x12a.*
insn.s +75 +0x134.*
insn.s +76 +0x13e.*
insn.s +77 +0x154.*
insn.s +78 +0x16a.*
insn.s +79 +0x180.*
insn.s +80 +0x196.*
insn.s +81 +0x1ac.*
insn.s +- +0x1c2
#pass
