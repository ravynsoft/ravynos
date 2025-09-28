;/*------------------------------------------------------------*/
;/* filename -       edits.asm                                 */
;/*                                                            */
;/* function(s)                                                */
;/*                  TEditor member functions                  */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 

        PUBLIC  @TEditor@bufChar$qui
        PUBLIC  @TEditor@bufPtr$qui
IFNDEF __FLAT__
        PUBLIC  @TEditor@formatLine$qnusuiius
ELSE
        PUBLIC  @TEditor@formatLine$qpusuiius
ENDIF
        PUBLIC  @TEditor@lineEnd$qui
        PUBLIC  @TEditor@lineStart$qui
        PUBLIC  @TEditor@nextChar$qui
        PUBLIC  @TEditor@prevChar$qui
        PUBLIC  _scan, _iScan, _countLines

        INCLUDE TV.INC

CODESEG

;char TEditor::bufChar( uint P )
@TEditor@bufChar$qui PROC

        ARG     thisPtr :PTR, P : ARGINT
IFNDEF __FLAT__
        USES    DI

        LES     DI, [thisPtr]
        MOV     BX, [P]
        CMP     BX, ES:[DI+TEditorCurPtr]
        JB    @@1
        ADD     BX, ES:[DI+TEditorGapLen]
@@1:    LES     DI, ES:[DI+TEditorBuffer]
        MOV     AL, ES:[DI+BX]
        RET
ELSE
        USES    EDI, EBX

        MOV     EDI, [thisPtr]
        MOV     EBX, [P]
        CMP     EBX, [EDI+TEditorCurPtr]
        JB    @@1
        ADD     EBX, [EDI+TEditorGapLen]
@@1:    MOV     EDI, [EDI+TEditorBuffer]
        MOV     AL, [EDI+EBX]
        RET
ENDIF
@TEditor@bufChar$qui ENDP

;function TEditor.bufPtr(P: uint): uint; assembler;

@TEditor@bufPtr$qui PROC
        ARG     thisPtr : PTR, P : ARGINT
IFNDEF __FLAT__
        USES    DI

        LES     DI, [thisPtr]
        MOV     AX, [P]
        CMP     AX, ES:[DI+TEditorCurPtr]
        JB    @@1
        ADD     AX, ES:[DI+TEditorGapLen]
@@1:    RET
ELSE
        USES    EBX

        MOV     EBX, [thisPtr]
        MOV     EAX, [P]
        CMP     EAX, [EBX+TEditorCurPtr]
        JB    @@1
        ADD     EAX, [EBX+TEditorGapLen]
@@1:    RET
ENDIF

ENDP @TEditor@bufPtr$qui

;void TEditor::formatLine( ushort *DrawBuf,
;                          uint LinePtr,
;                          int Width,
;                          ushort Colors
;                        )
IFNDEF __FLAT__
@TEditor@formatLine$qnusuiius PROC
ELSE
@TEditor@formatLine$qpusuiius PROC
ENDIF
        ARG     thisPtr:PTR, DrawBuf:PTR, LinePtr:ARGINT, \
                        W:ARGINT, Colors:WORD
IFNDEF __FLAT__
        USES    DS, SI, DI

        LDS     BX, [thisPtr]
        LES     DI, [DrawBuf]
        MOV     SI, [LinePtr]
        XOR     DX, DX
        CLD
        MOV     AH, BYTE PTR [Colors]
        MOV     CX, DS:[BX+TEditorSelStart]
        CALL  @@10
        MOV     AH, BYTE PTR [Colors+1]
        MOV     CX, DS:[BX+TEditorCurPtr]
        CALL  @@10
        ADD     SI, DS:[BX+TEditorGapLen]
        MOV     CX, DS:[BX+TEditorSelEnd]
        ADD     CX, DS:[BX+TEditorGapLen]
        CALL  @@10
        MOV     AH, BYTE PTR [Colors]
        MOV     CX, DS:[BX+TEditorBufSize]
        CALL  @@10
        JMP   @@31
@@10:   SUB     CX, SI
        JA    @@11
        RETN
@@11:   LDS     BX, DS:[BX+TEditorBuffer]
        ADD     SI, BX
        MOV     BX, [W]
@@12:   LODSB
        CMP     AL, ' '
        JB    @@20
@@13:   STOSW
        INC     DX
@@14:   CMP     DX, BX
        JAE   @@30
        LOOP  @@12
        LDS     BX, [thisPtr]
        SUB     SI, WORD PTR DS:[BX+TEditorBuffer]
        RETN
@@20:   CMP     AL, 0DH
        JE    @@30
        CMP     AL, 0AH
        JE    @@30
        CMP     AL, 09H
        JNE   @@13
        MOV     AL, ' '
@@21:   STOSW
        INC     DX
        CMP     DX, BX
        JAE   @@30
        TEST    DL, 7
        JNE   @@21
        JMP   @@14
@@30:   POP     CX
@@31:   MOV     AL, ' '
        MOV     CX, [W]
        SUB     CX, DX
        JBE   @@32
        REP     STOSW
@@32:   RET
ELSE        ;;;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit ;;;;;;;;;;;;;;;;;;;;;;;;;;;
        USES    ESI, EDI, EBX

        MOV     EBX, DWORD PTR [thisPtr]
        MOV     EDI, DWORD PTR [DrawBuf]
        MOV     ESI, [LinePtr]
        XOR     EDX, EDX
        CLD
        MOV     AH, BYTE PTR [Colors]
        MOV     ECX, [EBX+TEditorSelStart]
        CALL  @@10
        MOV     AH, BYTE PTR [Colors+1]
        MOV     ECX, [EBX+TEditorCurPtr]
        CALL  @@10
        MOV     ECX, [EBX+TEditorGapLen]
        ADD     ESI, ECX
        MOV     ECX, [EBX+TEditorSelEnd]
        ADD     ECX, [EBX+TEditorGapLen]
        CALL  @@10
        MOV     AH, BYTE PTR [Colors]
        MOV     ECX, [EBX+TEditorBufSize]
        CALL  @@10
        JMP   @@31

@@10:   SUB     ECX, ESI
        JA    @@11
        RETN
@@11:   MOV     EBX, [EBX+TEditorBuffer]
        ADD     ESI, EBX
        MOV     EBX, [W]
@@12:   LODSB
        CMP     AL, ' '
        JB    @@20
@@13:   STOSW
        INC     EDX
@@14:   CMP     EDX, EBX
        JAE   @@30
        LOOP  @@12
        MOV     EBX, [thisPtr]
        SUB     ESI, [EBX+TEditorBuffer]
        RETN
@@20:   CMP     AL, 0DH
        JE    @@30
        CMP     AL, 0AH
        JE    @@30
        CMP     AL, 09H
        JNE   @@13
        MOV     AL, ' '
@@21:   STOSW
        INC     EDX
        CMP     EDX, EBX
        JAE   @@30
        TEST    DL, 7
        JNE   @@21
        JMP   @@14

@@30:   POP     ECX
@@31:   MOV     AL, ' '
        MOV     ECX, [W]
        SUB     ECX, EDX
        JBE   @@32
        REP     STOSW
@@32:   RET
ENDIF

IFNDEF __FLAT__
ENDP @TEditor@formatLine$qnusuiius
ELSE
ENDP @TEditor@formatLine$qpusuiius
ENDIF

;function TEditor.lineEnd(P: uint): uint; assembler;
@TEditor@lineEnd$qui PROC

        ARG     thisPtr: PTR, P:ARGINT
IFNDEF __FLAT__
        USES    DS, SI, DI

        LDS     SI, [thisPtr]
        LES     BX, DS:[SI+TEditorBuffer]
        MOV     DI, [P]
        MOV     AL, 0AH
        CLD
        MOV     CX, DS:[SI+TEditorCurPtr]
        SUB     CX, DI
        JBE   @@1
        ADD     DI, BX
        REPNE   SCASB
        JE    @@2
        MOV     DI, DS:[SI+TEditorCurPtr]
@@1:    MOV     CX, DS:[SI+TEditorBufLen]
        SUB     CX, DI
        JCXZ  @@4
        ADD     BX, DS:[SI+TEditorGapLen]
        ADD     DI, BX
        REPNE   SCASB
        JNE   @@3
@@2:    DEC     DI
        CMP     BYTE PTR ES:[DI], 0DH
        JNE   @@3
        DEC     DI
@@3:    SUB     DI, BX
@@4:    MOV     AX, DI
        RET
ELSE        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;32-bit;;;;;;;;;;;;;;;;;;;;;;;;;;
        USES    ESI, EDI, EBX

        MOV     ESI, DWORD PTR [thisPtr]
        MOV     EBX, [ESI+TEditorBuffer]
        MOV     EDI, [P]
        MOV     AL, 0AH
        CLD
        MOV     ECX, [ESI+TEditorCurPtr]
        SUB     ECX, EDI
        JBE   @@1
        ADD     EDI, EBX
        REPNE   SCASB
        JE    @@2
        MOV     EDI, [ESI+TEditorCurPtr]
@@1:    MOV     ECX, [ESI+TEditorBufLen]
        SUB     ECX, EDI
        JCXZ  @@4
        MOV     EDX, [ESI+TEditorGapLen]
        ADD     EBX, EDX
        ADD     EDI, EBX
        REPNE   SCASB
        JNE   @@3
@@2:    DEC     EDI
        CMP     BYTE PTR [EDI], 0DH
        JNE   @@3
        DEC     EDI
@@3:    SUB     EDI, EBX
@@4:    MOV     EAX, EDI
        RET

ENDIF
@TEditor@lineEnd$qui ENDP

;function TEditor.lineStart(P: uint): uint; assembler;
@TEditor@lineStart$qui PROC

        ARG     thisPtr :  PTR, P : ARGINT
IFNDEF __FLAT__
        USES    DS, SI, DI

        LDS     SI, [thisPtr]
        LES     BX, DS:[SI+TEditorBuffer]
        MOV     DI, [P]
        MOV     AL, 0AH
        STD
        MOV     CX, DI
        SUB     CX, DS:[SI+TEditorCurPtr]
        JBE   @@1
        ADD     BX, DS:[SI+TEditorGapLen]
        ADD     DI, BX
        DEC     DI
        REPNE   SCASB
        JE    @@2
        SUB     BX, DS:[SI+TEditorGapLen]
        MOV     DI, DS:[SI+TEditorCurPtr]
@@1:    MOV     CX, DI
        JCXZ  @@4
        ADD     DI, BX
        DEC     DI
        REPNE   SCASB
        JNE   @@3
@@2:    INC     DI
        SUB     DI, BX
        CMP     DI, DS:[SI+TEditorCurPtr]
        JE    @@4
        CMP     DI, DS:[SI+TEditorBufLen]
        JE    @@4
        CMP     BYTE PTR ES:[BX+DI], 0AH
        JNE   @@4
        INC     DI
        JMP   @@4
@@3:    XOR     DI, DI
@@4:    MOV     AX, DI
        CLD
        RET
ELSE
        USES    ESI, EDI, EBX

        MOV     ESI, [thisPtr]
        MOV     EBX, [ESI+TEditorBuffer]
        MOV     EDI, [P]
        MOV     AL, 0AH
        STD
        MOV     ECX, EDI
        MOV     EDX, [ESI+TEditorCurPtr]
        SUB     ECX, EDX
        JBE   @@1
        MOV     EDX, [ESI+TEditorGapLen]
        ADD     EBX, EDX
        ADD     EDI, EBX
        DEC     EDI
        REPNE   SCASB
        JE    @@2
        MOV     EDX, [ESI+TEditorGapLen]
        SUB     EBX, EDX
        MOV     EDI, [ESI+TEditorCurPtr]
@@1:    MOV     ECX, EDI
        JECXZ @@4
        ADD     EDI, EBX
        DEC     EDI
        REPNE   SCASB
        JNE   @@3
@@2:    INC     EDI
        SUB     EDI, EBX
        CMP     EDI, [ESI+TEditorCurPtr]
        JE    @@4
        CMP     EDI, [ESI+TEditorBufLen]
        JE      @@4
        CMP     BYTE PTR [EBX+EDI], 0AH
        JNE   @@4
        INC     EDI
        JMP   @@4
@@3:    XOR     EDI, EDI
@@4:    MOV     EAX, EDI
        CLD
        RET
ENDIF
@TEditor@lineStart$qui ENDP

;function TEditor.nextChar(P: uint): uint; assembler;
@TEditor@nextChar$qui PROC

        ARG     thisPtr : PTR, P : ARGINT

IFNDEF __FLAT__
        USES    DS, SI, DI

        LDS     SI, [thisPtr]
        MOV     DI, [P]
        CMP     DI, DS:[SI+TEditorBufLen]
        JE    @@2
        INC     DI
        CMP     DI, DS:[SI+TEditorBufLen]
        JE    @@2
        LES     BX, DS:[SI+TEditorBuffer]
        CMP     DI, DS:[SI+TEditorCurPtr]
        JB    @@1
        ADD     BX, DS:[SI+TEditorGapLen]
@@1:    CMP     WORD PTR ES:[BX+DI-1], 0A0DH
        JNE   @@2
        INC     DI
@@2:    MOV     AX, DI
        RET
ELSE
        USES    ESI, EDI, EBX

        MOV     ESI, [thisPtr]
        MOV     EDI, [P]
        CMP     EDI, [ESI+TEditorBufLen]
        JE    @@2
        INC     EDI
        CMP     EDI, [ESI+TEditorBufLen]
        JE    @@2
        MOV     EBX, [ESI+TEditorBuffer]
        CMP     EDI, [ESI+TEditorCurPtr]
        JB    @@1
        MOV     EDX, [ESI+TEditorGapLen]
        ADD     EBX, EDX
@@1:    CMP     WORD PTR [EBX+EDI-1], 0A0DH
        JNE   @@2
        INC     EDI
@@2:    MOV     EAX, EDI
        RET
ENDIF

@TEditor@nextChar$qui ENDP

;function TEditor.prevChar(P: uint): uint; assembler;
@TEditor@prevChar$qui PROC

        ARG     thisPtr :  PTR, P : ARGINT
IFNDEF __FLAT__
        USES    DS, SI, DI

        LDS     SI, [thisPtr]
        MOV     DI, [P]
        OR      DI, DI
        JE    @@2
        DEC     DI
        JE    @@2
        LES     BX, DS:[SI+TEditorBuffer]
        CMP     DI, DS:[SI+TEditorCurPtr]
        JB    @@1
        ADD     BX, DS:[SI+TEditorGapLen]
@@1:    CMP     WORD PTR ES:[BX+DI-1], 0A0DH
        JNE   @@2
        DEC     DI
@@2:    MOV     AX, DI
        RET
ELSE        ;;;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        USES   ESI, EDI, EBX

        MOV     ESI, DWORD PTR [thisPtr]
        MOV     EDI, [P]
        OR      EDI, EDI
        JE    @@2
        DEC     EDI
        JE    @@2
        MOV     EBX, [ESI+TEditorBuffer]
        MOV     EDX, [ESI+TEditorCurPtr]
        CMP     EDI, EDX
        JB    @@1
        MOV     EDX, [ESI+TEditorGapLen]
        ADD     EBX, EDX
@@1:    CMP     WORD PTR [EBX+EDI-1], 0A0DH
        JNE   @@2
        DEC     EDI
@@2:    MOV     EAX, EDI
        RET
ENDIF
@TEditor@prevChar$qui ENDP


;-----------------------------------------------------------------------------

_countLines PROC C

        ARG buf:PTR, count:ARGINT

IFNDEF __FLAT__
        USES DI, SI

        LES     DI, buf
        MOV     CX, count
        XOR     DX, DX
        MOV     AL, 0Ah
        CLD
@@1:
        JCXZ  @@2
        REPNE   SCASB
        JNE   @@2
        INC     DX
        JMP   @@1
@@2:
        MOV     AX, DX
        RET
ELSE
        USES    EDI, ESI

        MOV     EDI, DWORD PTR [buf]
        MOV     ECX, count
        XOR     EDX, EDX
        MOV     AL, 0Ah
        CLD
@@1:
        JECXZ @@2
        REPNE   SCASB
        JNE   @@2
        INC     EDX
        JMP   @@1
@@2:
        MOV   EAX, EDX
        RET
ENDIF

ENDP



_scan PROC C
        ARG block:PTR, bsize:ARGINT, pstr:PTR
        LOCAL len:ARGINT

IFNDEF __FLAT__
        USES SI, DI

        LES     DI, pstr
        MOV     BX, DI
        MOV     CX, DI
        NOT     CX
        XOR     AL, AL
        CLD
        REPNE   SCASB
        SUB     DI, BX
        DEC     DI
        MOV     len, DI

        PUSH    DS
        LES     DI, block
        LDS     SI, pstr
        MOV     CX, bsize
        JCXZ  @@3
        CLD
        MOV     AX, len
        CMP     AX, 1
        JB    @@5
        JA    @@1
        LODSB           ;; searching for a single character
        REPNE   SCASB
        JNE   @@3
        JMP   @@5
@@1:
        MOV     BX, AX
        DEC     BX
        MOV     DX, CX
        SUB     DX, AX
        JB    @@3
        LODSB
        INC     DX
        INC     DX
@@2:
        DEC     DX
        MOV     CX, DX
        REPNE   SCASB
        JNE   @@3
        MOV     DX, CX
        MOV     CX, BX
        REP     CMPSB
        JE    @@4
        SUB     CX, BX
        ADD     SI, CX
        ADD     DI, CX
        INC     DI
        OR      DX, DX
        JNE   @@2
@@3:
        XOR     AX, AX
        JMP   @@6
@@4:
        SUB     DI, BX
@@5:
        MOV     AX, DI
        SUB     AX, WORD PTR block
@@6:
        DEC     AX
        POP     DS

        RET
ELSE                                ; 32-bit version
        USES ESI, EDI, EBX

        MOV     EDI, DWORD PTR [pstr]

        MOV     EBX, EDI
        MOV     ECX, EDI
        NOT     ECX
        XOR     AL, AL
        CLD
        REPNE   SCASB
        SUB     EDI, EBX
        DEC     EDI
        MOV     len, EDI

        MOV     EDI, DWORD PTR [block]
        MOV     ESI, DWORD PTR [pstr]
        MOV     ECX, bsize
        JECXZ @@3
        CLD
        MOV     EAX, len
        CMP     EAX, 1
        JB    @@5
        JA    @@1
        LODSB           ;; searching for a single character
        REPNE   SCASB
        JNE   @@3
        JMP   @@5
@@1:
        MOV     EBX, EAX
        DEC     EBX
        MOV     EDX, ECX
        SUB     EDX, EAX
        JB    @@3
        LODSB
        INC     EDX
        INC     EDX
@@2:
        DEC     EDX
        MOV     ECX, EDX
        REPNE   SCASB
        JNE   @@3
        MOV     EDX, ECX
        MOV     ECX, EBX
        REP     CMPSB
        JE    @@4
        SUB     ECX, EBX
        ADD     ESI, ECX
        ADD     EDI, ECX
        INC     EDI
        OR      EDX, EDX
        JNE   @@2
@@3:
        XOR     EAX, EAX
        JMP   @@6
@@4:
        SUB     EDI, EBX
@@5:
        MOV     EAX, EDI
        SUB     EAX, DWORD PTR [block]
@@6:
        DEC     EAX
        RET
ENDIF
ENDP

_iScan PROC C
        ARG block:PTR, bsize:ARGINT, pstr:PTR
        LOCAL len:ARGINT, s:BYTE:256

IFNDEF __FLAT__
        USES SI, DI

        LES     DI, pstr
        MOV     BX, DI
        MOV     CX, DI
        NOT     CX
        XOR     AL, AL
        CLD
        REPNE   SCASB
        SUB     DI, BX
        DEC     DI
        MOV     len, DI

        PUSH    DS
        MOV     AX, SS
        MOV     ES, AX
        LEA     DI, s
        LDS     SI, pstr
        MOV     AX, len;
        MOV     CX, AX
        MOV     BX, AX
        JCXZ  @@9
@@1:
        LODSB
        CMP     AL, 'a'
        JB    @@2
        CMP     AL, 'z'
        JA    @@2
        SUB     AL, 20h
@@2:
        STOSB
        LOOP  @@1
        SUB     DI, BX
        LDS     SI, block
        MOV     CX, bsize
        JCXZ  @@8
        CLD
        SUB     CX, BX
        JB    @@8
        INC     CX
@@4:
        MOV     AH, ES:[DI]
        AND     AH, 0DFh
@@5:
        LODSB
        AND     AL, 0DFh
        CMP     AL, AH
        LOOPNE @@5
        JNE   @@8
        DEC     SI
        MOV     DX, CX
        MOV     CX, BX
@@6:
        REPE    CMPSB
        JE    @@10
        MOV     AL, DS:[SI-1]
        CMP     AL, 'a'
        JB    @@7
        CMP     AL, 'z'
        JA    @@7
        SUB     AL, 20h
@@7:
        CMP     AL, ES:[DI-1]
        JE    @@6
        SUB     CX, BX
        ADD     SI, CX
        ADD     DI, CX
        INC     SI
        MOV     CX, DX
        OR      CX, DX
        JNE   @@4
@@8:
        XOR     AX, AX
        JMP   @@11
@@9:
        MOV     AX, 1
        JMP   @@11
@@10:
        SUB     SI, BX
        MOV     AX, SI
        SUB     AX, WORD PTR block
        INC     AX
@@11:
        DEC     AX
        POP     DS
        RET
ELSE                ; 32-bit version
        USES ESI, EDI, EBX

        MOV     EDI, DWORD PTR [pstr]

        MOV     EBX, EDI
        MOV     ECX, EDI
        NOT     ECX
        XOR     AL, AL
        CLD
        REPNE   SCASB
        SUB     EDI, EBX
        DEC     EDI
        MOV     len, EDI

        LEA     EDI, s
        MOV     ESI, DWORD PTR [pstr]
        MOV     EAX, len;
        MOV     ECX, EAX
        MOV     EBX, EAX
        JECXZ @@9
@@1:
        LODSB
        CMP     AL, 'a'
        JB    @@2
        CMP     AL, 'z'
        JA    @@2
        SUB     AL, 20h
@@2:
        STOSB
        LOOP @@1
        SUB     EDI, EBX
        MOV     ESI, DWORD PTR [block]
        MOV     ECX, bsize
        JECXZ @@8
        CLD
        SUB     ECX, EBX
        JB    @@8
        INC     ECX
@@4:
        MOV     AH, ES:[EDI]
        AND     AH, 0DFh
@@5:
        LODSB
        AND     AL, 0DFh
        CMP     AL, AH
        LOOPNE @@5
        JNE   @@8
        DEC     ESI
        MOV     EDX, ECX
        MOV     ECX, EBX
@@6:
        REPE    CMPSB
        JE    @@10
        MOV     AL, DS:[ESI-1]
        CMP     AL, 'a'
        JB    @@7
        CMP     AL, 'z'
        JA    @@7
        SUB     AL, 20h
@@7:
        CMP     AL, ES:[EDI-1]
        JE    @@6
        SUB     ECX, EBX
        ADD     ESI, ECX
        ADD     EDI, ECX
        INC     ESI
        MOV     ECX, EDX
        OR      ECX, ECX
        JNE   @@4
@@8:
        XOR     EAX, EAX
        JMP   @@11
@@9:
        MOV     EAX, 1
        JMP   @@11
@@10:
        SUB     ESI, EBX
        MOV     EAX, ESI
        SUB     EAX, DWORD PTR [block]
        INC     EAX
@@11:
        DEC     EAX
        RET
ENDIF
ENDP

        END

