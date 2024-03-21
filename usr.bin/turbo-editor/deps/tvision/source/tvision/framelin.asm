;/*------------------------------------------------------------*/
;/* filename -  framelin.asm                                   */
;/*                                                            */
;/* function(s)                                                */
;/*             TFrame frameLine member function               */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 

ifndef __FLAT__
        PUBLIC  @TFrame@frameLine$qm11TDrawBufferssuc
else
        PUBLIC  @TFrame@frameLine$qr11TDrawBufferssuc
endif
        EXTRN   @TFrame@initFrame : BYTE
        EXTRN   @TFrame@frameChars : BYTE

        INCLUDE TV.INC

        MaxViewWidth   equ   132

DATASEG

FrameMask   DB   MaxViewWidth dup(?)

CODESEG

ifndef __FLAT__
@TFrame@frameLine$qm11TDrawBufferssuc PROC
else
@TFrame@frameLine$qr11TDrawBufferssuc PROC
endif
        ARG   thisPtr:PTR, FrameBuf:PTR, Y:ARGINT, N:ARGINT, Color:BYTE

ifndef __FLAT__
        USES    SI,DI

        LES     BX, [thisPtr]
        MOV     DX, ES:[BX+TFrameSizeX]
        MOV     CX, DX
        DEC     CX
        DEC     CX
        MOV     SI, OFFSET @TFrame@initFrame
        ADD     SI, [N]
        LEA     DI, [FrameMask]
        PUSH    DS
        POP     ES
        CLD
        MOVSB
        LODSB
        REP     STOSB
        MOVSB
        LES     BX, [thisPtr]
        LES     BX, ES:[BX+TFrameOwner]
        LES     BX, ES:[BX+TGroupLast]
        DEC     DX
@@1:    LES     BX, ES:[BX+TViewNext]
        CMP     BX, WORD PTR [ thisPtr]
        JNE   @@2
        MOV     AX, ES
        CMP     AX, WORD PTR [ thisPtr+2]
        JE    @@10
@@2:    TEST    WORD PTR ES:[BX+TViewOptions], ofFramed
        JE    @@1
        TEST    WORD PTR ES:[BX+TViewState], sfVisible
        JE    @@1
        MOV     AX, [Y]
        SUB     AX, ES:[BX+TViewOriginY]
        JL    @@3
        CMP     AX, ES:[BX+TViewSizeY]
        JG    @@1
        MOV     AX, 0005H
        JL    @@4
        MOV     AX, 0A03H
        JMP   @@4
@@3:    INC     AX
        JNE   @@1
        MOV     AX, 0A06H
@@4:    MOV     SI, ES:[BX+TViewOriginX]
        MOV     DI, ES:[BX+TViewSizeX]
        ADD     DI, SI
        CMP     SI, 1
        JG    @@5
        MOV     SI, 1
@@5:    CMP     DI, DX
        JL    @@6
        MOV     DI, DX
@@6:    CMP     SI, DI
        JGE   @@1
        OR      [BYTE PTR FrameMask+SI-1], AL
        XOR     AL, AH
        OR      [BYTE PTR FrameMask+DI], AL
        OR      AH, AH
        JE    @@1
        MOV     CX, DI
        SUB     CX, SI
@@8:    OR      [BYTE PTR FrameMask+SI], AH
        INC     SI
        LOOP  @@8
        JMP   @@1
@@10:   INC     DX
        MOV     AH, [Color]
        MOV     BX, OFFSET @TFrame@frameChars
        MOV     CX, DX
        LEA     SI, [FrameMask]
        LES     DI, [FrameBuf]
        ADD     DI, TDrawBufferData
@@11:   LODSB
        XLAT
        STOSW
        LOOP    @@11
        RET

else    ;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit version ;;;;;;;;;;;;;;;;;;;;;;;;;;;

        USES    ESI,EDI,EBX

        XOR     EAX, EAX
        MOV     WORD PTR [Y+2], AX
        MOV     WORD PTR [N+2], AX

        MOV     EBX, DWORD PTR [thisPtr]
        MOV     EDX, [EBX+TFrameSizeX]
        MOV     ECX, EDX
        DEC     ECX
        DEC     ECX
        MOV     ESI, OFFSET @TFrame@initFrame
        ADD     ESI, [N]
        LEA     EDI, [FrameMask]
        CLD

        MOVSB
        LODSB
        MOV     AH, AL
        MOV     EBX, EAX
        SHL     EAX, 16
        MOV     AX, BX
        MOV     EBX, ECX
        SHR     ECX, 2
        REP     STOSD
        MOV     ECX, EBX
        AND     ECX, 03H
        REP STOSB
        MOVSB
        MOV     EBX, DWORD PTR [thisPtr]
        MOV     EBX, [EBX+TFrameOwner]
        MOV     EBX, [EBX+TGroupLast]
        DEC     EDX
@@1:    MOV     EBX, [EBX+TViewNext]
        CMP     EBX, DWORD PTR [thisPtr]
        JE    @@10
@@2:    TEST    WORD PTR [EBX+TViewOptions], ofFramed
        JE    @@1
        TEST    WORD PTR [EBX+TViewState], sfVisible
        JE    @@1
        MOV     EAX, [Y]
        SUB     EAX, [EBX+TViewOriginY]
        JL    @@3
        CMP     EAX, [EBX+TViewSizeY]
        JG    @@1
        MOV     EAX, 0005H
        JL    @@4
        MOV     EAX, 0A03H
        JMP   @@4
@@3:    INC     EAX
        JNE   @@1
        MOV     EAX, 0A06H
@@4:    MOV     ESI, [EBX+TViewOriginX]
        MOV     EDI, [EBX+TViewSizeX]
        ADD     EDI, ESI
        CMP     ESI, 1
        JG    @@5
        MOV     ESI, 1
@@5:    CMP     EDI, EDX
        JL    @@6
        MOV     EDI, EDX
@@6:    CMP     ESI, EDI
        JGE   @@1
        OR      [BYTE PTR FrameMask+ESI-1], AL
        XOR     AL,AH
        OR      [BYTE PTR FrameMask+EDI], AL
        OR      AH,AH
        JE    @@1
        MOV     ECX, EDI
        SUB     ECX, ESI
@@8:    OR      [BYTE PTR FrameMask+ESI], AH
        INC     ESI
        LOOP  @@8
        JMP   @@1
@@10:   INC     EDX
        MOV     AH, [Color]
        MOV     EBX, OFFSET @TFrame@frameChars
        MOV     ECX, EDX
        LEA     ESI, [FrameMask]
        MOV     EDI, DWORD PTR [FrameBuf]
        ADD     EDI, TDrawBufferData
@@11:   LODSB
        XLAT
        STOSW
        LOOP  @@11
        RET
endif
ifndef __FLAT__
@TFrame@frameLine$qm11TDrawBufferssuc ENDP
else
@TFrame@frameLine$qr11TDrawBufferssuc ENDP
endif

END
