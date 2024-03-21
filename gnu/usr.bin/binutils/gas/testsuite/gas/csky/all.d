# name: csky - all
#as: -mcpu=ck610e -W
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*0000\s*bkpt
\s*[0-9a-f]*:\s*0001\s*sync
\s*[0-9a-f]*:\s*0002\s*rte
\s*[0-9a-f]*:\s*0002\s*rte
\s*[0-9a-f]*:\s*0003\s*rfi
\s*[0-9a-f]*:\s*0004\s*stop
\s*[0-9a-f]*:\s*0005\s*wait
\s*[0-9a-f]*:\s*0006\s*doze
\s*[0-9a-f]*:\s*0007\s*idly4
\s*[0-9a-f]*:\s*000b\s*trap\s*3
\s*[0-9a-f]*:\s*0021\s*mvc\s*r1
\s*[0-9a-f]*:\s*0032\s*mvcv\s*r2
\s*[0-9a-f]*:\s*0042\s*ldq\s*r4-r7, \(r2\)
\s*[0-9a-f]*:\s*0052\s*stq\s*r4-r7, \(r2\)
\s*[0-9a-f]*:\s*0061\s*ldm\s*r1-r15, \(r0\)
\s*[0-9a-f]*:\s*0082\s*dect\s*r2, r2, 1
\s*[0-9a-f]*:\s*0092\s*decf\s*r2, r2, 1
\s*[0-9a-f]*:\s*00a2\s*inct\s*r2, r2, 1
\s*[0-9a-f]*:\s*00b2\s*incf\s*r2, r2, 1
\s*[0-9a-f]*:\s*00c1\s*jmp\s*r1
\s*[0-9a-f]*:\s*00d1\s*jsr\s*r1
\s*[0-9a-f]*:\s*00eb\s*ff1\s*r11, r11
\s*[0-9a-f]*:\s*00f1\s*brev\s*r1, r1
\s*[0-9a-f]*:\s*0102\s*xtrb3\s*r1, r2
\s*[0-9a-f]*:\s*0112\s*xtrb2\s*r1, r2
\s*[0-9a-f]*:\s*0122\s*xtrb1\s*r1, r2
\s*[0-9a-f]*:\s*0132\s*xtrb0\s*r1, r2
\s*[0-9a-f]*:\s*0132\s*xtrb0\s*r1, r2
\s*[0-9a-f]*:\s*0132\s*xtrb0\s*r1, r2
\s*[0-9a-f]*:\s*1213\s*mov\s*r3, r1
\s*[0-9a-f]*:\s*0142\s*zextb\s*r2, r2
\s*[0-9a-f]*:\s*0152\s*sextb\s*r2, r2
\s*[0-9a-f]*:\s*0162\s*zexth\s*r2, r2
\s*[0-9a-f]*:\s*0172\s*sexth\s*r2, r2
\s*[0-9a-f]*:\s*0182\s*declt\s*r2, r2, 1
\s*[0-9a-f]*:\s*01b1\s*decne\s*r1, r1, 1
\s*[0-9a-f]*:\s*01a1\s*decgt\s*r1, r1, 1
\s*[0-9a-f]*:\s*01c1\s*clrt\s*r1
\s*[0-9a-f]*:\s*01d1\s*clrf\s*r1
\s*[0-9a-f]*:\s*01e3\s*abs\s*r3, r3
\s*[0-9a-f]*:\s*01fc\s*not\s*r12, r12
\s*[0-9a-f]*:\s*0221\s*movt\s*r1, r2
\s*[0-9a-f]*:\s*0343\s*mult\s*r3, r3, r4
\s*[0-9a-f]*:\s*0587\s*subu\s*r7, r7, r8
\s*[0-9a-f]*:\s*0587\s*subu\s*r7, r7, r8
\s*[0-9a-f]*:\s*06a9\s*addc\s*r9, r9, r10
\s*[0-9a-f]*:\s*07cb\s*subc\s*r11, r11, r12
\s*[0-9a-f]*:\s*0adc\s*movf\s*r12, r13
\s*[0-9a-f]*:\s*0bdc\s*lsr\s*r12, r12, r13
\s*[0-9a-f]*:\s*0ced\s*cmphs\s*r13, r14
\s*[0-9a-f]*:\s*0ded\s*cmplt\s*r13, r14
\s*[0-9a-f]*:\s*0eed\s*tst\s*r13, r14
\s*[0-9a-f]*:\s*0fed\s*cmpne\s*r13, r14
\s*[0-9a-f]*:\s*11f7\s*psrclr\s*ie, fe, ee
\s*[0-9a-f]*:\s*1253\s*mov\s*r3, r5
\s*[0-9a-f]*:\s*1332\s*bgenr\s*r2, r3
\s*[0-9a-f]*:\s*1643\s*and\s*r3, r3, r4
\s*[0-9a-f]*:\s*1543\s*ixw\s*r3, r3, r4
\s*[0-9a-f]*:\s*1a43\s*asr\s*r3, r3, r4
\s*[0-9a-f]*:\s*1c43\s*addu\s*r3, r3, r4
\s*[0-9a-f]*:\s*1d32\s*ixh\s*r2, r2, r3
\s*[0-9a-f]*:\s*1f43\s*andn\s*r3, r3, r4
\s*[0-9a-f]*:\s*21f3\s*addi\s*r3, r3, 32
\s*[0-9a-f]*:\s*23f3\s*cmplti\s*r3, 32
\s*[0-9a-f]*:\s*2413\s*subi\s*r3, r3, 2
\s*[0-9a-f]*:\s*2823\s*rsubi\s*r3, r3, 2
\s*[0-9a-f]*:\s*2a33\s*cmpnei\s*r3, 3
\s*[0-9a-f]*:\s*2c83\s*bmaski\s*r3, 8
\s*[0-9a-f]*:\s*2c13\s*divu\s*r3, r3, r1
\s*[0-9a-f]*:\s*2c22\s*mflos\s*r2
\s*[0-9a-f]*:\s*2c32\s*mfhis\s*r2
\s*[0-9a-f]*:\s*2c42\s*mtlo\s*r2
\s*[0-9a-f]*:\s*2c52\s*mthi\s*r2
\s*[0-9a-f]*:\s*2c62\s*mflo\s*r2
\s*[0-9a-f]*:\s*2c72\s*mfhi\s*r2
\s*[0-9a-f]*:\s*2e33\s*andi\s*r3, r3, 3
\s*[0-9a-f]*:\s*3033\s*bclri\s*r3, r3, 3
\s*[0-9a-f]*:\s*3293\s*bgeni\s*r3, 9
\s*[0-9a-f]*:\s*6403\s*movi\s*r3, 64
\s*[0-9a-f]*:\s*3213\s*divs\s*r3, r3, r1
\s*[0-9a-f]*:\s*1221\s*mov\s*r1, r2
\s*[0-9a-f]*:\s*3213\s*divs\s*r3, r3, r1
\s*[0-9a-f]*:\s*3493\s*bseti\s*r3, r3, 9
\s*[0-9a-f]*:\s*3693\s*btsti\s*r3, 9
\s*[0-9a-f]*:\s*3803\s*xsr\s*r3, r3, 1
\s*[0-9a-f]*:\s*3823\s*rotli\s*r3, r3, 2
\s*[0-9a-f]*:\s*3a03\s*asrc\s*r3, r3, 1
\s*[0-9a-f]*:\s*3a31\s*asri\s*r1, r1, 3
\s*[0-9a-f]*:\s*67f7\s*movi\s*r7, 127
\s*[0-9a-f]*:\s*8200\s*ld.w\s*r2,\s*\(r0,\s*0x0\)
\s*[0-9a-f]*:\s*8210\s*ld.w\s*r2,\s*\(r0,\s*0x4\)
\s*[0-9a-f]*:\s*8220\s*ld.w\s*r2,\s*\(r0,\s*0x8\)
\s*[0-9a-f]*:\s*9200\s*st.w\s*r2,\s*\(r0,\s*0x0\)
\s*[0-9a-f]*:\s*9210\s*st.w\s*r2,\s*\(r0,\s*0x4\)
\s*[0-9a-f]*:\s*9220\s*st.w\s*r2,\s*\(r0,\s*0x8\)
\s*[0-9a-f]*:\s*c210\s*ld.h\s*r2,\s*\(r0,\s*0x2\)
\s*[0-9a-f]*:\s*c220\s*ld.h\s*r2,\s*\(r0,\s*0x4\)
\s*[0-9a-f]*:\s*d210\s*st.h\s*r2,\s*\(r0,\s*0x2\)
\s*[0-9a-f]*:\s*d220\s*st.h\s*r2,\s*\(r0,\s*0x4\)
\s*[0-9a-f]*:\s*a200\s*ld.b\s*r2,\s*\(r0,\s*0x0\)
\s*[0-9a-f]*:\s*a210\s*ld.b\s*r2,\s*\(r0,\s*0x1\)
\s*[0-9a-f]*:\s*b200\s*st.b\s*r2,\s*\(r0,\s*0x0\)
\s*[0-9a-f]*:\s*b210\s*st.b\s*r2,\s*\(r0,\s*0x1\)
\s*[0-9a-f]*:\s*e798\s*bt\s*0x0.*
\s*[0-9a-f]*:\s*ef97\s*bf\s*0x0.*
\s*[0-9a-f]*:\s*f796\s*br\s*0x0.*
\s*[0-9a-f]*:\s*0c00\s*cmphs\s*r0, r0
\s*[0-9a-f]*:\s*0f00\s*cmpne\s*r0, r0
\s*[0-9a-f]*:\s*2205\s*cmplti\s*r5, 1
\s*[0-9a-f]*:\s*2263\s*cmplti\s*r3, 7
\s*[0-9a-f]*:\s*2807\s*rsubi\s*r7, r7, 0
\s*[0-9a-f]*:\s*2a06\s*cmpnei\s*r6, 0
\s*[0-9a-f]*:\s*37f0\s*btsti\s*r0, 31
\s*[0-9a-f]*:\s*31f3\s*bclri\s*r3, r3, 31
\s*[0-9a-f]*:\s*6404\s*movi\s*r4, 64
\s*[0-9a-f]*:\s*3274\s*bgeni\s*r4, 7
\s*[0-9a-f]*:\s*3501\s*bseti\s*r1, r1, 16
\s*[0-9a-f]*:\s*3644\s*btsti\s*r4, 4
\s*[0-9a-f]*:\s*38c6\s*rotli\s*r6, r6, 12
\s*[0-9a-f]*:\s*39f2\s*rotli\s*r2, r2, 31
\s*[0-9a-f]*:\s*1200\s*mov\s*r0, r0
\s*[0-9a-f]*:\s*0007\s*idly4
\s*[0-9a-f]*:\s*0644\s*addc\s*r4, r4, r4
\s*[0-9a-f]*:\s*0655\s*addc\s*r5, r5, r5
\s*[0-9a-f]*:\s*0132\s*xtrb0\s*r1, r2
\s*[0-9a-f]*:\s*0151\s*sextb\s*r1, r1
\s*[0-9a-f]*:\s*0123\s*xtrb1\s*r1, r3
\s*[0-9a-f]*:\s*0151\s*sextb\s*r1, r1
\s*[0-9a-f]*:\s*0114\s*xtrb2\s*r1, r4
\s*[0-9a-f]*:\s*0151\s*sextb\s*r1, r1
\s*[0-9a-f]*:\s*0221\s*movt\s*r1, r2
\s*[0-9a-f]*:\s*0a31\s*movf\s*r1, r3
\s*[0-9a-f]*:\s*0d22\s*cmplt\s*r2, r2
\s*[0-9a-f]*:\s*0672\s*addc\s*r2, r2, r7
\s*[0-9a-f]*:\s*0683\s*addc\s*r3, r3, r8
\s*[0-9a-f]*:\s*0c44\s*cmphs\s*r4, r4
\s*[0-9a-f]*:\s*0764\s*subc\s*r4, r4, r6
\s*[0-9a-f]*:\s*0775\s*subc\s*r5, r5, r7
\s*[0-9a-f]*:\s*1e26\s*or\s*r6, r6, r2
\s*[0-9a-f]*:\s*1e37\s*or\s*r7, r7, r3
\s*[0-9a-f]*:\s*1715\s*xor\s*r5, r5, r1
\s*[0-9a-f]*:\s*1726\s*xor\s*r6, r6, r2
#...
