;/*------------------------------------------------------------*/
;/* filename -       hardware.cpp                              */
;/*                                                            */
;/* function(s)                                                */
;/*                  THardwareInfo member functions and        */
;/*                  variables.                                */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 

        INCLUDE TV.INC


IFNDEF __FLAT__
        PUBLIC  @THardwareInfo@$bctr$qv
        PUBLIC  @THardwareInfo@$bdtr$qv
        PUBLIC  @THardwareInfo@getBiosEquipmentFlag$qi
        PUBLIC  @THardwareInfo@getBiosSelector$qv

        EXTRN   @THardwareInfo@dpmiFlag : BYTE
        EXTRN   @THardwareInfo@colorSel : WORD
        EXTRN   @THardwareInfo@monoSel : WORD
        EXTRN   @THardwareInfo@biosSel : WORD
ENDIF

        CODESEG
        ASSUME DS:DGROUP

; THardwareInfo non-inline functions

IFNDEF __FLAT__

@THardwareInfo@$bctr$qv  PROC    FAR

; Are we running in protected mode?
        MOV     AX, 352FH   ; Check for a null INT 2F handler first
        INT     21H         ; just in case.
        MOV     AX, ES
        OR      AX, BX
        JZ    @@nodpmi

        MOV     AX, 0FB42H
        MOV     BX, 01H
        INT     2FH
        CMP     AX, 01H
        JNE   @@nodpmi

; Yes, in protected mode, thus we need to allocate selectors...
        MOV     [@THardwareInfo@dpmiFlag], 01H

        MOV     AX, 02H
        MOV     BX, 0040H
        INT     31H
        MOV     [@THardwareInfo@biosSel], AX

        MOV     AX, 02H
        MOV     BX, 0B000H
        INT     31H
        MOV     [@THardwareInfo@monoSel], AX

        MOV     AX, 02H
        MOV     BX, 0B800H
        INT     31H
        MOV     [@THardwareInfo@colorSel], AX

        RET

@@nodpmi:
        MOV     [@THardwareInfo@dpmiFlag], 00H
        MOV     [@THardwareInfo@biosSel], 00040H
        MOV     [@THardwareInfo@monoSel], 0B000H
        MOV     [@THardwareInfo@colorSel], 0B800H

        RET
@THardwareInfo@$bctr$qv  ENDP

@THardwareInfo@$bdtr$qv  PROC    FAR
        RET
@THardwareInfo@$bdtr$qv  ENDP

@THardwareInfo@getBiosEquipmentFlag$qi   PROC FAR
        PUSH    DS
        MOV     AX, SEG DGROUP
        MOV     DS, AX

        MOV     BX, 10H
        MOV     ES, WORD PTR DGROUP:[@THardwareInfo@biosSel]
        MOV     AX, ES:[BX]

        POP     DS
        RET
@THardwareInfo@getBiosEquipmentFlag$qi   ENDP

@THardwareInfo@getBiosSelector$qv    PROC FAR
        PUSH    DS
        MOV     AX, SEG DGROUP
        MOV     DS, AX
        MOV     AX, WORD PTR DGROUP:[@THardwareInfo@biosSel]
        POP     DS
        RET
@THardwareInfo@getBiosSelector$qv    ENDP

ENDIF

        END
