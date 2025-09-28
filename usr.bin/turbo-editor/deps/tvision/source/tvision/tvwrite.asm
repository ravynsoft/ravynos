;/*------------------------------------------------------------*/
;/* filename -       tvwrit.asm                               */
;/*                                                            */
;/* function(s)                                                */
;/*                  TView write member functions              */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 

ifndef __FLAT__
        PUBLIC  @TView@writeLine$qssssnxv
        PUBLIC  @TView@writeStr$qssnxcuc
        PUBLIC  @TView@writeBuf$qssssnxv
else
        PUBLIC  @TView@writeLine$qsssspxv
        PUBLIC  @TView@writeStr$qsspxcuc
        PUBLIC  @TView@writeBuf$qsssspxv
endif
        PUBLIC  @TView@writeView$qv
        PUBLIC  @TView@writeChar$qsscucs

ifndef __FLAT__
        EXTRN   @THWMouse@show$qv : FAR
        EXTRN   @THWMouse@hide$qv : FAR
        EXTRN   @TView@mapColor$quc : FAR
        EXTRN   @TEventQueue@mouseIntFlag : BYTE
else
        EXTRN   @THWMouse@show$qv : NEAR
        EXTRN   @THWMouse@hide$qv : NEAR
        EXTRN   @TView@mapColor$quc : NEAR
        EXTRN   @THardwareInfo@screenWrite$qususpusul : NEAR
endif
        EXTRN   @TEventQueue@curMouse : WORD
        EXTRN   @TScreen@screenBuffer : DWORD
        EXTRN   @TScreen@checkSnow : BYTE

        EXTRN   _shadowSize : WORD
        EXTRN   _shadowAttr : BYTE

        INCLUDE TV.INC

ifndef __FLAT__
Fptr    STRUC
        offs    DW   ?
        segm    DW   ?
Fptr    ENDS
else
Fptr    STRUC
        offs    DD   ?
Fptr    ENDS
endif


WriteArgs   STRUC
        Self    DD      ?
        Target  Fptr    ?
        Buffer  Fptr    ?
        wOffset DW      ?
WriteArgs   ENDS

DATASEG

wArgs   WriteArgs       ?

IFDEF __FLAT__
        EXTRN _MONOSEG:WORD
        EXTRN _COLRSEG:WORD
ENDIF

CODESEG

; Write to view
; In    AX       = Y coordinate
;       BX       = X coordinate
;       CX       = Count
;       ES:(E)DI = Buffer Pointer

@TView@writeView$qv PROC near
ifndef __FLAT__
        MOV     [wArgs.wOffset], BX
        MOV     [wArgs.Buffer.offs], DI
        MOV     [wArgs.Buffer.segm], ES
        ADD     CX, BX
        XOR     DX, DX
        LES     DI, [wArgs.Self]
        OR      AX, AX
        JL    @@3
        CMP     AX, ES:[DI+TViewSizeY]
        JGE   @@3
        OR      BX, BX
        JGE   @@1
        XOR     BX, BX
@@1:
        CMP     CX, ES:[DI+TViewSizeX]
        JLE   @@2
        MOV     CX, ES:[DI+TViewSizeX]
@@2:
        CMP     BX, CX
        JL    @@10
@@3:
        RETN
@@10:
        TEST    WORD PTR ES:[DI+TViewState], sfVisible
        JE    @@3
        CMP     WORD PTR ES:[DI+TViewOwner+2], 0
        JE    @@3
        MOV     [wArgs.Target.offs], DI
        MOV     [wArgs.Target.segm], ES
        ADD     AX, ES:[DI+TViewOriginY]
        MOV     SI, ES:[DI+TViewOriginX]
        ADD     BX, SI
        ADD     CX, SI
        ADD     [wArgs.wOffset], SI
        LES     DI, ES:[DI+TViewOwner]
        CMP     AX, ES:[DI+TGroupClipAY]
        JL    @@3
        CMP     AX, ES:[DI+TGroupClipBY]
        JGE   @@3
        CMP     BX, ES:[DI+TGroupClipAX]
        JGE   @@11
        MOV     BX, ES:[DI+TGroupClipAX]
@@11:
        CMP     CX, ES:[DI+TGroupClipBX]
        JLE   @@12
        MOV     CX, ES:[DI+TGroupClipBX]
@@12:
        CMP     BX, CX
        JGE   @@3
        LES     DI, ES:[DI+TGroupLast]
@@20:
        LES     DI, ES:[DI+TViewNext]
        CMP     DI, [wArgs.Target.offs]
        JNE   @@21
        MOV     SI, ES
        CMP     SI, [wArgs.Target.segm]
        JNE   @@21
        JMP   @@40
@@21:
        TEST    WORD PTR ES:[DI+TViewState], sfVisible
        JE    @@20
        MOV     SI, ES:[DI+TViewOriginY]
        CMP     AX, SI
        JL    @@20
        ADD     SI, ES:[DI+TViewSizeY]
        CMP     AX, SI
        JL    @@23
        TEST    WORD PTR ES:[DI+TViewState], sfShadow
        JE    @@20
        ADD     SI, [_shadowSize+TPointY]
        CMP     AX, SI
        JGE   @@20
        MOV     SI, ES:[DI+TViewOriginX]
        ADD     SI, [_shadowSize+TPointX]
        CMP     BX, SI
        JGE   @@22
        CMP     CX, SI
        JLE   @@20
        CALL  @@30
@@22:
        ADD     SI, ES:[DI+TViewSizeX]
        JMP   @@26
@@23:
        MOV     SI, ES:[DI+TViewOriginX]
        CMP     BX, SI
        JGE   @@24
        CMP     CX, SI
        JLE   @@20
        CALL  @@30
@@24:
        ADD     SI, ES:[DI+TViewSizeX]
        CMP     BX, SI
        JGE   @@25
        CMP     CX, SI
        JLE   @@31
        MOV     BX, SI
@@25:
        TEST    WORD PTR ES:[DI+TViewState], sfShadow
        JE    @@20
        PUSH    SI
        MOV     SI, ES:[DI+TViewOriginY]
        ADD     SI, [_shadowSize+TPointY]
        CMP     AX, SI
        POP     SI
        JL    @@27
        ADD     SI, [_shadowSize+TPointX]
@@26:
        CMP     BX, SI
        JGE   @@27
        INC     DX
        CMP     CX, SI
        JLE   @@27
        CALL  @@30
        DEC     DX
@@27:
        JMP   @@20
@@30:
        PUSH    [wArgs.Target.segm]
        PUSH    [wArgs.Target.offs]
        PUSH    [wArgs.wOffset]
        PUSH    ES
        PUSH    DI
        PUSH    SI
        PUSH    DX
        PUSH    CX
        PUSH    AX
        MOV     CX, SI
        CALL  @@20
        POP     AX
        POP     CX
        POP     DX
        POP     SI
        POP     DI
        POP     ES
        POP     [wArgs.wOffset]
        POP     [wArgs.Target.offs]
        POP     [wArgs.Target.segm]
        MOV     BX, SI
@@31:
        RETN
@@40:
        LES     DI, ES:[DI+TViewOwner]
        MOV     SI, ES:[DI+TGroupBuffer+2]
        OR      SI, SI
        JE    @@44
        CMP     SI, WORD PTR [@TScreen@screenBuffer+2]
        JE    @@41
        CALL  @@50
        JMP   @@44
@@41:
        CLI
        CMP     AX, WORD PTR [@TEventQueue@curMouse+MsEventWhereY]
        JNE   @@42
        CMP     BX, WORD PTR [@TEventQueue@curMouse+MsEventWhereX]
        JA    @@42
        CMP     CX, WORD PTR [@TEventQueue@curMouse+MsEventWhereX]
        JA    @@43
@@42:
        MOV     [@TEventQueue@mouseIntFlag], 0
        STI
        CALL  @@50
        CMP     [@TEventQueue@mouseIntFlag], 0
        JE    @@44
@@43:
        STI
        CALL    @THWMouse@hide$qv
        CALL  @@50
        CALL    @THWMouse@show$qv
@@44:
        CMP     BYTE PTR ES:[DI+TGroupLockFlag], 0
        JNE   @@31
        JMP   @@10
@@50:
        PUSH    ES
        PUSH    DS
        PUSH    DI
        PUSH    CX
        PUSH    AX
        MUL     BYTE PTR ES:[DI+TViewSizeX]
        ADD     AX, BX
        SHL     AX, 1
        ADD     AX, ES:[DI+TGroupBuffer]
        MOV     DI, AX
        MOV     ES, SI
        XOR     AL, AL
        CMP     SI, WORD PTR [@TScreen@screenBuffer+2]
        JNE   @@51
        MOV     AL, [@TScreen@checkSnow]
@@51:
        MOV     AH, [_shadowAttr]
        SUB     CX, BX
        MOV     SI, BX
        SUB     SI, [wArgs.wOffset]
        SHL     SI, 1
        ADD     SI, [wArgs.Buffer.offs]
        MOV     DS, [wArgs.Buffer.segm]
        CLD
        OR      AL, AL
        JNE   @@60
        OR      DX, DX
        JNE   @@52
        REP     MOVSW
        JMP   @@70
@@52:
        LODSB
        INC     SI
        STOSW
        LOOP  @@52
        JMP   @@70
@@60:   PUSH    DX
        PUSH    BX
        OR      DX, DX
        MOV     DX, 03DAH
        JNE   @@65
@@61:   LODSW
        MOV     BX, AX
@@62:   IN      AL, DX
        TEST    AL, 1
        JNE   @@62
        CLI
@@63:   IN      AL, DX
        TEST    AL, 1
        JE    @@63
        MOV     AX, BX
        STOSW
        STI
        LOOP  @@61
        JMP   @@68
@@65:   LODSB
        MOV     BL, AL
        INC     SI
@@66:   IN      AL, DX
        TEST    AL, 1
        JNE   @@66
        CLI
@@67:   IN      AL, DX
        TEST    AL, 1
        JE    @@67
        MOV     AL, BL
        STOSW
        STI
        LOOP  @@65
@@68:   POP     BX
        POP     DX
@@70:
        MOV     SI, ES
        POP     AX
        POP     CX
        POP     DI
        POP     DS
        POP     ES
        RETN

else        ;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit version ;;;;;;;;;;;;;;;;;;;;;;;;;;;

        MOV     [wArgs.wOffset], BX
        MOV     [wArgs.Buffer.offs], EDI
        ADD     CX, BX
        XOR     DX, DX

        MOV     EDI, [wArgs.Self]
        OR      AX, AX
        JL    @@3
        CMP     AX, [EDI+TViewSizeY]
        JGE   @@3
        OR      BX, BX
        JGE   @@1
        XOR     BX, BX
@@1:
        CMP     CX, [EDI+TViewSizeX]
        JLE   @@2
        MOV     CX, [EDI+TViewSizeX]
@@2:
        CMP     BX, CX
        JL    @@10
@@3:
        RETN
@@10:
        TEST    WORD PTR [EDI+TViewState], sfVisible
        JE    @@3
        CMP     DWORD PTR [EDI+TViewOwner], 0
        JE    @@3
        MOV     [wArgs.Target.offs], EDI
        ADD     AX, [EDI+TViewOriginY]
        MOV     SI, [EDI+TViewOriginX]
        ADD     BX, SI
        ADD     CX, SI
        ADD     [wArgs.wOffset], SI
        MOV     EDI, [EDI+TViewOwner]
        CMP     AX, [EDI+TGroupClipAY]
        JL    @@3
        CMP     AX, [EDI+TGroupClipBY]
        JGE   @@3
        CMP     BX, [EDI+TGroupClipAX]
        JGE   @@11
        MOV     BX, [EDI+TGroupClipAX]
@@11:
        CMP     CX, [EDI+TGroupClipBX]
        JLE   @@12
        MOV     CX, [EDI+TGroupClipBX]
@@12:
        CMP     BX, CX
        JGE   @@3
        MOV     EDI, [EDI+TGroupLast]
@@20:
        MOV     EDI, [EDI+TViewNext]
        CMP     EDI, [wArgs.Target.offs]
        JE    @@40
@@21:
        TEST    WORD PTR [EDI+TViewState], sfVisible
        JE    @@20
        MOV     SI, [EDI+TViewOriginY]
        CMP     AX, SI
        JL    @@20
        ADD     SI, [EDI+TViewSizeY]
        CMP     AX, SI
        JL    @@23
        TEST    WORD PTR [EDI+TViewState], sfShadow
        JE    @@20
        ADD     SI, [LARGE _shadowSize+TPointY]
        CMP     AX, SI
        JGE   @@20
        MOV     SI, [EDI+TViewOriginX]
        ADD     SI, [LARGE _shadowSize+TPointX]
        CMP     BX, SI
        JGE   @@22
        CMP     CX, SI
        JLE   @@20
        CALL  @@30
@@22:
        ADD     SI, [EDI+TViewSizeX]
        JMP   @@26
@@23:
        MOV     SI, [EDI+TViewOriginX]
        CMP     BX, SI
        JGE   @@24
        CMP     CX, SI
        JLE   @@20
        CALL  @@30
@@24:
        ADD     SI, [EDI+TViewSizeX]
        CMP     BX, SI
        JGE   @@25
        CMP     CX, SI
        JLE   @@31
        MOV     BX, SI
@@25:
        TEST    WORD PTR [EDI+TViewState], sfShadow
        JE    @@20
        PUSH    SI
        MOV     SI, [EDI+TViewOriginY]
        ADD     SI, [LARGE _shadowSize+TPointY]
        CMP     AX, SI
        POP     SI
        JL    @@27
        ADD     SI, [LARGE _shadowSize+TPointX]
@@26:
        CMP     BX, SI
        JGE   @@27
        INC     DX
        CMP     CX, SI
        JLE   @@27
        CALL  @@30
        DEC     DX
@@27:
        JMP   @@20
@@30:
        PUSH    [wArgs.Target.offs]
        PUSH    DWORD PTR [wArgs.wOffset]
        PUSH    EDI
        PUSH    ESI
        PUSH    EDX
        PUSH    ECX
        PUSH    EAX
        MOV     CX, SI
        CALL  @@20
        POP     EAX
        POP     ECX
        POP     EDX
        POP     ESI
        POP     EDI
        POP     DWORD PTR [wArgs.wOffset]
        POP     [wArgs.Target.offs]
        MOV     BX, SI
@@31:
        RETN
@@40:
        MOV     EDI, [EDI+TViewOwner]
        MOV     ESI, [EDI+TGroupBuffer]
        OR      ESI, ESI
        JE    @@44

        CMP     ESI, DWORD PTR [@TScreen@screenBuffer]
        JE    @@41
        CALL  @@50
        JMP   @@44

@@41:
IFDEF HIDEMOUSE
        PUSHAD
        CALL    @THWMouse@hide$qv
        POPAD
ENDIF

        CALL  @@50

IFDEF HIDEMOUSE
        PUSHAD
        CALL    @THWMouse@show$qv
        POPAD
ENDIF

@@44:
        CMP     BYTE PTR [EDI+TGroupLockFlag], 0
        JNE   @@31
        JMP   @@10
@@50:
        PUSH    ESI
        PUSH    EDI
        PUSH    ECX
        PUSH    EAX

        PUSH    EAX
        MUL     BYTE PTR [EDI+TViewSizeX]
        ADD     AX, BX
        SHL     AX, 1
        MOVZX   EDI, AX
        ADD     EDI, ESI                ; ESI = [EDI+TGroupBuffer]
        CMP     ESI, DWORD PTR [@TScreen@screenBuffer]
        POP     EAX
        JNE   @@60

        SUB     CX, BX
        MOV     SI, BX
        SUB     SI, [wArgs.wOffset]
        SHL     SI, 1
        MOVZX   ESI, SI
        ADD     ESI, [wArgs.Buffer.offs]
        CLD
        PUSH    EAX
        PUSH    ECX
        PUSH    EDI
        XOR     AH, AH
        OR      DX, DX
        JNE   @@52

;Expand character/attribute pair
@@51:
        LODSB
        STOSW
        LODSB
        STOSW
        LOOP  @@51

        JMP   @@54

;Mix in shadow attribute
@@52:
        PUSH    EDX
        MOV     DL,  [_shadowAttr]
@@53:
        LODSB
        STOSW
        MOV     AL, DL
        INC     ESI
        STOSW
        LOOP  @@53
        POP     EDX

@@54:
        POP     EDI
        POP     ECX
        POP     EAX
        PUSHAD
        CALL    @THardwareInfo@screenWrite$qususpusul, EBX, EAX, EDI, ECX
        POPAD
        JMP   @@70

@@60:
        MOV     AH, [LARGE _shadowAttr]
        SUB     CX, BX
        MOV     SI, BX
        SUB     SI, [wArgs.wOffset]
        SHL     SI, 1
        MOVZX   ESI, SI
        ADD     ESI, [wArgs.Buffer.offs]
        CLD
        OR      DX, DX
        JNE   @@61
        MOV     EDX, ECX
        SHR     ECX, 1
        REP     MOVSD
        MOV     ECX, EDX
        AND     ECX, 01H
        REP     MOVSW
        XOR     EDX, EDX
        JMP   @@70
@@61:
        LODSB
        INC     ESI
        STOSW
        LOOP  @@61

@@70:
        POP     EAX
        POP     ECX
        POP     EDI
        POP     ESI
        RETN
endif
ENDP

ifndef __FLAT__
@TView@writeBuf$qssssnxv PROC
else
@TView@writeBuf$qsssspxv PROC
endif
        ARG     thisPtr:PTR, X:ARGINT, Y:ARGINT, W:ARGINT, H:ARGINT, Buf:PTR
ifndef __FLAT__
        USES    SI, DI

        MOV     AX, WORD PTR [thisPtr]
        MOV     WORD PTR [wArgs.Self], AX
        MOV     AX, WORD PTR [thisPtr+2]
        MOV     [(WORD PTR wArgs.Self)+2], AX

        CMP     [H], 0
        JLE   @@2
@@1:
        MOV     AX, [Y]
        MOV     BX, [X]
        MOV     CX, [W]
        LES     DI, [Buf]
        CALL    @TView@writeView$qv
        MOV     AX, [W]
        SHL     AX, 1
        ADD     WORD PTR [Buf], AX
        INC     [Y]
        DEC     [H]
        JNE   @@1
@@2:
        RET

else        ;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit version ;;;;;;;;;;;;;;;;;;;;;;;;;;;

        USES    EBX, ESI, EDI

        XOR     EAX,  EAX
        MOV     WORD PTR [X+2],  AX
        MOV     WORD PTR [Y+2],  AX
        MOV     WORD PTR [W+2],  AX
        MOV     WORD PTR [H+2],  AX

        MOV     EAX, DWORD PTR [thisPtr]
        MOV     [wArgs.Self], EAX

        CMP     [H], 0
        JLE   @@2
@@1:
        MOV     EAX, [Y]
        MOV     EBX, [X]
        MOV     ECX, [W]
        MOV     EDI, DWORD PTR [Buf]
@@4:
        CALL    @TView@writeView$qv
        MOV     EAX, [W]
        SHL     EAX, 1
        ADD     DWORD PTR [Buf], EAX
        INC     [Y]
        DEC     [H]
        JNE   @@1
@@2:
        RET
endif

ENDP

@TView@writeChar$qsscucs PROC
        ARG     thisPtr:PTR,  X:ARGINT,  Y:ARGINT,  C:ARGINT,  Color:ARGINT,  \
                Count : ARGINT
ifndef __FLAT__
        USES    SI, DI

        MOV     AX, WORD PTR [thisPtr]
        MOV     WORD PTR [wArgs.Self], AX
        MOV     AX, WORD PTR [thisPtr+2]
        MOV     [(WORD PTR wArgs.Self)+2], AX

        PUSH    WORD PTR [Color]
        PUSH    WORD PTR [thisPtr+2]
        PUSH    WORD PTR [thisPtr]
        CALL    @TView@mapColor$quc
        ADD             SP, 6
        MOV     AH, AL
        MOV     AL, BYTE PTR [ C]
        MOV     CX, [Count]
        OR      CX, CX
        JLE   @@2
        CMP     CX, 256
        JLE   @@1
        MOV     CX, 256
@@1:
        MOV     DI, CX
        SHL     DI, 1
        SUB     SP, DI
        PUSH    DI
        MOV     DI, SP
        ADD     DI, 2
        PUSH    SS
        POP     ES
        MOV     DX, CX
        CLD
        REP     STOSW
        MOV     CX, DX
        MOV     DI, SP
        ADD     DI, 2
        MOV     AX, [Y]
        MOV     BX, [X]
        CALL    @TView@writeView$qv
        POP     DI
        ADD     SP, DI
@@2:
        RET

else        ;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit version ;;;;;;;;;;;;;;;;;;;;;;;;;;;

        USES    EBX, ESI, EDI

        XOR     EAX,  EAX
        MOV     WORD PTR [X+2],  AX
        MOV     WORD PTR [Y+2],  AX
        MOV     WORD PTR [C+2],  AX
        MOV     WORD PTR [Color+2],  AX
        MOV     WORD PTR [Count+2],  AX

        MOV     EAX, DWORD PTR [thisPtr]
        MOV     [wArgs.Self], EAX

        CALL    @TView@mapColor$quc, [thisPtr], [Color]
        MOV     AH, AL
        MOV     AL, BYTE PTR [C]
        PUSH    AX
        PUSH    AX
        POP     EAX
        MOV     ECX, [Count]
        OR      ECX, ECX
        JLE   @@2
        CMP     ECX, 256
        JLE   @@1
        MOV     ECX, 256
@@1:
        MOV     EDI, ECX
        SHL     EDI, 1
        SUB     ESP, EDI
        PUSH    EDI
        MOV     EDI, ESP
        ADD     EDI, 4
        MOV     EDX, ECX
        CLD
        SHR     ECX, 1
        REP     STOSD
        MOV     ECX, EDX
        AND     ECX, 01H
        REP     STOSW
        MOV     ECX, EDX
        MOV     EDI, ESP
        ADD     EDI, 4
        MOV     EAX, [Y]
        MOV     EBX, [X]
        CALL    @TView@writeView$qv
        POP     EDI
        ADD     ESP, EDI
@@2:
        RET
endif
ENDP

ifndef __FLAT__
@TView@writeLine$qssssnxv PROC
else
@TView@writeLine$qsssspxv PROC
endif
        ARG     thisPtr:PTR,  X:ARGINT,  Y:ARGINT,  W:ARGINT,  H:ARGINT,  \
                Buf : PTR
ifndef __FLAT__
        USES    SI, DI

        MOV     AX, WORD PTR [thisPtr]
        MOV     WORD PTR [wArgs.Self], AX
        MOV     AX, WORD PTR [thisPtr+2]
        MOV     [(WORD PTR wArgs.Self)+2], AX

        CMP     [H], 0
        JLE   @@2
@@1:
        MOV     AX, [Y]
        MOV     BX, [X]
        MOV     CX, [W]
        LES     DI, [Buf]
        CALL    @TView@writeView$qv
        INC     [Y]
        DEC     [H]
        JNE   @@1
@@2:
        RET

else        ;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit version ;;;;;;;;;;;;;;;;;;;;;;;;;;;

        USES    EBX, ESI, EDI

        XOR     EAX,  EAX
        MOV     WORD PTR [X+2],  AX
        MOV     WORD PTR [Y+2],  AX
        MOV     WORD PTR [W+2],  AX
        MOV     WORD PTR [H+2],  AX

        MOV     EAX, DWORD PTR [thisPtr]
        MOV     [wArgs.Self], EAX

        CMP     [H], 0
        JLE   @@2
@@1:
        MOV     EAX, [Y]
        MOV     EBX, [X]
        MOV     ECX, [W]
        MOV     EDI, DWORD PTR [Buf]
        CALL    @TView@writeView$qv
        INC     [Y]
        DEC     [H]
        JNE   @@1
@@2:
        RET

endif
ENDP

ifndef __FLAT__
@TView@writeStr$qssnxcuc PROC
else
@TView@writeStr$qsspxcuc PROC
endif
        ARG     thisPtr:PTR,  X:ARGINT,  Y:ARGINT,  Strng:PTR,  Color:ARGINT
        LOCAL   ssize : ARGINT

ifndef __FLAT__
        USES    SI, DI

        MOV     AX, WORD PTR [thisPtr]
        MOV     WORD PTR [wArgs.Self], AX
        MOV     AX, WORD PTR [thisPtr+2]
        MOV     [(WORD PTR wArgs.Self)+2], AX

        MOV     DI, WORD PTR [Strng]
        OR      DI, WORD PTR [Strng+2]
        JZ    @@2
        LES     DI, [Strng]
        XOR     AX, AX

        CLD
        MOV     CX, 0FFFFh
        REPNE   SCASB
        XCHG    AX, CX
        NOT     AX
        DEC     AX
        CMP     AX, 0
        JE    @@2             ; don't write zero length string
        MOV     SI, AX           ; save char count
        SHL     AX, 1
        SUB     SP, AX           ; make room for attributed string
        MOV     [ssize], AX

        PUSH    WORD PTR [Color]
        PUSH    WORD PTR [thisPtr+2]
        PUSH    WORD PTR [thisPtr]
        CALL    @TView@mapColor$quc
        ADD     SP, 6
        MOV     AH, AL           ; attribute into AH
        MOV     CX, SI           ; char count into CX

        MOV     BX, DS
        LDS     SI, [Strng]
        MOV     DI, SP
        PUSH    SS
        POP     ES
        MOV     DX, CX
@@1:
        LODSB
        STOSW
        LOOP  @@1
        MOV     DS, BX
        MOV     CX, DX
        MOV     DI, SP
        MOV     AX, [Y]
        MOV     BX, [X]
        CALL    @TView@writeView$qv
        ADD     SP, [ssize]
        JMP   @@2
@@3:
        MOV     DS, BX
@@2:
        RET

else        ;;;;;;;;;;;;;;;;;;;;;;;;;; 32-bit version ;;;;;;;;;;;;;;;;;;;;;;;;;;;

        USES    EBX, ESI, EDI

        XOR     EAX,  EAX
        MOV     WORD PTR [X+2],  AX
        MOV     WORD PTR [Y+2],  AX
        MOV     WORD PTR [Color+2],  AX
        MOV     WORD PTR [ssize+2],  AX

        MOV     EAX, DWORD PTR [thisPtr]
        MOV     DWORD PTR[ wArgs.Self], EAX

        MOV     EDI, DWORD PTR[ Strng]
        OR      EDI, EDI
        JZ    @@2

        XOR     EAX, EAX

        CLD
        MOV     ECX, -1
        REPNE   SCASB
        XCHG    EAX, ECX
        NOT     EAX
        DEC     EAX
        OR      EAX, EAX
        JZ    @@2                   ; don't write zero length string
        MOV     ESI, EAX            ; save char count
        INC     EAX
        SHR     EAX, 1
        SHL     EAX, 2
        SUB     ESP, EAX            ; make room for attributed string
        MOV     [ssize], EAX

        CALL    @TView@mapColor$quc, [thisPtr], [Color]
        MOV     AH, AL              ; attribute into AH
        MOV     ECX, ESI            ; char count into CX

        MOV     ESI, DWORD PTR [Strng]
        MOV     EDI, ESP
        MOV     EDX, ECX
@@1:
        LODSB
        STOSW
        LOOP  @@1

        MOV     ECX, EDX
        MOV     EDI, ESP
        MOV     EAX, [Y]
        MOV     EBX, [X]
        CALL    @TView@writeView$qv
        ADD     ESP, [ssize]
@@2:
        RET
endif
ENDP

        END
