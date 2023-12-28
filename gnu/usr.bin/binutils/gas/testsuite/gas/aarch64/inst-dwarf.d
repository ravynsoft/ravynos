#as: -gdwarf-2
#name: Dwarf line number info for .inst
#objdump: -WL -w
#source: inst-directive.s

.*:[ 	]+file format .*

Contents of the .debug_line section:

CU: .*/inst-directive.s:
File name +Line number +Starting address.*
inst-directive.s +5 +0.*
inst-directive.s +6 +0x4.*
inst-directive.s +- +0x10
#pass
