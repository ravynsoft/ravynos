#!/bin/bash
#
# Test various instructions to check whether half<->full widening/narrowing
# works.  The basic premise is to perform the same instruction with and
# without the widening/narrowing folded in and check if the results match.
#
# Note this doesn't currently diferentiate between signed/unsigned/bool,
# and just assumes int is signed (since unsigned is basically(ish) like
# signed but without sign extension)
#
# TODO probably good pick numeric src values that are better at triggering
# edge cases, while still not loosing precision in a full->half->full
# seqeuence.. but some instructions like absneg don't even appear to be
# subtlely wrong when you try to fold in a precision conversion.
#
# add '-v' arg to see the result values

set -e

#
# Templates for float->float instructions:
#
f2f_instrs=(
	'add.f $dst, $src1, $src2'
	'min.f $dst, $src1, $src2'
	'min.f $dst, $src2, $src1'
	'max.f $dst, $src1, $src2'
	'max.f $dst, $src2, $src1'
	'mul.f $dst, $src1, $src2'
	'sign.f $dst, $src1'
	'absneg.f $dst, \(neg\)$src1'
	'absneg.f $dst, \(abs\)$src1'
	'floor.f $dst, $src1'
	'ceil.f $dst, $src1'
	'rndne.f $dst, $src1'
	'rndaz.f $dst, $src1'
	'trunc.f $dst, $src1'
)

#
# Templates for float->int instructions:
#
f2i_instrs=(
	'cmps.f.gt $dst, $src1, $src2'
	'cmps.f.lt $dst, $src1, $src2'
	'cmpv.f.gt $dst, $src1, $src2'
	'cmpv.f.lt $dst, $src1, $src2'
)

#
# Templates for int->int instructions:
#
i2i_instrs=(
	'add.u $dst, $src1, $src2'
	'add.s $dst, $src1, $src2'
	'sub.u $dst, $src1, $src2'
	'sub.s $dst, $src1, $src2'
	'cmps.f.gt $dst, $src1, $src2'
	'cmps.f.lt $dst, $src1, $src2'
	'min.u $dst, $src1, $src2'
	'min.u $dst, $src2, $src1'
	'min.s $dst, $src1, $src2'
	'min.s $dst, $src2, $src1'
	'max.u $dst, $src1, $src2'
	'max.u $dst, $src2, $src1'
	'max.s $dst, $src1, $src2'
	'max.s $dst, $src2, $src1'
	'absneg.s $dst, \(neg\)$src1'
	'absneg.s $dst, \(abs\)$src1'
	'and.b $dst, $src2, $src3'
	'or.b $dst, $src1, $src2'
	'not.b $dst, $src1'
	'xor.b $dst, $src1, $src2'
	'cmpv.u.gt $dst, $src1, $src2'
	'cmpv.u.lt $dst, $src1, $src2'
	'cmpv.s.gt $dst, $src1, $src2'
	'cmpv.s.lt $dst, $src1, $src2'
	'mul.u24 $dst, $src1, $src2'
	'mul.s24 $dst, $src1, $src2'
	'mull.u $dst, $src1, $src2'
	'bfrev.b $dst, $src1'
	'clz.s $dst, $src2'
	'clz.b $dst, $src2'
	'shl.b $dst, $src1, $src2'
	'shr.b $dst, $src3, $src1'
	'ashr.b $dst, $src3, $src1'
	'mgen.b $dst, $src1, $src2'
	'getbit.b $dst, $src3, $src2'
	'setrm $dst, $src1'
	'cbits.b $dst, $src3'
	'shb $dst, $src1, $src2'
	'msad $dst, $src1, $src2'
)

#
# Helper to expand instruction template:
#
expand() {
	instr=$1
	dst=$2
	src1=$3
	src2=$4
	src3=$5
	eval echo $instr
}

expand_test() {
	instr=$1

	echo '; control, half->half:'
	expand $instr "hr1.x" "hr0.x" "hr0.y" "hr0.z"
	echo '; test, full->half:'
	expand $instr "hr1.y" "r1.x" "r1.y" "r1.z"

	echo '; control, full->full:'
	expand $instr "r2.x" "r1.x" "r1.y" "r1.z"
	echo '; test, half->full:'
	expand $instr "r2.y" "hr0.x" "hr0.y" "hr0.z"

	echo "(rpt5)nop"
}

#
# Helpers to construct test program assembly:
#
header_asm() {
	cat <<EOF
@localsize 1, 1, 1
@buf 4  ; g[0]
EOF
}

footer_asm() {
	cat <<EOF
; dest offsets:
mov.u32u32 r3.x, 0
mov.u32u32 r3.y, 1
mov.u32u32 r3.z, 2
mov.u32u32 r3.w, 3
(rpt5)nop

; and store results:
stib.untyped.1d.u32.1 r2.x, r3.x, 0   ; control: full->full
stib.untyped.1d.u32.1 r2.y, r3.y, 0   ; test:    half->full
stib.untyped.1d.u32.1 r2.z, r3.z, 0   ; control: half->half
stib.untyped.1d.u32.1 r2.w, r3.w, 0   ; test:    full->half
(sy)nop
end
EOF
}

setup_asm_float() {
	cat <<EOF
; hr0->hr1 (r0) avail for half, hr0 for src, hr1 for dst
; r1->r2 avail for full, r1 for src, r2 for dst
cov.f32f16 hr0.x, (1.0)
cov.f32f16 hr0.y, (2.0)
cov.f32f16 hr0.z, (3.0)
mov.f32f32 r1.x,  (1.0)
mov.f32f32 r1.y,  (2.0)
mov.f32f32 r1.z,  (3.0)
(rpt5)nop
EOF
}

setup_asm_int() {
	cat <<EOF
; hr0->hr1 (r0) avail for half, hr0 for src, hr1 for dst
; r1->r2 avail for full, r1 for src, r2 for dst
cov.s32s16 hr0.x,  1
cov.s32s16 hr0.y, -2
cov.s32s16 hr0.z,  3
mov.s32s32 r1.x,   1
mov.s32s32 r1.y,  -2
mov.s32s32 r1.z,   3
(rpt5)nop
EOF
}

#
# Generate assembly code to test float->float opcode
#
f2f_asm() {
	instr=$1

	header_asm
	setup_asm_float
	expand_test $instr

	cat <<EOF
; convert half results back to full:
cov.f16f32 r2.z, hr1.x
cov.f16f32 r2.w, hr1.y
EOF

	footer_asm
}

#
# Generate assembly code to test float->int opcode
#
f2i_asm() {
	instr=$1

	header_asm
	setup_asm_float
	expand_test $instr

	cat <<EOF
; convert half results back to full:
cov.s16s32 r2.z, hr1.x
cov.s16s32 r2.w, hr1.y
EOF

	footer_asm
}

#
# Generate assembly code to test int->int opcode
#
i2i_asm() {
	instr=$1

	header_asm
	setup_asm_int
	expand_test $instr

	cat <<EOF
; convert half results back to full:
cov.s16s32 r2.z, hr1.x
cov.s16s32 r2.w, hr1.y
EOF

	footer_asm
}


#
# Helper to parse computerator output and print results:
#
check_results() {
	str=`cat - | grep "	" | head -1 | xargs`

	if [ "$verbose" = "true" ]; then
		echo $str
	fi

	# Split components of result buffer:
	cf=$(echo $str | cut -f1 -d' ')
	tf=$(echo $str | cut -f2 -d' ')
	ch=$(echo $str | cut -f3 -d' ')
	th=$(echo $str | cut -f4 -d' ')

	# Sanity test, make sure the control results match:
	if [ $cf != $ch ]; then
		echo "    FAIL: control results do not match!  Half vs full op is not equivalent!"
		echo "    full=$cf half=$ch"
	fi

	# Compare test (with conversion folded) to control:
	if [ $cf != $tf ]; then
		echo "    FAIL: half -> full widening result does not match control!"
		echo "    control=$cf result=$tf"
	fi
	if [ $ch != $th ]; then
		echo "    FAIL: full -> half narrowing result does not match control!"
		echo "    control=$ch result=$th"
	fi

	# HACK without a delay different invocations
	# of computerator seem to somehow clobber each
	# other.. which isn't great..
	sleep 0.1
}

#
# Run the tests!
#

if [ "$1" = "-v" ]; then
	verbose="true"
fi

IFS=""
for instr in ${f2f_instrs[@]}; do
	echo "TEST: $instr"
	f2f_asm $instr | ./computerator -g 1,1,1 | check_results
done
for instr in ${f2i_instrs[@]}; do
	echo "TEST: $instr"
	f2i_asm $instr | ./computerator -g 1,1,1 | check_results
done
for instr in ${i2i_instrs[@]}; do
	echo "TEST: $instr"
	i2i_asm $instr | ./computerator -g 1,1,1 | check_results
done

