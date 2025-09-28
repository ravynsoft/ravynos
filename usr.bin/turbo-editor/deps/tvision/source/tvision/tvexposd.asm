;/*------------------------------------------------------------*/
;/* filename -       tvexposd.asm                              */
;/*                                                            */
;/* function(s)                                                */
;/*                  TView exposed member function             */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
;

ifndef DEBUG_TVEXPOSD
        PUBLIC  @TView@exposed$qv
else
        PUBLIC  @TView@exposedAsm$qnvni
endif
        INCLUDE TV.INC

ifndef __FLAT__
Fptr   STRUC
        offs    DW      ?
        segm    DW      ?
Fptr   ENDS
else
Fptr   STRUC
        offs    DD      ?
Fptr   ENDS
endif

CODESEG

ifndef DEBUG_TVEXPOSD
@TView@exposed$qv PROC
        ARG     thisPtr : DWORD
        LOCAL   Target  : Fptr
else
@TView@exposedAsm$qnvni PROC FAR
        ARG     thisPtr : DWORD, v : DWORD, i : DWORD
        LOCAL   Target  : Fptr, z : WORD, _z : WORD
endif

ifndef __FLAT__
        USES    SI,DI
                                        ifdef DEBUG_TVEXPOSD
                                        mov     [z], 0
                                        mov     dx, 0
                                        call    putV
                                        endif
        XOR     AX,AX
        LES     DI, [thisPtr]
        TEST    WORD PTR ES:[DI+TViewState],sfExposed
        JE    @@2
        CMP     AX,ES:[DI+TViewSizeX]
        JGE   @@2
        CMP     AX,ES:[DI+TViewSizeY]
        JGE   @@2
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, [z]
                                        mov     [_z], dx
                                        endif
@@1:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, [_z]
                                        mov     [z], dx
                                        mov     dx, 1
                                        call    putV
                                        endif
        XOR     BX,BX
        MOV     CX,ES:[DI+TViewSizeX]
        PUSH    AX
        CALL  @@11
        POP     AX
        JNC   @@3
        LES     DI,[thisPtr]
        INC     AX
        CMP     AX,ES:[DI+TViewSizeY]
        JL    @@1
@@2:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 2
                                        call    putV
                                        mov     dx, ax
                                        call    putI
                                        endif
        MOV     AL,0
        JMP   @@30
@@3:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 3
                                        call    putV
                                        mov     dx, ax
                                        call    putI
                                        endif
        MOV     AL,1
        JMP   @@30
@@8:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 8
                                        call    putV
                                        endif
        STC
@@9:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 9
                                        call    putV
                                        endif
        RETN
@@10:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 10
                                        call    putV
                                        endif
        LES     DI,DWORD PTR ES:[DI+TViewOwner]
        CMP     WORD PTR ES:[DI+TGroupBuffer+2],0
        JNE   @@9
@@11:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 11
                                        call    putV
                                        endif
        MOV     [Target.offs],DI
        MOV     [Target.segm],ES
        ADD     AX,ES:[DI+TViewOriginY]
        MOV     SI,ES:[DI+TViewOriginX]
        ADD     BX,SI
        ADD     CX,SI
        LES     DI,ES:[DI+TViewOwner]
        MOV     SI,ES
        OR      SI,DI
        JE    @@9
        CMP     AX,ES:[DI+TGroupClipAY]
        JL    @@8
        CMP     AX,ES:[DI+TGroupClipBY]
        JGE   @@8
        CMP     BX,ES:[DI+TGroupClipAX]
        JGE   @@12
        MOV     BX,ES:[DI+TGroupClipAX]
@@12:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 12
                                        call    putV
                                        endif
        CMP     CX,ES:[DI+TGroupClipBX]
        JLE   @@13
        MOV     CX,ES:[DI+TGroupClipBX]
@@13:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 13
                                        call    putV
                                        endif
        CMP     BX,CX
        JGE   @@8
        LES     DI,ES:[DI+TGroupLast]
@@20:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 20
                                        call    putV
                                        endif
        LES     DI,ES:[DI+TViewNext]
        CMP     DI,[Target.offs]
        JNE   @@21
        MOV     SI,ES
        CMP     SI,[Target.segm]
        JE    @@10
@@21:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 21
                                        call    putV
                                        endif
        TEST    WORD PTR ES:[DI+TViewState],sfVisible
        JE    @@20
        MOV     SI,ES:[DI+TViewOriginY]
        CMP     AX,SI
        JL    @@20
        ADD     SI,ES:[DI+TViewSizeY]
        CMP     AX,SI
        JGE   @@20
        MOV     SI,ES:[DI+TViewOriginX]
        CMP     BX,SI
        JL    @@22
        ADD     SI,ES:[DI+TViewSizeX]
        CMP     BX,SI
        JGE   @@20
        MOV     BX,SI
        CMP     BX,CX
        JL    @@20
        STC
        RETN
@@22:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 22
                                        call    putV
                                        endif
        CMP     CX,SI
        JLE   @@20
        ADD     SI,ES:[DI+TViewSizeX]
        CMP     CX,SI
        JG    @@23
        MOV     CX,ES:[DI+TViewOriginX]
@@@20:
        JMP   @@20
@@23:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 23
                                        call    putV
                                        endif
        PUSH    [Target.segm]
        PUSH    [Target.offs]
        PUSH    ES
        PUSH    DI
        PUSH    SI
        PUSH    CX
        PUSH    AX
        MOV     CX,ES:[DI+TViewOriginX]
        CALL  @@20
        POP     AX
        POP     CX
        POP     BX
        POP     DI
        POP     ES
        POP     [Target.offs]
        POP     [Target.segm]
        JC    @@@20
        RETN
@@30:
                                        ifdef DEBUG_TVEXPOSD
                                        mov     dx, 30
                                        call    putV
                                        mov     dx, -1
                                        call    putV
                                        endif
        RET

ifdef DEBUG_TVEXPOSD
putI PROC NEAR
        pushf
        push di
        push es
    	les	di, dword ptr [i]
    	mov	word ptr es:[di],dx
        pop es
        pop di
        popf
        retn
putI ENDP

putV PROC NEAR
        pushf
        push si
        push di
        push es
        mov	si, [z]
    	les	di, dword ptr [v]
    	add	di, si
    	mov	word ptr es:[di],dx
        inc [z]
        pop es
        pop di
        pop si
        popf
        retn
putV ENDP
endif

else        ;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit version ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        USES    ESI, EDI, EBX

        MOV     EDI, DWORD PTR [thisPtr]
        TEST    WORD PTR [EDI+TViewState], sfExposed
        JZ    @@2
        XOR     EAX, EAX
        CMP     AX, [EDI+TViewSizeX]
        JGE   @@2
        CMP     AX, [EDI+TViewSizeY]
        JGE   @@2
@@1:
        XOR     EBX, EBX
        MOV     CX, [EDI+TViewSizeX]
        PUSH    EAX
        CALL  @@11
        POP     EAX
        JNC   @@3
        MOV     EDI, [thisPtr]
        INC     EAX
        CMP     AX, [EDI+TViewSizeY]
        JL    @@1
@@2:
        XOR     EAX, EAX
        JMP   @@30
@@3:
        MOV     EAX, 1
        JMP   @@30
@@8:
        STC
@@9:
        LocalRet
@@10:
        MOV     EDI,DWORD PTR [EDI+TViewOwner]
        CMP     DWORD PTR [EDI+TGroupBuffer], 0
        JNE   @@9
        CMP     WORD PTR [EDI+TGroupBuffer+4], 0
        JNE   @@9
@@11:
        MOV     [Target.offs], EDI
        ADD     AX, [EDI+TViewOriginY]
        MOV     SI, [EDI+TViewOriginX]
        ADD     BX, SI
        ADD     CX, SI
        MOV     EDI, [EDI+TViewOwner]
        OR      EDI, EDI
        JZ    @@9
        CMP     AX, [EDI+TGroupClipAY]
        JL    @@8
        CMP     AX, [EDI+TGroupClipBY]
        JGE   @@8
        CMP     BX, [EDI+TGroupClipAX]
        JGE   @@12
        MOV     BX, [EDI+TGroupClipAX]
@@12:
        CMP     CX, [EDI+TGroupClipBX]
        JLE   @@13
        MOV     CX, [EDI+TGroupClipBX]
@@13:
        CMP     BX, CX
        JGE   @@8
        MOV     EDI, [EDI+TGroupLast]
@@20:
        MOV     EDI, [EDI+TViewNext]
        CMP     EDI, [Target.offs]
        JE    @@10
@@21:
        TEST    WORD PTR [EDI+TViewState], sfVisible
        JE    @@20
        MOV     SI, [EDI+TViewOriginY]
        CMP     AX, SI
        JL    @@20
        ADD     SI, [EDI+TViewSizeY]
        CMP     AX, SI
        JGE   @@20
        MOV     SI, [EDI+TViewOriginX]
        CMP     BX, SI
        JL    @@22
        ADD     SI, [EDI+TViewSizeX]
        CMP     BX, SI
        JGE   @@20
        MOV     BX, SI
        CMP     BX, CX
        JL    @@20
        STC
        LocalRet
@@22:
        CMP     CX, SI
        JLE   @@20
        ADD     SI, [EDI+TViewSizeX]
        CMP     CX, SI
        JG    @@23
        MOV     CX, [EDI+TViewOriginX]
        JMP   @@20
@@23:
        PUSH    [Target.offs]
        PUSH    EDI
        PUSH    ESI
        PUSH    ECX
        PUSH    EAX
        MOV     CX,[EDI+TViewOriginX]
        CALL  @@20
        POP     EAX
        POP     ECX
        POP     EBX
        POP     EDI
        POP     [Target.offs]
        JC    @@20
        LocalRet
@@30:
        RET
endif
ENDP

END
endif
