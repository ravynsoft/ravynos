       .allow_index_reg

.macro test_insn mnemonic
       \mnemonic	%xmm12, %xmm4, %xmm2
       {evex} \mnemonic %xmm12, %xmm4, %xmm2
       {vex}  \mnemonic %xmm12, %xmm4, %xmm2
       {vex}  \mnemonic (%rcx), %xmm4, %xmm2
       \mnemonic	%xmm22, %xmm4, %xmm2
       \mnemonic	%ymm12, %ymm4, %ymm2
       {evex} \mnemonic %ymm12, %ymm4, %ymm2
       {vex}  \mnemonic %ymm12, %ymm4, %ymm2
       {vex}  \mnemonic (%rcx), %ymm4, %ymm2
       \mnemonic	%ymm22, %ymm4, %ymm2
.endm

       .text
_start:
       test_insn vpmadd52huq
       test_insn vpmadd52luq

       .arch .avx_ifma
        vpmadd52huq       %xmm12, %xmm4, %xmm2
        vpmadd52huq       %ymm12, %ymm4, %ymm2
