#as: -gdwarf-2
#name: Dwarf line number info for .inst
#objdump: -WL -w
#source: inst-po.s
#skip: *-*-pe *-*-wince

.*:[ 	]+file format .*

Contents of the .debug_line section:

CU: .*/inst-po.s:
File name +Line number +Starting address.*
inst-po.s +5 +0.*
inst-po.s +12 +0x4.*
inst-po.s +13 +0x8.*
inst-po.s +20 +0xe.*
inst-po.s +22 +0x14.*
inst-po.s +26 +0x16.*
inst-po.s +27 +0x1a.*
inst-po.s +- +0x1e
#pass
