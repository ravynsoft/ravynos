# 2 operand insn test

        dsp_fp_sqrt r0,r1
        dsp_fp_sqrt fp,sp

        dsp_fp_sqrt r0,0
        dsp_fp_sqrt r1,-1
        dsp_fp_sqrt 0,r2
        dsp_fp_sqrt r4,255
        dsp_fp_sqrt r6,-256

        dsp_fp_sqrt r8,256
        dsp_fp_sqrt r9,-257
        dsp_fp_sqrt r11,0x42424242

        dsp_fp_sqrt r0,foo

        dsp_fp_sqrt.f r0,r1
        dsp_fp_sqrt.f r2,1
        dsp_fp_sqrt.f 0,r4
        dsp_fp_sqrt.f r5,512
