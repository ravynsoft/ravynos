foo:
	lw $r0, [$r1 + ($r2 << 1)]
	lh $r0, [$r1 + ($r2 << 1)]
	lhs $r0, [$r1 + ($r2 << 1)]
	lb $r0, [$r1 + ($r2 << 1)]
	lbs $r0, [$r1 + ($r2 << 1)]
	sw $r0, [$r1 + ($r2 << 1)]
	sh $r0, [$r1 + ($r2 << 1)]
	sb $r0, [$r1 + ($r2 << 1)]
	lw.bi $r0, [$r1], $r2 << 1
	lh.bi $r0, [$r1], $r2 << 1
	lhs.bi $r0, [$r1], $r2 << 1
	lb.bi $r0, [$r1], $r2 << 1
	lbs.bi $r0, [$r1], $r2 << 1
	sw.bi $r0, [$r1], $r2 << 1
	sh.bi $r0, [$r1], $r2 << 1
	sb.bi $r0, [$r1], $r2 << 1
