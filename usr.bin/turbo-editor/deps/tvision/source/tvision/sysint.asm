;/*------------------------------------------------------------*/
;/* filename -       sysint.asm                                */
;/*                                                            */
;/* function(s)                                                */
;/*                  TSystemError member function              */
;/*------------------------------------------------------------*/

;
;       Turbo Vision - Version 2.0
; 
;       Copyright (c) 1994 by Borland International
;       All Rights Reserved.
; 

        TITLE   SYSINT

IFNDEF __FLAT__

        PUBLIC  @TSystemError@suspend$qv
        PUBLIC  @TSystemError@resume$qv
        PUBLIC  @TSystemError@Int24PMThunk$qv
        PUBLIC  @TSystemError@setupDPMI$qv
        PUBLIC  @TSystemError@shutdownDPMI$qv

        EXTRN   _AuxPrintR : FAR

        EXTRN   @TSystemError@Int24Regs : INT_REGS
        EXTRN   @TSystemError@Int24RMThunk : CODEPTR
        EXTRN   @TSystemError@Int24RMThunkSel : WORD
        EXTRN   @TSystemError@Int24RMCallback : CODEPTR

        EXTRN   @TSystemError@ctrlBreakHit : BYTE
        EXTRN   @TSystemError@saveCtrlBreak : BYTE
        EXTRN   @TSystemError@sysErrorFunc : CODEPTR
        EXTRN   @TSystemError@inIDE : BYTE

        EXTRN   @THardwareInfo@getBiosEquipmentFlag$qi : FAR
        EXTRN   @THardwareInfo@getBiosSelector$qv : FAR

        EXTRN   @THardwareInfo@dpmiFlag : BYTE

ENDIF

        INCLUDE TV.INC
;        JUMPS

IFNDEF __FLAT__

; Keyboard scan codes

scSpaceKey      EQU     39H
scInsKey        EQU     52H
scDelKey        EQU     53H

; Keyboard shift flags

kbShiftKey      EQU     03H
kbCtrlKey       EQU     04H
kbAltKey        EQU     08H

; ROM BIOS workspace

KeyFlags        EQU     (BYTE PTR 17H)
KeyBufHead      EQU     (WORD PTR 1AH)
KeyBufTail      EQU     (WORD PTR 1CH)
KeyBufOrgPtr    EQU     (WORD PTR 80H)
KeyBufEndPtr    EQU     (WORD PTR 82H)

; DOS function call classes

cNothing        EQU     0       ;No check needed
cName           EQU     2       ;Check name at DS:DX
cHandle         EQU     4       ;Check handle in BX
cDrive          EQU     6       ;Check drive in DL

ENDIF

; Data segment

DATASEG

; Externals

IFNDEF __FLAT__

; Structure definition for calling DPMI function 0300.

INT_REGS STRUC
        _di     dd      ?
        _si     dd      ?
        _bp     dd      ?
                dd      ?
        _bx     dd      ?
        _dx     dd      ?
        _cx     dd      ?
        _ax     dd      ?
        _flags  dw      ?
        _es     dw      ?
        _ds     dw      ?
        _fs     dw      ?
        _gs     dw      ?
        _ip     dw      ?
        _cs     dw      ?
        _sp     dw      ?
        _ss     dw      ?
INT_REGS ENDS

SaveInt09       DD      ?       ;Saved INT 09H vector
SaveInt1B       DD      ?       ;Saved INT 1BH vector
SaveInt21       DD      ?       ;Saved INT 21H vector
SaveInt23       DD      ?       ;Saved INT 23H vector
SaveInt24       DD      ?       ;Saved INT 24H vector
SaveInt24R      DD      ?       ;Saved INT 24H realmode vector for DPMI16.
SaveInt10       DD      ?       ;Saved INT 10H vector

critFlag        DW      ?       ;Critical error code (FF = no error)
critDrive       DW      ?       ;Drive on which critical error occured.

GInt21Stack     DB 0400H DUP (0FFH)

OldSS           DW      ?
OldSP           DW      ?
NewSS           DW      SEG GInt21Stack
NewSP           DW      OFFSET GInt21Stack+03FEH
SavedFlags      DW      ?
InGInt21        DB      0

ENDIF

; Code segment

CODESEG

IFNDEF __FLAT__
; Keyboard conversion table

KeyConvertTab   LABEL   BYTE

        DB      scSpaceKey,kbAltKey
        DW      0200H
        DB      scInsKey,kbCtrlKey
        DW      0400H
        DB      scInsKey,kbShiftKey
        DW      0500H
        DB      scDelKey,kbCtrlKey
        DW      0600H
        DB      scDelKey,kbShiftKey
        DW      0700H

KeyConvertCnt   EQU     ($-KeyConvertTab)/4

; DOS function call class table

FuncClassTab    LABEL   BYTE

        DB      cDrive          ;36H - Get disk free space
        DB      cNothing
        DB      cNothing
        DB      cName           ;39H - Make directory
        DB      cName           ;3AH - Remove directory
        DB      cName           ;3BH - Change directory
        DB      cName           ;3CH - Create file
        DB      cName           ;3DH - Open file
        DB      cHandle         ;3EH - Close file
        DB      cHandle         ;3FH - Read file
        DB      cHandle         ;40H - Write file
        DB      cName           ;41H - Delete file
        DB      cHandle         ;42H - Seek file
        DB      cName           ;43H - Change file attributes
        DB      cNothing
        DB      cNothing
        DB      cNothing
        DB      cDrive          ;47H - Get current directory
        DB      cNothing
        DB      cNothing
        DB      cNothing
        DB      cName           ;4BH - Load or execute program
        DB      cNothing
        DB      cNothing
        DB      cName           ;4EH - Find first
        DB      cNothing
        DB      cNothing
        DB      cNothing
        DB      cNothing
        DB      cNothing
        DB      cNothing
        DB      cNothing
        DB      cName           ;56H - Rename file
        DB      cHandle         ;57H - Get/Set file date and time

; Function check routines table

FuncCheckTab    LABEL   ARGINT

        Dnear   CheckNothing
        Dnear   CheckName
        Dnear   CheckHandle
        Dnear   CheckDrive

DataSel DW      @data

ENDIF

IFNDEF __FLAT__

; Install system error handlers

@TSystemError@resume$qv PROC                ; 32-bit version is in SYSERR.CPP

        PUSH    SI
        PUSH    DI

; Save state of break flag and clear it.
        MOV     AX,3300H
        INT     21H
        MOV     @TSystemError@saveCtrlBreak,DL
        MOV     AX,3301H
        MOV     DL,0
        INT     21H

; Save & set Int 9 handler.
        MOV     AX, 3509H
        INT     21H
        MOV     [WORD PTR SaveInt09], BX
        MOV     [WORD PTR SaveInt09+2], ES

; If running inside the DOS IDE, do not install Int 9 handler.
        CMP     @TSystemError@inIDE,0
        JNE   @@1

        PUSH    DS
        MOV     AX, 2509H
        MOV     DX, OFFSET Int09Handler
        PUSH    CS
        POP     DS
        INT     21H
        POP     DS

; Save & set Int 1B handler.
@@1:
        MOV     AX, 351BH
        INT     21H
        MOV     [WORD PTR SaveInt1B], BX
        MOV     [WORD PTR SaveInt1B+2], ES
        PUSH    DS
        MOV     AX, 251BH
        MOV     DX, OFFSET Int1BHandler
        PUSH    CS
        POP     DS
        INT     21H

        ; Ensure the Ctrl+Break flag is Off by default.
        CALL    @THardwareInfo@getBiosSelector$qv
        MOV     DS, AX
        AND     BYTE PTR DS:[71H], 7FH

        POP     DS

; Save Int 21 handler.
        MOV     AX, 3521H
        INT     21H
        MOV     [WORD PTR SaveInt21], BX
        MOV     [WORD PTR SaveInt21+2], ES

; DX is the offset of the handler that we'll install.  If there is exactly
; one floppy drive, we install a special handler that chains to our global
; Int 21 handler.  If there is more than one handler, then we simply install
; the global handler.  The global handler chains to the old int 21 vector.

        MOV     DX, OFFSET GInt21Handler
        CALL    @THardwareInfo@getBiosEquipmentFlag$qi
        AND     AX,0C1H
        DEC     AX
        JNE   @@1A
        MOV     DX, OFFSET SDInt21Handler
@@1A:   PUSH    DS
        MOV     AX, 2521H
        PUSH    CS
        POP     DS
        INT     21H
        POP     DS

; Save & set Int 23 handler.
@@2:
        MOV     AX, 3523H
        INT     21H
        MOV     [WORD PTR SaveInt23], BX
        MOV     [WORD PTR SaveInt23+2], ES
        PUSH    DS
        MOV     AX, 2523H
        MOV     DX, OFFSET Int23Handler
        PUSH    CS
        POP     DS
        INT     21H
        POP     DS

; Save & set Int 24 handler.  This sets the protected mode version
;  of the handler if we're running under DPMI16.
        MOV     AX, 3524H
        INT     21H
        MOV     [WORD PTR SaveInt24], BX
        MOV     [WORD PTR SaveInt24+2], ES
        PUSH    DS
        MOV     AX, 2524H
        MOV     DX, OFFSET Int24Handler
        PUSH    CS
        POP     DS
        INT     21H
        POP     DS

; If we're in DPMI16, we also need to set a real mode handler for
;  Int 24.

        CMP     @THardwareInfo@dpmiFlag, 01H
        JNE   @@no_real_int24
        CALL    @TSystemError@installRealInt24$qv

@@no_real_int24:

; Save & set Int 10 handler, Check input status (to force an Int 23 if
;   a ctrl-C is in the buffer?) and the reinstall old Int 10 handler.
        MOV     AX, 3510H
        INT     21H
        MOV     [WORD PTR SaveInt10], BX
        MOV     [WORD PTR SaveInt10+2], ES
        PUSH    DS
        MOV     AX, 2510H
        MOV     DX, OFFSET Int10Handler
        PUSH    CS
        POP     DS
        INT     21H
        POP     DS

        MOV     AH,0BH
        INT     21H

        PUSH    DS
        MOV     AX, 2510H
        MOV     DX, [WORD PTR SaveInt10]
        MOV     DS, [WORD PTR SaveInt10+2]
        INT     21H
        POP     DS

; Exit...
        POP     DI
        POP     SI
        RET
@TSystemError@resume$qv endp

@TSystemError@installRealInt24$qv PROC
        USES    DI

        MOV     CX, 19H     ; Zero out Int24Regs.
        PUSH    DS          ; Assume direction flag is clear!
        POP     ES
        LEA     DI, [@TSystemError@Int24Regs]
        XOR     AX, AX
        REP     STOSW

        MOV     WORD PTR [@TSystemError@Int24Regs._ax], 3524H
        MOV     AX, 0300H
        MOV     BX, 0021H
        XOR     CX, CX
        LEA     DI, [@TSystemError@Int24Regs]
        INT     31H

        MOV     AX, WORD PTR [@TSystemError@Int24Regs._bx]
        MOV     WORD PTR [SaveInt24R], AX
        MOV     AX, WORD PTR [@TSystemError@Int24Regs._es]
        MOV     WORD PTR [SaveInt24R+2], AX

        MOV     CX, 19H     ; Zero out Int24Regs.
        LEA     DI, [@TSystemError@Int24Regs]
        XOR     AX, AX
        REP     STOSW

        MOV     WORD PTR [@TSystemError@Int24Regs._ax], 2524H
        MOV     AX, WORD PTR [@TSystemError@Int24RMCallback]
        MOV     WORD PTR [@TSystemError@Int24Regs._dx], AX
        MOV     AX, WORD PTR [@TSystemError@Int24RMCallback+2]
        MOV     WORD PTR [@TSystemError@Int24Regs._ds], AX
        MOV     AX, 0300H
        MOV     BX, 0021H
        XOR     CX, CX
        LEA     DI, [@TSystemError@Int24Regs]
        INT     31H

        RET
@TSystemError@installRealInt24$qv ENDP

@TSystemError@removeRealInt24$qv PROC
        USES    DI

        MOV     CX, 19H     ; zero out INT_REGS structure.
        PUSH    DS          ; assume direction flag is clear!
        POP     ES
        LEA     DI, [@TSystemError@Int24Regs]
        XOR     AX, AX
        REP     STOSW

        MOV     BX, WORD PTR [SaveInt24R]
        MOV     CX, WORD PTR [SaveInt24R+2]
        MOV     WORD PTR [@TSystemError@Int24Regs._ax], 2524H
        MOV     WORD PTR [@TSystemError@Int24Regs._dx], BX
        MOV     WORD PTR [@TSystemError@Int24Regs._ds], CX
        MOV     AX, 0300H
        MOV     BX, 0021H
        XOR     CX, CX
        LEA     DI, [@TSystemError@Int24Regs]

        INT     31H
        RET
@TSystemError@removeRealInt24$qv ENDP


; Remove system error handlers

@TSystemError@suspend$qv PROC               ; 32-bit version is in SYSERR.CPP
        PUSH    SI
        PUSH    DI

; Restore handlers for Int 9, 1B, 21, 23, 24.
        PUSH    DS
        MOV     AX, 2509H
        MOV     DX, [WORD PTR SaveInt09]
        MOV     DS, [WORD PTR SaveInt09+2]
        INT     21H
        POP     DS

        PUSH    DS
        MOV     AX, 251BH
        MOV     DX, [WORD PTR SaveInt1B]
        MOV     DS, [WORD PTR SaveInt1B+2]
        INT     21H
        POP     DS

        PUSH    DS
        MOV     AX, 2521H
        MOV     DX, [WORD PTR SaveInt21]
        MOV     DS, [WORD PTR SaveInt21+2]
        INT     21H
        POP     DS

        PUSH    DS
        MOV     AX, 2523H
        MOV     DX, [WORD PTR SaveInt23]
        MOV     DS, [WORD PTR SaveInt23+2]
        INT     21H
        POP     DS

        PUSH    DS
        MOV     AX, 2524H
        MOV     DX, [WORD PTR SaveInt24]
        MOV     DS, [WORD PTR SaveInt24+2]
        INT     21H
        POP     DS

; If we're in DPMI16, we also need to remove the real mode handler for
; Int 24.

        CMP     @THardwareInfo@dpmiFlag, 01H
        JNE   @@no_real_int24
        CALL    @TSystemError@removeRealInt24$qv

@@no_real_int24:

; Restore original state of Ctrl-Break flag.
        MOV     AX,3301H
        MOV     DL, @TSystemError@saveCtrlBreak
        INT     21H

        POP     DI
        POP     SI
        RET
@TSystemError@suspend$qv endp
ENDIF

IFNDEF __FLAT__

; INT 09H handler signature

        DB      'TVI9'

; INT 09H handler

Int09Handler PROC FAR

        PUSH    ES
        PUSH    DS
        PUSH    DI
        PUSH    AX

; Get key state information before calling old handler to handle key.
; This is so that if the old handler stuffs a key that we want to alter,
; we can!
        CALL    @THardwareInfo@getBiosSelector$qv
        MOV     DS, AX
        MOV     ES, CS:[DataSel]
        MOV     DI, DS:[KeyBufTail]
        IN      AL, 60H
        MOV     AH, DS:[KeyFlags]
        PUSHF
        CALL    ES:[SaveInt09]

; If key is not being released, exit.
        TEST    AL, 80H
        JNE   @@9

; Search key conversion table for a match of the scan code and correct shift
;   state.
        PUSH    SI
        PUSH    CX
        MOV     SI, OFFSET CS:KeyConvertTab
        MOV     CX, KeyConvertCnt
@@1:    CMP     AL, CS:[SI]
        JNE   @@2
        TEST    AH, CS:[SI+1]
        JNE   @@3
@@2:    ADD     SI, 4
        LOOP  @@1
        JMP     SHORT @@8

; Having found match, if the old handler inserted a keystroke (in which
;   case KeyBufTail will be different) just overwrite that keystroke in
;   the buffer.  If not, then we need to increment the keyboard buffer,
;   adjusting for wraparound and possible overflow.
@@3:    CMP     DI, DS:KeyBufTail
        JNE   @@5
        MOV     AX, DI
        INC     AX
        INC     AX
        CMP     AX, DS:[KeyBufEndPtr]
        JNE   @@4
        MOV     AX, DS:[KeyBufOrgPtr]
@@4:    CMP     AX, DS:[KeyBufHead]
        JE    @@8
        MOV     DS:[KeyBufTail],AX
        MOV     DI, AX

; Write our "keystroke" into the buffer.
@@5:    MOV     AX, CS:[SI+2]
        MOV     [DI], AX

; Exit.
@@8:    POP     CX
        POP     SI

        CALL    @THardwareInfo@getBiosSelector$qv
        MOV     DS, AX
        TEST    BYTE PTR DS:[71H], 80H      ; Ctrl+Break hit?
        JZ    @@9

        AND     BYTE PTR DS:[71H], 7FH
        MOV     DS, CS:[DataSel]
        MOV     @TSystemError@ctrlBreakHit, 1

@@9:    POP     AX
        POP     DI
        POP     DS
        POP     ES
        IRET
Int09Handler ENDP

; INT 1BH handler

Int1BHandler    PROC FAR

; Clear Bios Ctrl-Break flag and set Turbo Vision's Ctrl-Break flag.
        PUSH    DS
        PUSH    AX
        CALL    @THardwareInfo@getBiosSelector$qv
        MOV     DS,AX
        AND     BYTE PTR DS:[71H],7FH
        MOV     DS, CS:[DataSel]
        MOV     @TSystemError@ctrlBreakHit,1
        POP     AX
        POP     DS
        IRET

Int1BHandler    ENDP

; INT 21H handler for all systems.  This assists in dealing with critical
;  errors.

GInt21Handler   PROC FAR
        PUSH    DS

        PUSHF                     ; Check for re-entrance immediately.
        MOV     DS, CS:[DataSel]
        CLI
        CMP     [InGInt21], 00H
        JNE   @@jmpToInt21
        INC     [InGInt21]
        POPF

        PUSH    AX
        PUSH    BP
        MOV     BP, SP
        MOV     DS, CS:[DataSel]
        MOV     AX, [BP+4]              ; Copy DS from old to new stack.
        MOV     WORD PTR [GInt21Stack+03FEH], AX

        MOV     AX, [BP+10]             ; These are flags prior to INT 21H.
        MOV     [SavedFlags], AX        ; Store the flags.

        POP     BP
        POP     AX

        MOV     [OldSS], SS
        MOV     [OldSP], SP
        MOV     SS, [NewSS]
        MOV     SP, [NewSP]
        POP     DS

@@entry:
        ; Save registers for retry.
        PUSH    AX
        PUSH    BX
        PUSH    CX
        PUSH    DX
        PUSH    SI
        PUSH    DI
        PUSH    ES
        PUSH    DS
        PUSH    BP

        ; Call old Int 21 handler & clear flag for critical error handler
@@callToInt21:
        PUSHF

        PUSH    DS
        PUSH    AX              ; Save AX for later.
        PUSH    BP
        MOV     BP, SP
        MOV     DS, CS:[DataSel]
        MOV     AX, [SavedFlags]
        MOV     [BP+6], AX      ; Set the flags that the INT 21H call will get.
        POP     BP
        POP     AX
        POP     DS

        PUSH    CS
        SUB     SP, 6
        PUSH    BP
        MOV     BP, SP
        PUSH    DS
        PUSH    SI
        MOV     DS, CS:[DataSel]
        MOV     SI, WORD PTR [SaveInt21]
        MOV     WORD PTR [critFlag], 0FFH
        MOV     [BP+2], SI
        MOV     SI, WORD PTR [SaveInt21+2]
        MOV     [BP+4], SI
        MOV     SI, offset @@retFromDOS
        MOV     [BP+6], SI
        POP     SI
        POP     DS
        POP     BP
        RETF

        ; Since this handler is not reentrant, this part of the code is
        ; used to jump to the old int 21 handler unconditionally if we
        ; are already active.
@@jmpToInt21:
        POPF
        POP     DS
        SUB     SP, 4
        PUSH    BP
        MOV     BP, SP
        PUSH    DS
        PUSH    SI
        MOV     DS, CS:[DataSel]
        MOV     SI, [WORD PTR SaveInt21]
        MOV     [BP+2], SI
        MOV     SI, [WORD PTR SaveInt21+2]
        MOV     [BP+4], SI
        POP     SI
        POP     DS
        POP     BP
        RETF

@@retFromDOS:
        ; Check for critical error during call.
        PUSHF
        PUSH    DS
        MOV     DS, CS:[DataSel]
        CMP     WORD PTR [critFlag], 0FFH
        JNE   @@criticalError

@@done:
        ; Alter saved flags on stack to reflect new flag settings.
        ; Note that the stack at this point still has the old DS and Flags and
        ; DS points to the data segment.
        PUSH    BP
        MOV     BP, SP
        PUSH    AX
        MOV     AX, [BP+4]
        MOV     [SavedFlags], AX
        POP     AX
        POP     BP

        ; Switch back to old stack.  (hence why we didn't have to clear off
        ; the other stack.)
        MOV     SS, [OldSS]
        MOV     SP, [OldSP]
        MOV     [InGInt21], 00H

        PUSH    BP
        MOV     BP, SP
        PUSH    AX
        MOV     AX, [SavedFlags]
        MOV     [BP+8], AX

        ; Pass back the DS returned by INT 21H. It may have changed.
        MOV     AX, WORD PTR [GInt21Stack+03EAH]
        MOV     [BP+2], AX

        POP     AX
        POP     BP

        POP     DS
        IRET

@@criticalError:
        ; There was a critical error so ask the user for a response.
        PUSH    AX
        PUSH    BX
        PUSH    CX
        PUSH    DX
        PUSH    SI
        PUSH    DI
        PUSH    ES

        MOV     AX, [critDrive]
        MOV     DI, [critFlag]
        PUSH    AX
        PUSH    DI
        MOV     AX, SEG @TSystemError@sysErrorFunc
        MOV     ES, AX
        CALL    DWORD PTR ES:[@TSystemError@sysErrorFunc]
        ADD     SP, 4
        OR      AX, AX
        JE    @@retry

        POP     ES
        POP     DI
        POP     SI
        POP     DX
        POP     CX
        POP     BX
        POP     AX
        JMP   @@done

@@retry:
        ; User said retry, so restore the registers to entry conditions.
        ADD     SP, 12H
        POP     BP
        POP     DS
        POP     ES
        POP     DI
        POP     SI
        POP     DX
        POP     CX
        POP     BX
        POP     AX
        JMP   @@entry

GInt21Handler   ENDP


; INT 21H handler for single drive systems.

SDInt21Handler  PROC FAR

        PUSHF
        STI
        CMP     AH,36H
        JB    @@1
        CMP     AH,57H
        JA    @@1
        PUSH    DX
        PUSH    BX
        MOV     BL,AH
        XOR     BH,BH
        MOV     BL,CS:FuncClassTab[BX-36H]
        CALL    CS:FuncCheckTab[BX]
        POP     BX
        POP     DX
        JC    @@2

@@1:    POPF
        JMP     GInt21Handler           ; Chain to old handler.

@@2:    POPF
        STI
        CMP     AH,36H
        MOV     AX,0FFFFH
        JE    @@3
        MOV     AX,5

@@3:    STC
        RETF    2

SDInt21Handler    ENDP

; Check filename

CheckName:

        MOV     BX,DX
        MOV     DX,[BX]
        AND     DL,1FH
        DEC     DL
        CMP     DH,':'
        JE      CheckAbsDrive
        JMP     SHORT CheckCurDrive

; Check handle

CheckHandle:

        MOV     BX,SP
        MOV     BX,SS:[BX+2]
        PUSH    AX
        PUSH    DS
        MOV     AX,4400H
        MOV     DS, CS:[DataSel]
        PUSHF
        CALl    GInt21Handler
        POP     DS
        POP     AX
        OR      DL,DL
        JNS     CheckAbsDrive
        JMP     SHORT CheckNothing

; Check drive

CheckDrive:

        DEC     DL
        JNS     CheckAbsDrive

; Check current drive

CheckCurDrive:

        PUSH    AX
        PUSH    DS
        MOV     AH,19H
        MOV     DS, CS:[DataSel]
        PUSHF
        CALL    GInt21Handler
        MOV     DL,AL
        POP     DS
        POP     AX

; Check absolute drive
; In    DL = Drive (0=A, 1=B, etc)
; Out   CF = 1 if drive swap failed

CheckAbsDrive:

        CMP     DL,2
        JAE     CheckNothing
        PUSH    DS
        PUSH    AX

        CALL    @THardwareInfo@getBiosSelector$qv
        MOV     DS,AX

        PUSH    CX
        MOV     CL, 6
        MOV     AL, DS:[10h]
        SHR     AL, CL
        POP     CX
        CMP     AL, 1
        JAE   @@1

        MOV     AL, DS:[104H]
        CMP     AL, 0FFH
        JE    @@1
        CMP     DL, AL
        JE    @@1
        PUSH    ES
        PUSH    DS
        PUSH    DI
        PUSH    SI
        PUSH    DX
        PUSH    CX
        MOV     DS, CS:[DataSel]
        PUSH    DX
        MOV     AX, 21
        PUSH    AX
        MOV     AX, SEG @TSystemError@sysErrorFunc
        MOV     ES, AX
        CALL    DWORD PTR ES:[@TSystemError@sysErrorFunc]
        ADD     SP, 4
        POP     CX
        POP     DX
        POP     SI
        POP     DI
        POP     DS
        POP     ES
        MOV     DS:[104H], DL
@@1:
        POP     AX
        POP     DS

; No check required

CheckNothing:
        CLC
        RET

; INT 23H and temporary INT 10H handler

Int10Handler:
Int23Handler:
        IRET

; INT 24H handler

Int24Handler    PROC FAR

        STI                             ;Enable interrupts
        PUSH    DS
        PUSH    DI

        PUSH    ES
        PUSH    AX
        PUSH    BX
        PUSH    CX
        PUSH    DX
        PUSH    BP
        PUSH    SI

        XOR     BX, BX      ; Get extended error information
        MOV     AH, 59H
        INT     21H

        SUB     AX, 13H     ; Convert extended error code to 00H-14H
                            ; Anything over 14H will display a generic message
                            ; which isn't likely.

        MOV     DI, AX      ; Save the extended error code

        POP     SI
        POP     BP
        POP     DX
        POP     CX
        POP     BX
        POP     AX
        POP     ES

        CMP     DI, 09H                 ;Printer out of paper
        JE    @@0
        TEST    AH, 80H                 ;0 = disk error
        JE    @@1
        MOV     DS, BP
        TEST    BYTE PTR DS:[SI+5], 80H ;Block device gets error 0DH
        JE    @@1
@@0:    MOV     AL, 0FEH
@@1:    MOV     DS, CS:[DataSel]
        MOV     WORD PTR [critFlag], DI
        MOV     WORD PTR [critDrive], AX  ; AH = 0, AL is drive code.
        POP     DI
        POP     DS
        MOV     AX, 03H                 ;Fail the error in all cases.
        IRET

Int24Handler    ENDP

@TSystemError@Int24PMThunk$qv   PROC FAR

        PUSH    SI
        PUSH    DS

        PUSH    BP          ; Save original BP

        XOR     AX, AX      ; Allocate a descriptor for the device
        MOV     CX, 1       ; header's segment.
        INT     31H
        MOV     BP, AX      ; BP = descriptor for the segment

        MOV     DX, WORD PTR (INT_REGS PTR ES:[DI]._bp)
        MOV     CL, 4
        SHL     DX, CL
        XOR     CX, CX
        MOV     BX, BP
        MOV     AX, 7       ; Set base address (16 * real mode segment addr)
        INT     31H

        MOV     BX, BP
        XOR     CX, CX
        MOV     DX, -1
        MOV     AX, 8       ; Set segment limit (64k)
        INT     31H

        MOV     BX, DI
        MOV     AX, WORD PTR (INT_REGS PTR ES:[BX]._ax)
        MOV     DI, WORD PTR (INT_REGS PTR ES:[BX]._di)
        MOV     SI, WORD PTR (INT_REGS PTR ES:[BX]._si)
        PUSHF
        CALL    Int24Handler
        MOV     DI, BX

        PUSH    AX

        MOV     BX, BP
        MOV     AX, 1
        INT     31h         ; Free the descriptor

        POP     AX
        POP     BP

        POP     DS
        POP     SI

        MOV     WORD PTR (INT_REGS PTR ES:[DI]._ax), AX
        LODSW
        MOV     WORD PTR (INT_REGS PTR ES:[DI]._ip), AX
        LODSW
        MOV     WORD PTR (INT_REGS PTR ES:[DI]._cs), AX
        LODSW
        MOV     WORD PTR (INT_REGS PTR ES:[DI]._flags), AX
        ADD     WORD PTR (INT_REGS PTR ES:[DI]._sp), 6

        IRET
@TSystemError@Int24PMThunk$qv   ENDP

@TSystemError@setupDPMI$qv      PROC FAR
        USES    SI, DI

        ; Allocate real mode callback address.
        PUSH    DS
        MOV     AX, 0303H
        PUSH    DS
        POP     ES
        MOV     DI, OFFSET @TSystemError@Int24Regs
        MOV     SI, SEG @TSystemError@Int24PMThunk$qv
        MOV     DS, SI
        MOV     SI, OFFSET @TSystemError@Int24PMThunk$qv
        INT     31H
        POP     DS
        MOV     WORD PTR [@TSystemError@Int24RMCallback], DX
        MOV     WORD PTR [@TSystemError@Int24RMCallback+2], CX

        RET
@TSystemError@setupDPMI$qv      ENDP

@TSystemError@shutdownDPMI$qv   PROC FAR

        ; Free real mode callback thunk.
        MOV     AX, 0304H
        MOV     CX, [@TSystemError@Int24RMCallback+2]
        MOV     DX, [@TSystemError@Int24RMCallback]
        INT     31H

        RET

@TSystemError@shutdownDPMI$qv   ENDP

ENDIF

        END
