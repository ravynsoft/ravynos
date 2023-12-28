.syntax unified
.thumb
.text
.align 2
.global __thumbFn
.type __testFn, %function
.thumb_func
__thumbFn:
   bx lr
   nop
   
.arm
.global __armFn
.type   __armFn, %function
__armFn:
  bx lr
  
.thumb
.global __test_thumb
.type __test_thumb, %function
.thumb_func
__test_thumb:
    ADR  R0,__thumbFn
    BLX  R0
    ADR  R2,__armFn
    BLX  R2

.arm
.global __test_arm
.type   __test_arm, %function
__test_arm:
    ADRL R4,__thumbFn
    BLX  R4
    ADRL R6,__armFn
    BLX  R6
    ADR  r8, __thumbFn
    blx  r8
    ADR  r10, __armFn
    blx  r10
    adrlo r12, __thumbFn
