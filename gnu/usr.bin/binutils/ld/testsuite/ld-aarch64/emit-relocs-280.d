#source: emit-relocs-280.s
#ld: -T relocs.ld --defsym target=0xc000 -e0 --emit-relocs
#objdump: -dr
#...
 +10000:	8a000000 	and	.*
 +10004:	8a000000 	and	.*
 +10008:	8a000000 	and	.*
 +1000c:	8a000000 	and	.*
 +10010:	54fdff80 	b.eq	c000 <target>  // .*
	+10010: R_AARCH64_CONDBR19	target
 +10014:	54fdffe0 	b.eq	c010 <target\+0x10>  // .*
	+10014: R_AARCH64_CONDBR19	target\+0x10
