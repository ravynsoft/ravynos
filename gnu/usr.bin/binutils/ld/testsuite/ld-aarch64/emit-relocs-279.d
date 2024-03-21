#source: emit-relocs-279.s
#ld: -T relocs.ld --defsym target=0xc000 --defsym target2=0x12340 -e0 --emit-relocs
#objdump: -dr
#...
 +10000:	8a000000 	and	.*
 +10004:	8a000000 	and	.*
 +10008:	8a000000 	and	.*
 +1000c:	8a000000 	and	.*
 +10010:	363dff84 	tbz	w4, #7, c000 <target>
	+10010: R_AARCH64_TSTBR14	target
 +10014:	b745ffe7 	tbnz	x7, #40, c010 <target\+0x10>
	+10014: R_AARCH64_TSTBR14	target\+0x10
 +10018:	3619194c 	tbz	w12, #3, 12340 <target2>
	+10018: R_AARCH64_TSTBR14	target2
 +1001c:	b7c118d1 	tbnz	x17, #56, 12334 <target.*
	+1001c: R_AARCH64_TSTBR14	target.*

