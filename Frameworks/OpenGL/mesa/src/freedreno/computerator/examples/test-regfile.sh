#!/bin/bash

# This tests for the size of the register file. We do this by launching a
# lot of workgroups with only one invocation, which causes the GPU to be
# saturated with in-flight waves. Each thread records its wave id using "getwid"
# (only available in a6xx+!) and stores it in the buffer. We then vary the
# register footprint by introducing uses of higher and higher registers. This
# lets us determine:
# 1. The total number of waves available (always 16 for known models)
# 2. The wave granularity (how many waves are always launched together, always 2
# for known models).
# 3. The total size of the register file that is divvied up between the waves.

set -e

gen_shader() {
	n=$1;
	cat <<EOF
@localsize 1, 1, 1
@buf 128  ; g[0]
@wgid(r48.x)
getwid.u32 r1.x
mov.u32u32 r0.x, r48.x

; busy loop to make sure it actually uses all possible waves
mov.u32u32 r0.y, 16
(rpt2)nop
loop:
cmps.u.gt p0.x, r0.y, 0
sub.u r0.y, r0.y, 1
(rpt5)nop
br p0.x, #loop
add.f r1.y, r1.x, r$n.w

(ss)(sy)(rpt5)nop
stib.b.untyped.1d.u32.1.imm r1.x, r0.x, 0
end
nop
EOF
}

# generate reference:
gen_shader 1 | ./computerator -g 128,1,1 | tee reference.log

for n in `seq 2 32`; do
	echo "Trying max reg: r$n"
	gen_shader $n | ./computerator -g 128,1,1 | tee new.log
	diff reference.log new.log
	if [ "$?" != "0" ]; then
		echo "Changes at r$n"
		break
	fi
done
