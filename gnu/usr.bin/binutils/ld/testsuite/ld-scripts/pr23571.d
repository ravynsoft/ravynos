#source: align2a.s
#ld: -T pr23571.t -z common-page-size=0x1000 -z max-page-size=0x1000
#objdump: -h -w

.*: +file format .*

Sections:
Idx Name +Size +VMA +LMA +File off +Algn +Flags
 +0 \.text +[0-9a-f]* +0+1000 +0+1000 .*
 +1 \.data +[0-9a-f]* +0+2000 +0+2000 +[0-9a-f]* +2\*\*12 .*
