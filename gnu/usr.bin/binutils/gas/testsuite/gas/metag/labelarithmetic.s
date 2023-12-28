lbl1:       ADD TXL1START, CPC0, #(loop_start-lbl1)
lbl2:       MOV A0.0, CPC0
            ADD A0.0, A0.0, #(loop_end-lbl2)
            MOV TXL1END, A0.0

loop_start: MOV D0Ar2, D0Ar4
            NOP
loop_end:   MOV D1Ar1, D1Ar5
