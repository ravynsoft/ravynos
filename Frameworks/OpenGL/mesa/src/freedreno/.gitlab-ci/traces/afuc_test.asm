; Copyright (c) 2020 Valve Corporation
;
; Permission is hereby granted, free of charge, to any person obtaining a
; copy of this software and associated documentation files (the "Software"),
; to deal in the Software without restriction, including without limitation
; the rights to use, copy, modify, merge, publish, distribute, sublicense,
; and/or sell copies of the Software, and to permit persons to whom the
; Software is furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice (including the next
; paragraph) shall be included in all copies or substantial portions of the
; Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
; THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.
;
;
; This file is the source for a simple mock firmware used to regression test
; the afuc assembler/disassembler. Note, it won't actually work if you try to
; load it on the GPU! First this is assembled, compared to the reference
; binary, then disassambled and compared to the reference disassembly. We do
; this to avoid having to host the actual firmware, especially the disassembled
; version, in Mesa.
[01000001]
[01000000]
loc02:
; packet table loading:
mov $01, 0x0830 ; CP_SQE_INSTR_BASE
mov $02, 0x0002
cwrite $01, [$00 + @REG_READ_ADDR]
cwrite $02, [$00 + @REG_READ_DWORDS]
; move hi/lo of SQE fw addrs to registers:
mov $01, $regdata
mov $02, $regdata
; skip first dword
add $01, $01, 0x0004
addhi $02, $02, 0x0000
mov $03, 0x0001
cwrite $01, [$00 + @MEM_READ_ADDR]
cwrite $02, [$00 + @MEM_READ_ADDR+0x1]
cwrite $03, [$00 + @MEM_READ_DWORDS]
; read 2nd dword of fw, and add offset (minus 4 because we skipped first dword)
; to base address of sqe fw
rot $04, $memdata, 0x0008
ushr $04, $04, 0x0006
sub $04, $04, 0x0004
add $01, $01, $04
addhi $02, $02, 0x0000

; load packet table:
mov $rem, 0x0080
cwrite $01, [$00 + @MEM_READ_ADDR]
cwrite $02, [$00 + @MEM_READ_ADDR+0x1]
cwrite $02, [$00 + @LOAD_STORE_HI]
cwrite $rem, [$00 + @MEM_READ_DWORDS]
cwrite $00, [$00 + @PACKET_TABLE_WRITE_ADDR]
(rep)cwrite $memdata, [$00 + @PACKET_TABLE_WRITE]

mov $02, 0x883
mov $03, 0xbeef
mov $04, 0xdead << 16
or $03, $03, $04
cwrite $02, [$00 + @REG_WRITE_ADDR]
cwrite $03, [$00 + @REG_WRITE]
waitin
mov $01, $data

CP_ME_INIT:
; test label-as-immediate feature
mov $02, #loc02 ; should be 0x0002
waitin
mov $01, $data

CP_MEM_WRITE:
; test $addr + (rep) + (xmovN) with ALU
mov $addr, 0xa0 << 24
mov $02, 4
(xmov1)add $data, $02, $data
mov $addr, 0xa204 << 16
(rep)(xmov3)mov $data, $data
waitin
mov $01, $data

CP_SCRATCH_WRITE:
; test (rep) + flags + non-zero offset with cwrite
; TODO: 0x4 flag is actually pre-increment addressing, handle it as such
mov $02, 0xff
(rep)cwrite $data, [$02 + 0x001]!
waitin
mov $01, $data

CP_SET_DRAW_STATE:
; test (sds)
(rep)(sds2) cwrite $data, [$00 + @DRAW_STATE_SET_HDR]
waitin
mov $01, $data

CP_SET_BIN_DATA5:
; test SQE registers
sread $02, [$00 + %SP]
swrite $02, [$00 + %SP]
mov $02, 7
(rep)swrite $data, [$02 + 1]!
waitin
mov $01, $data

CP_SET_SECURE_MODE:
; test setsecure
mov $02, $data
setsecure $02, #setsecure_success
err:
jump #err
nop
setsecure_success:
waitin
mov $01, $data

euclid:
; Euclid's algorithm in afuc: https://en.wikipedia.org/wiki/Euclidean_algorithm
; Since afuc doesn't do modulo, we implement the subtraction-based version.
;
; Demonstrates/tests comparisons and conditional branches. This also
; demonstrates the common trick of branching in a delay slot. Note that if a
; branch is taken and its delay slot includes another branch, the second
; branch cannot also be taken, which is why the last branch in the sequence
; cannot be unconditional.
;
; Inputs are in $02 and $03, and output is in $02.
cmp $04, $02, $03
breq $04, b0, #euclid_exit
brne $04, b1, #euclid_gt
breq $04, b2, #euclid
sub $03, $03, $02
euclid_gt:
jump #euclid
sub $02, $02, $03
euclid_exit:
ret
nop

CP_REG_RMW:
; Test various ALU instructions, and read/write $regdata
cwrite $data, [$00 + @REG_READ_ADDR]
add $02, $regdata, 0x42
addhi $03, $00, $regdata
sub $02, $02, $regdata
call #euclid
subhi $03, $03, $regdata
and $02, $02, $regdata
or $02, $02, 0x1
xor $02, $02, 0x1
not $02, $02
shl $02, $02, $regdata
ushr $02, $02, $regdata
ishr $02, $02, $regdata
rot $02, $02, $regdata
min $02, $02, $regdata
max $02, $02, $regdata
mul8 $02, $02, $regdata
msb $02, $02
mov $usraddr, $data
mov $data, $02
waitin
mov $01, $data

CP_MEMCPY:
; implement CP_MEMCPY using load/store instructions
mov $02, $data
mov $03, $data
mov $04, $data
mov $05, $data
mov $06, $data
cpy_header:
breq $06, 0, #cpy_exit
cwrite $03, [$00 + @LOAD_STORE_HI]
load $07, [$02 + 0x004]!
cwrite $05, [$00 + @LOAD_STORE_HI]
jump #cpy_header
store $07, [$04 + 0x004]!
cpy_exit:
waitin
mov $01, $data

CP_MEM_TO_MEM:
; implement CP_MEMCPY using mem read control regs
; tests @FOO+0x1 for 64-bit control regs, and reading/writing $rem
cwrite $data, [$00 + @MEM_READ_ADDR]
cwrite $data, [$00 + @MEM_READ_ADDR+1]
mov $02, $data
cwrite $data, [$00 + @LOAD_STORE_HI]
mov $rem, $data
cwrite $rem, [$00 + @MEM_READ_DWORDS]
(rep)store $memdata, [$02 + 0x004]!
waitin
mov $01, $data

UNKN15:
; test preemptleave + iret + conditional branch w/ immed
cread $02, [$00 + 0x101]
brne $02, 0x0001, #exit_iret
nop
preemptleave #err
nop
nop
nop
waitin
mov $01, $data
exit_iret:
iret
nop

UNKN0:
UNKN1:
UNKN2:
UNKN3:
PKT4:
UNKN5:
UNKN6:
UNKN7:
UNKN8:
UNKN9:
UNKN10:
UNKN11:
UNKN12:
UNKN13:
UNKN14:
CP_NOP:
CP_RECORD_PFP_TIMESTAMP:
CP_WAIT_MEM_WRITES:
CP_WAIT_FOR_ME:
CP_WAIT_MEM_GTE:
UNKN21:
UNKN22:
UNKN23:
UNKN24:
CP_DRAW_PRED_ENABLE_GLOBAL:
CP_DRAW_PRED_ENABLE_LOCAL:
UNKN27:
CP_PREEMPT_ENABLE:
CP_SKIP_IB2_ENABLE_GLOBAL:
CP_PREEMPT_TOKEN:
UNKN31:
UNKN32:
CP_DRAW_INDX:
CP_SKIP_IB2_ENABLE_LOCAL:
CP_DRAW_AUTO:
CP_SET_STATE:
CP_WAIT_FOR_IDLE:
CP_IM_LOAD:
CP_DRAW_INDIRECT:
CP_DRAW_INDX_INDIRECT:
CP_DRAW_INDIRECT_MULTI:
CP_IM_LOAD_IMMEDIATE:
CP_BLIT:
CP_SET_CONSTANT:
CP_SET_BIN_DATA5_OFFSET:
UNKN48:
CP_RUN_OPENCL:
CP_LOAD_STATE6_GEOM:
CP_EXEC_CS:
CP_LOAD_STATE6_FRAG:
CP_SET_SUBDRAW_SIZE:
CP_LOAD_STATE6:
CP_INDIRECT_BUFFER_PFD:
CP_DRAW_INDX_OFFSET:
CP_REG_TEST:
CP_COND_INDIRECT_BUFFER_PFE:
CP_INVALIDATE_STATE:
CP_WAIT_REG_MEM:
CP_REG_TO_MEM:
CP_INDIRECT_BUFFER:
CP_INTERRUPT:
CP_EXEC_CS_INDIRECT:
CP_MEM_TO_REG:
CP_COND_EXEC:
CP_COND_WRITE5:
CP_EVENT_WRITE:
CP_COND_REG_EXEC:
UNKN73:
CP_REG_TO_SCRATCH:
CP_SET_DRAW_INIT_FLAGS:
CP_SCRATCH_TO_REG:
CP_DRAW_PRED_SET:
CP_MEM_WRITE_CNTR:
CP_START_BIN:
CP_END_BIN:
CP_WAIT_REG_EQ:
CP_SMMU_TABLE_UPDATE:
UNKN84:
CP_SET_CTXSWITCH_IB:
CP_SET_PSEUDO_REG:
CP_INDIRECT_BUFFER_CHAIN:
CP_EVENT_WRITE_SHD:
CP_EVENT_WRITE_CFL:
UNKN90:
CP_EVENT_WRITE_ZPD:
CP_CONTEXT_REG_BUNCH:
CP_WAIT_IB_PFD_COMPLETE:
CP_CONTEXT_UPDATE:
CP_SET_PROTECTED_MODE:
UNKN96:
UNKN97:
UNKN98:
CP_SET_MODE:
CP_SET_VISIBILITY_OVERRIDE:
CP_SET_MARKER:
UNKN103:
UNKN104:
UNKN105:
UNKN106:
UNKN107:
UNKN108:
CP_REG_WRITE:
UNKN110:
CP_BOOTSTRAP_UCODE:
CP_WAIT_TWO_REGS:
CP_TEST_TWO_MEMS:
CP_REG_TO_MEM_OFFSET_REG:
CP_REG_TO_MEM_OFFSET_MEM:
UNKN118:
UNKN119:
CP_REG_WR_NO_CTXT:
UNKN121:
UNKN122:
UNKN123:
UNKN124:
UNKN125:
UNKN126:
UNKN127:
        waitin
        mov $01, $data
