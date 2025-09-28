	.syntax unified
func:
	vstr fpcxtns,[sp,#-4]!
	vstr fpcxts,[sp,#-4]!
	vstr FPCXTNS,[sp,#-4]!
	vstr FPCXTS,[sp,#-4]!
	vstr fpcxt_ns,[sp,#-4]!
	vstr fpcxt_s,[sp,#-4]!
	vstr FPCXT_NS,[sp,#-4]!
	vstr FPCXT_S,[sp,#-4]!

	vldr FPCXTNS, [r3]
	vldr FPCXT_NS, [r3]
	vldr fpcxtns, [r3]
	vldr fpcxt_ns, [r3]
	vldr FPCXTS, [r3]
	vldr FPCXT_S, [r3]
	vldr fpcxt_s, [r3]
	vldr fpcxts, [r3]

	vmrs r4, FPCXT_NS
	vmrs r4, FPCXTNS
	vmrs r5, FPCXTS
	vmrs r5, FPCXT_S
	vmrs r4, fpcxt_ns
	vmrs r4, fpcxtns
	vmrs r5, fpcxts
	vmrs r5, fpcxt_s

	vmsr FPCXT_NS, r4
	vmsr FPCXTNS, r4
	vmsr FPCXTS, r5
	vmsr FPCXT_S, r5
	vmsr fpcxt_ns, r4
	vmsr fpcxtns, r4
	vmsr fpcxts, r5
	vmsr fpcxt_s, r5
