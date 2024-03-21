       .allow_index_reg

.macro test_insn mnemonic
       \mnemonic	%xmm2, %xmm4, %xmm2
       {evex} \mnemonic %xmm2, %xmm4, %xmm2
       {vex}  \mnemonic %xmm2, %xmm4, %xmm2
       {vex}  \mnemonic (%ecx), %xmm4, %xmm2
       \mnemonic	%ymm2, %ymm4, %ymm2
       {evex} \mnemonic %ymm2, %ymm4, %ymm2
       {vex}  \mnemonic %ymm2, %ymm4, %ymm2
       {vex}  \mnemonic (%ecx), %ymm4, %ymm2
.endm

       .text
_start:
       test_insn vpmadd52huq
       test_insn vpmadd52luq

       .arch .noavx512vl

       vpmadd52huq	  %zmm0, %zmm0, %zmm0
       vpmadd52huq	  %ymm0, %ymm0, %ymm0
       vpmadd52huq	  %xmm0, %xmm0, %xmm0

       .arch default
       .arch .noavx512ifma
       
       vpmadd52huq	  %ymm0, %ymm0, %ymm0
       vpmadd52huq	  %xmm0, %xmm0, %xmm0

       .arch default
       .arch .noavx512f

       vpmadd52huq	  %ymm0, %ymm0, %ymm0
       vpmadd52huq	  %xmm0, %xmm0, %xmm0

       .arch default
       .arch .avx_ifma
        vpmadd52huq       %xmm2, %xmm4, %xmm2
        vpmadd52huq       %ymm2, %ymm4, %ymm2
