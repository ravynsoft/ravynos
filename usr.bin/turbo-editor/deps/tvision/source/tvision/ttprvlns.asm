;/*------------------------------------------------------------*/
;/* filename -       ttprvlns.asm                              */
;/*                                                            */
;/* function(s)                                                */
;/*                  TTerminal prevLines member function       */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 

    PUBLIC  @TTerminal@prevLines$qusus
    INCLUDE TV.INC

    CODESEG

DecDI    PROC
ifndef __FLAT__
        CMP     DI,WORD PTR [SI+TTerminalBuffer]
        JA      short @@1
        ADD     DI,WORD PTR [SI+TTerminalBufSize]
@@1:    DEC     DI
else
        CMP     EDI,[ESI+TTerminalBuffer]
        JA      short @@1
        MOVZX   EAX, WORD PTR [ESI+TTerminalBufSize]
        ADD     EDI, EAX
@@1:    DEC     EDI
endif
        RET
ENDP


IncDI    PROC
ifndef __FLAT__
        INC     DI
        MOV     AX,WORD PTR [SI+TTerminalBuffer]
        ADD     AX,[SI+TTerminalBufSize]
        CMP     DI,AX
        JB      short @@1
        MOV     DI,WORD PTR [SI+TTerminalBuffer]
@@1:
        RET
else
        INC     EDI
        MOVZX   EAX,WORD PTR [ESI+TTerminalBufSize]
        ADD     EAX,[ESI+TTerminalBuffer]
        CMP     EDI,EAX
        JB      short @@1
        MOV     EDI,[ESI+TTerminalBuffer]
@@1:
        RET
endif
ENDP


@TTerminal@prevLines$qusus PROC
        ARG     thisPtr :PTR, Pos : WORD, Lines : WORD
LineSeparator   EQU 10

ifndef __FLAT__
        USES    DS,SI,DI

        LDS     SI,[thisPtr]
        LES     DI,[SI+TTerminalBuffer]
        ADD     DI,[Pos]
@@1:    MOV     CX,[Lines]
        JCXZ    @@6
        MOV     AX,[SI+TTerminalQueBack]
        ADD     AX,WORD PTR [SI+TTerminalBuffer]
        CMP     DI,AX
        JE      @@7
        CALL    DecDI
@@2:    MOV     AX,[SI+TTerminalQueBack]
        ADD     AX,WORD PTR [SI+TTerminalBuffer]
        CMP     DI,AX
        JAE     @@3
        MOV     CX,DI
        SUB     CX,WORD PTR [SI+TTerminalBuffer]
        JMP     @@4
@@3:    MOV     CX,DI
        SUB     CX,AX
@@4:    MOV     AL,LineSeparator
        INC     CX
        STD
        REPNE   SCASB
        JE      @@5
        MOV     AX,DI
        SUB     AX,WORD PTR [SI+TTerminalBuffer]
        INC     AX
        CMP     AX,[SI+TTerminalQueBack]
        JE      @@8
        MOV     DI,WORD PTR [SI+TTerminalBuffer]
        ADD     DI,WORD PTR [SI+TTerminalBufSize]
        DEC     DI
        JMP     @@2
@@5:    DEC     [Lines]
        JNZ     @@2
@@6:    CALL    IncDI
        CALL    IncDI
        MOV     AX,DI
@@7:    SUB     AX,WORD PTR [SI+TTerminalBuffer]
@@8:
        CLD
        RET
else
        USES    ESI,EDI

        MOV     ESI, DWORD PTR [thisPtr]
        MOV     EDI,[ESI+TTerminalBuffer]
        ADD     DI,[Pos]
@@1:    XOR     ECX,ECX
        MOV     CX,[Lines]
        JECXZ   short @@6
        MOVZX   EAX, WORD PTR [ESI+TTerminalQueBack]
        ADD     EAX,[ESI+TTerminalBuffer]
        CMP     EDI,EAX
        JE      short @@7
        CALL    DecDI
@@2:    MOVZX   EAX, WORD PTR [ESI+TTerminalQueBack]
        ADD     EAX,[ESI+TTerminalBuffer]
        CMP     EDI,EAX
        JAE     short @@3
        MOV     ECX,EDI
        SUB     ECX,[ESI+TTerminalBuffer]
        JMP     short @@4
@@3:    MOV     ECX,EDI
        SUB     ECX,EAX
@@4:    MOV     AL,LineSeparator
        INC     ECX
        STD
        REPNE   SCASB
        JE      short @@5
        MOV     EAX,EDI
        SUB     EAX,[ESI+TTerminalBuffer]
        INC     EAX
        CMP     AX, WORD PTR [ESI+TTerminalQueBack]
        JE      @@8
        MOVZX   EDI,WORD PTR [ESI+TTerminalBufSize]
        ADD     EDI,[ESI+TTerminalBuffer]
        DEC     EDI
        JMP     @@2
@@5:    DEC     [Lines]
        JNZ     @@2
@@6:    CALL    IncDI
        CALL    IncDI
        MOV     EAX,EDI
@@7:    SUB     EAX,[ESI+TTerminalBuffer]
@@8:
        CLD
        RET
endif
        ENDP
        END
