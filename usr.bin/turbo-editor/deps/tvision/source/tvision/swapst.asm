;/*------------------------------------------------------------*/
;/* filename -           swapst.asm                            */
;/*                                                            */
;/* function(s)                                                */
;/*                      TSystemError swapStatusLine function  */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 


IFNDEF __FLAT__
        PUBLIC  @TSystemError@swapStatusLine$qm11TDrawBuffer
ELSE
        PUBLIC  @TSystemError@swapStatusLine$qr11TDrawBuffer
ENDIF
        EXTRN   @TScreen@screenWidth :  BYTE
        EXTRN   @TScreen@screenHeight : BYTE


IFNDEF __FLAT__
        EXTRN   @TScreen@screenBuffer : FAR PTR
ELSE
        EXTRN   @TScreen@screenBuffer : FWORD
ENDIF


        INCLUDE TV.INC
CODESEG

IFNDEF __FLAT__
@TSystemError@swapStatusLine$qm11TDrawBuffer PROC
ELSE
@TSystemError@swapStatusLine$qr11TDrawBuffer PROC
ENDIF
        ARG     Buffer : PTR
IFNDEF __FLAT__
        USES    SI, DI

        MOV     CL, BYTE PTR [@TScreen@screenWidth]
        XOR     CH, CH
        MOV     AL, [@TScreen@screenHeight]
        DEC     AL
        MUL     CL
        SHL     AX, 1
        LES     DI, [@TScreen@screenBuffer]
        ADD     DI, AX
        PUSH    DS
        LDS     SI, [Buffer]
@@1:
        MOV     AX, ES:[DI]
        MOVSW
        MOV     DS:[SI-2], AX
        LOOP  @@1
        POP     DS
        RET
ELSE         ;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        USES ESI, EDI

        MOVZX   ECX, BYTE PTR  [LARGE @TScreen@screenWidth]
        MOV     AL, BYTE PTR [LARGE @TScreen@screenHeight]
        DEC     AL
        MUL     CL
        MOVZX   EAX, AX
        SHL     EAX, 1
        LES     EDI, [LARGE @TScreen@screenBuffer]
        ADD     EDI, EAX
        MOV     ESI, DWORD PTR [Buffer]
@@1:
        MOV     AX, ES:[EDI]
        MOVSW
        MOV     [ESI-2], AX
        LOOP  @@1
        RET
ENDIF

ENDP
END
