.cfi_sections .c6xabi.exidx

# unexpected .handlerdata directive
.handlerdata

.cfi_startproc
.personalityindex 0
# duplicate .personalityindex directive
.personalityindex 1
# personality routine specified for cantunwind frame
.cantunwind
nop
.cfi_endproc
.endp

.cfi_startproc
.personality foo
# personality routine specified for cantunwind frame
.cantunwind
nop
.cfi_endproc
.endp

.cfi_startproc
nop
.cfi_endproc
.personality foo
# duplicate .personality directive
.personality bar
.handlerdata
# unexpected .cantunwind directive
.cantunwind
# duplicate .handlerdata directive
.handlerdata
.endp

.cfi_startproc
nop
.cfi_endproc
# personality routine required before .handlerdata directive
.handlerdata
.endp

.cfi_startproc
nop
.cfi_endproc
# bad personality routine number
.personalityindex 16
# bad personality routine number
.personalityindex -1
.endp

.cfi_startproc
nop
.cfi_endproc
.personalityindex 1
.handlerdata
# missing .endp before .cfi_startproc
.cfi_startproc
.cfi_endproc
.endp

