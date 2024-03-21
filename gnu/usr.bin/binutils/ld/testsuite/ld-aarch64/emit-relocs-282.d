#source: emit-relocs-282.s
#ld: -T relocs.ld --defsym target=0xc000 -e0 --emit-relocs
#objdump: -dr
#...
 +10000:	8a000000 	and	.*
 +10004:	8a000000 	and	.*
 +10008:	8a000000 	and	.*
 +1000c:	8a000000 	and	.*
 +10010:	17ffeffc 	b	c000 <target>
	+10010: R_AARCH64_JUMP26	target
 +10014:	17ffefff 	b	c010 <target\+0x10>
	+10014: R_AARCH64_JUMP26	target\+0x10
