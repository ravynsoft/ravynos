;/*------------------------------------------------------------*/
;/* filename -       tvcursor.asm                              */
;/*                                                            */
;/* function(s)                                                */
;/*                  TView resetCursor member function         */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 


        PUBLIC  @TView@resetCursor$qv

        EXTRN   @TDisplay@isEGAorVGA$qv : FAR
        EXTRN   @TScreen@cursorLines : WORD

IFDEF __FLAT__
        EXTRN   @THardwareInfo@setCaretSize$qus : NEAR
        EXTRN   @THardwareInfo@setCaretPosition$qusus : NEAR
ENDIF

        INCLUDE TV.INC

; 32-bit cursor information structures.
IFDEF __FLAT__

CONSOLE_CURSOR_INFO STRUC
    dwSize      dd  ?
    bVisible    dd  ?
CONSOLE_CURSOR_INFO ENDS

COORD STRUC
    crdX        dw  ?
    crdY        dw  ?
COORD ENDS

ENDIF
;

CODESEG

@TView@resetCursor$qv PROC
        ARG     thisPtr :  PTR

IFNDEF __FLAT__
        LOCAL   start : BYTE, end : BYTE, base : BYTE
        USES    SI, DI

        LES     DI, [thisPtr]

        MOV     AX, ES:[DI+TViewState]
        NOT     AX
        TEST    AX, sfVisible+sfCursorVis+sfFocused
        JNE   @@4
        MOV     AX, ES:[DI+TViewCursorY]
        MOV     DX, ES:[DI+TViewCursorX]
@@1:
        OR      AX, AX
        JL    @@4
        CMP     AX, ES:[DI+TViewSizeY]
        JGE   @@4
        OR      DX, DX
        JL    @@4
        CMP     DX, ES:[DI+TViewSizeX]
        JGE   @@4
        ADD     AX, ES:[DI+TViewOriginY]
        ADD     DX, ES:[DI+TViewOriginX]
        MOV     CX, DI
        MOV     BX, ES
        LES     DI, ES:[DI+TViewOwner]
        MOV     SI, ES
        OR      SI, DI
        JE    @@5
        TEST    WORD PTR ES:[DI+TViewState], sfVisible
        JE    @@4
        LES     DI, ES:[DI+TGroupLast]
@@2:
        LES     DI, ES:[DI+TViewNext]
        CMP     CX, DI
        JNE   @@3
        MOV     SI, ES
        CMP     BX, SI
        JNE   @@3
        LES     DI, ES:[DI+TViewOwner]
        JMP   @@1
@@3:
        TEST    WORD PTR ES:[DI+TViewState], sfVisible
        JE    @@2
        MOV     SI, ES:[DI+TViewOriginY]
        CMP     AX, SI
        JL    @@2
        ADD     SI, ES:[DI+TViewSizeY]
        CMP     AX, SI
        JGE   @@2
        MOV     SI, ES:[DI+TViewOriginX]
        CMP     DX, SI
        JL    @@2
        ADD     SI, ES:[DI+TViewSizeX]
        CMP     DX, SI
        JGE   @@2
@@4:
        MOV     CX, 2000H
        JMP   @@6
@@5:
        MOV     DH, AL
        XOR     BH, BH
        MOV     AH, 2
        INT     10H

; Calculate CH/CL cursor from NT percentage cursor.
        MOV     AX, WORD PTR [@TScreen@cursorLines]
        OR      AX, AX
        JE    @@4
        MOV     [start], AH
        MOV     [end], AL
        MOV     [base], 8

        CALL    @TDisplay@isEGAorVGA$qv
        JE      @@cga

        MOV     AX, 1130H
        XOR     BL, BL
        PUSH    BP
        INT     10h
        POP     BP
        MOV     [base], CL
@@cga:
        XOR     AH, AH
        MOV     AL, [start]
        IMUL    [base]
        MOV     BL, 64H
        ADD     AX, 32H
        DIV     BL
        MOV     CH, AL

        XOR     AH, AH
        MOV     AL, [end]
        IMUL    [base]
        ADD     AX, 32H
        DIV     BL
        MOV     CL, AL

        LES     DI, [thisPtr]
        TEST    WORD PTR ES:[DI+TViewState], sfCursorIns
        JE    @@6
        MOV     CH, 0
        OR      CL, CL
        JNE   @@6
        MOV     CL, 7
@@6:
        MOV     AH, 1
        INT     10H
        RET

ELSE ; 32-bit version
        USES    ESI, EDI, EBX

        MOV     EDI, DWORD PTR [thisPtr]

        MOV     AX, [EDI+TViewState]
        NOT     AX
        TEST    AX, sfVisible+sfCursorVis+sfFocused
        JNE   @@4
        MOV     EAX, [EDI+TViewCursorY]
        MOV     EDX, [EDI+TViewCursorX]
@@1:
        OR      EAX, EAX
        JL    @@4
        CMP     EAX, [EDI+TViewSizeY]
        JGE   @@4
        OR      EDX, EDX
        JL    @@4
        CMP     EDX, [EDI+TViewSizeX]
        JGE   @@4
        ADD     EAX, [EDI+TViewOriginY]
        ADD     EDX, [EDI+TViewOriginX]
        MOV     ECX, EDI
        MOV     EDI, [EDI+TViewOwner]
        OR      EDI, EDI
        JZ    @@5
        TEST    WORD PTR [EDI+TViewState], sfVisible
        JZ    @@4
        MOV     EDI, [EDI+TGroupLast]
@@2:
        MOV     EDI, [EDI+TViewNext]
        CMP     ECX, EDI
        JNE   @@3
        MOV     EDI, [EDI+TViewOwner]
        JMP   @@1
@@3:
        TEST    WORD PTR [EDI+TViewState], sfVisible
        JZ    @@2
        MOV     ESI, [EDI+TViewOriginY]
        CMP     EAX, ESI
        JL    @@2
        ADD     ESI, [EDI+TViewSizeY]
        CMP     EAX, ESI
        JGE   @@2
        MOV     ESI, [EDI+TViewOriginX]
        CMP     EDX, ESI
        JL    @@2
        ADD     ESI, [EDI+TViewSizeX]
        CMP     EDX, ESI
        JGE   @@2
@@4:
; Cursor is not visible if we get here.
        XOR     EAX, EAX
        JMP   @@6
@@5:
; Cursor is visible, so set it's position.
        CALL    @THardwareInfo@setCaretPosition$qusus, EDX, EAX

; Determine cursor size.

        XOR     EAX, EAX
        MOV     AL, BYTE PTR [LARGE @TScreen@cursorLines] ; Load lo byte only.
        MOV     EDI, DWORD PTR [thisPtr]
        TEST    WORD PTR [EDI+TViewState], sfCursorIns
        JE    @@6
        MOV     AL, 64H
@@6:
        CALL    @THardwareInfo@setCaretSize$qus, EAX
        RET
ENDIF
ENDP

END
