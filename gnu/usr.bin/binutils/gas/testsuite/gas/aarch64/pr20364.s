 .macro vector_base label
 .section .vectors, "ax"
 .align 11, 0
 \label:
 .endm

 .macro vector_entry label
 .section .vectors, "ax"
 .align 7, 0
 \label:
 .endm

 .macro check_vector_size since
   .if (. - \since) > (32 * 4)
     .error "Vector exceeds 32 instructions"
   .endif
 .endm

 .globl bl1_exceptions

vector_base bl1_exceptions

vector_entry SynchronousExceptionSP0
 mov x0, #0x0
 bl plat_report_exception
 b SynchronousExceptionSP0
 check_vector_size SynchronousExceptionSP0

