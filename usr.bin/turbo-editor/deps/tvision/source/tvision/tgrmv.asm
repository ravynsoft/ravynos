;/*------------------------------------------------------------*/
;/* filename - tgrmv.asm                                       */
;/*                                                            */
;/* function(s)                                                */
;/*                     TGroup removeView member function      */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 

ifndef __FLAT__
        PUBLIC  @TGroup@removeView$qn5TView
else
        PUBLIC  @TGroup@removeView$qp5TView
endif

        INCLUDE TV.INC

CODESEG

IFNDEF __FLAT__
@TGroup@removeView$qn5TView PROC
ELSE
@TGroup@removeView$qp5TView PROC
ENDIF
        ARG     thisPtr : PTR, P : PTR
IFNDEF __FLAT__
        USES    SI, DI, DS

        LDS     SI, [thisPtr]
        LES     DI, [P]
        LDS     SI, DS:[SI+TGroupLast]
        PUSH    BP
        MOV     AX, DS
        OR      AX, SI
        JE    @@7
        MOV     AX, SI
        MOV     DX, DS
        MOV     BP, ES
@@1:
        MOV     BX, WORD PTR DS:[SI+TViewNext]
        MOV     CX, WORD PTR DS:[SI+TViewNext+2]
        CMP     CX, BP
        JE    @@5
@@2:
        CMP     CX, DX
        JE    @@4
@@3:
        MOV     SI, BX
        MOV     DS, CX
        JMP   @@1
@@4:
        CMP     BX, AX
        JNE   @@3
        JMP   @@7
@@5:
        CMP     BX, DI
        JNE   @@2
        MOV     BX, WORD PTR ES:[DI+TViewNext]
        MOV     CX, WORD PTR ES:[DI+TViewNext+2]
        MOV     WORD PTR DS:[SI+TViewNext], BX
        MOV     WORD PTR DS:[SI+TViewNext+2], CX
        CMP     DX, BP
        JNE   @@7
        CMP     AX, DI
        JNE   @@7
        CMP     CX, BP
        JNE   @@6
        CMP     BX, DI
        JNE   @@6
        XOR     SI, SI
        MOV     DS, SI
@@6:
        POP     BP
        PUSH    BP
        LES     DI, [thisPtr]
        MOV     WORD PTR ES:[DI+TGroupLast], SI
        MOV     WORD PTR ES:[DI+TGroupLast+2], DS
@@7:
        POP     BP
        RET
ELSE        ;;;;;;;;;;;;;;;;;;;;;;;; 32-bit version ;;;;;;;;;;;;;;;;;;;;;;;;;;;
        USES    ESI,EDI,EBX

        MOV     ESI, DWORD PTR [thisPtr]
        MOV     EDI, DWORD PTR [P]
        MOV     ESI, [ESI+TGroupLast]
        OR      ESI, ESI
        JZ    @@7
        MOV     EAX, ESI
@@1:
        MOV     EBX, [ESI+TViewNext]
        JMP   @@5
@@3:
        MOV     ESI, EBX
        JMP   @@1
@@4:
        CMP     EBX, EAX
        JNE   @@3
        JMP   @@7
@@5:
        CMP     EBX, EDI
        JNE   @@4
        MOV     EBX, [EDI+TViewNext]
        MOV     [ESI+TViewNext], EBX
        CMP     EAX, EDI
        JNE   @@7
        CMP     EBX, EDI
        JNE   @@6
        XOR     ESI, ESI
@@6:
        MOV     EDI, DWORD PTR [thisPtr]
        MOV     [EDI+TGroupLast], ESI
@@7:
        RET
ENDIF

ENDP
END
