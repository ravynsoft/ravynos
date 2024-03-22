#!/bin/bash

set -e

gen_shader() {
	imm=$1
	if [ "$2" = "half" ]; then
		h="h"
		mov="cov.u16u32"
	else
		h=""
		mov="mov.u32u32"
	fi
	cat <<EOF
@localsize 1, 1, 1
@buf 4  ; g[0]
mov.u32u32 r0.x, 0
(rpt5)nop
(rpt5)nop
add.f ${h}r2.x, ${h}r0.x, $imm
(rpt5)nop
$mov r1.x, ${h}r2.x
(rpt5)nop
; and store results:
stib.b.untyped.1d.u32.1.imm r1.x, r0.x, 0
(sy)nop
end
EOF
}


run() {
	echo "TEST: $*"
	gen_shader $* | ./computerator -g 1,1,1 -d
}

for n in `seq 0 16`; do
	run "$n" "full"
	run "h($n)" "half"
done
