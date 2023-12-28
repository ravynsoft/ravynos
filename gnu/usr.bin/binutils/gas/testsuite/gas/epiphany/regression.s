;; -*-asm-*-

	TABLE=0x8000
	RZ=r63

	.macro FAIL
	mov	r0,#1
	trap	3
	.endm

	.macro PASS
	mov	r0,#0
	trap	3
	.endm


	.macro VERIFY ra,rb,ref,label
	sub	\ra,\rb,\ref
	beq	\label
	FAIL
	.endm


/*****************************************/
/*INITIALIZING REGISTERS                 */
/*****************************************/
/*Check that sum is correct*/
START:	MOV R0, #TABLE   ; //Setting R0 to TABLE
	LSL R0,R0,#2	 ; //Create 00020000

	;; Load r1.63 with 1..63
	.irpc num,63
	mov r\num,#\num
	.endr


	;; Sum the registers
	.irpc num,62
	add r63,r63,r\num
	.endr

	mov r62,#2016	 ;//Correct sum of 1..63 = 63*32 + 63
	VERIFY	r63,r63,R62,BRANCH1;//CHECK SUM


/*****************************************/
/*BRANCHING                              */
/*****************************************/
//Check that all condition codes work
BRANCH1:	BEQ  BRANCH2     ; //taken
	FAIL	         ;
	FAIL		 ;
	FAIL		 ;
	FAIL             ;
BRANCH2:	BNE  FAIL_BRANCH ; //not taken
BRANCH3:	BGT  FAIL_BRANCH ; //not taken
BRANCH4:	BGTE BRANCH5     ; //taken
	FAIL		;
BRANCH5:	BLTE BRANCH6	; //taken
	FAIL		;
BRANCH6:	BLT  FAIL_BRANCH ; //not taken
BRANCH8:	B    LONGJUMP    ; //taken
	FAIL		 ;
RETURN:	bl  FUNCTION	 ; //jump to subroutine
	MOV R63,JARLAB   ;//REGISTER JUMP
	JR  R63		 ;
	FAIL		 ;
JARLAB:	MOV R63,FUNCTION ; //REGISTER CALL
	JALR R63		 ; //16 bit
        B    NEXT	 ; //jump over fail
	FAIL		;

FAIL_BRANCH:	FAIL	; //fail branch

/*****************************************/
/*LOAD-STORE DISPLACEMENT                */
/*****************************************/
//Check max displacement value(0xf)
//Check that offset is correct
//all load/stores are aligned
//this gives greater range(2 more bits)
//offset is shifted by 2x bits

NEXT:	STRB R4,[R0,#0x0] ;//Store Byte
	LDRB R63,[R0,#0x0] ;//Load Byte
	VERIFY	R63,R63,R4,STOREB  ;

STOREB:	STRB R5,[R0,#0xf] ;//Store Byte
	LDRB R63,[R0,#0xf] ;//Load Byte
	VERIFY	R63,R63,R5,STORES  ;

STORES:	STRH R4,[R0,#0x0] ;//Store Short
	LDRH R63,[R0,#0x0] ;//Load Short
	VERIFY	R63,R63,R4,STORES2  ;

STORES2:	STRH R5,[R0,#0xe] ;//Store Short
	LDRH R63,[R0,#0xe] ;//Load Short
	VERIFY	R63,R63,R5,STORE  ;

STORE:	STR  R4,[R0,#0x0] ;//Store Word
	LDR  R63,[R0,#0x0] ;//Load Word
	VERIFY	R63,R63,R4,STORE2  ;

STORE2:	STR  R5,[R0,#0xc] ;//Store Word
	LDR  R63,[R0,#0xc] ;//Load Word
	VERIFY	R63,R63,R5,STOREBI ;


/*****************************************/
/*LOAD-STORE INDEX                       */
/*****************************************/

STOREBI:	STRB R4,[R0,R4]	 ;//Store Word
	LDRB R63,[R0,R4]	 ;//Load Word
	VERIFY	R63,R63,R4,STORESI	;

STORESI:	STRH R5,[R0,R4]	 ;//Store Word
	LDRH R63,[R0,R4]	 ;//Load Word
	VERIFY	R63,R63,R5,STOREI	 ;

STOREI:	STR  R6,[R0,R4]	 ;//Store Word
	LDR  R63,[R0,R4]	 ;//Load Word
	VERIFY	R63,R63,R6,PMB	 ;

/*****************************************/
/*LOAD-STORE POSTMODIFY                  */
/*****************************************/

PMB:	STRB R4,[R0],R4	 ;//Store Word
	SUB  R0,R0,#0x4	 ;//restoring R0
	LDRB R63,[R0],R4	 ;//Load Word
	SUB  R0,R0,#0x4	 ;//restoring R0
	VERIFY	R63,R63,R4,PMS	 ;

PMS:	STRH R5,[R0],R4	 ;//Store Word
	SUB  R0,R0,#0x4	 ;//restoring R0
	LDRH R63,[R0],R4	 ;//Load Word
	VERIFY	R63,R63,R5,PM	 ;

PM:	SUB  R0,R0,#0x4	 ;//restoring R0
	STR  R6,[R0],R4	 ;//Store Word
	SUB  R0,R0,#0x4	 ;//restoring R0
	LDR  R63,[R0],R4	 ;//Load Word
	SUB  R0,R0,#0x4	 ;//restoring R0
	VERIFY	R63,R63,R6,MOVLAB	 ;



/*****************************************/
/*IMMEDIATE LOAD                         */
/*****************************************/
MOVLAB:	MOV R63,#0xFF;
	MOV R1,#0xFF;
	VERIFY	R63,R63,R1,ADDLAB	;

/*****************************************/
/*2 REG ADD/SUB PROCESSING               */
/*****************************************/
ADDLAB:	ADD R63,R2,#3;	//2+3=5
	VERIFY	R63,R63,#5,SUBLAB	;
SUBLAB:	SUB R63,R2,#1;	//2+1=1
	VERIFY	R63,R63,#1,LSRLAB	;

/*****************************************/
/*SHIFTS                                 */
/*****************************************/
//Note ASR does not work

	//Immediates
LSRLAB:	LSR R63,R6,#0x2   ; //6>>2=1
	VERIFY	R63,R63,#1,LSLLAB	 ;
LSLLAB:	LSL R63,R3,#0x2   ; //3<<2=12
	VERIFY	R63,R63,#12,LSRILAB	;
	//Registers
LSRILAB:	LSR R63,R6,R2     ; //6>>2=1
	VERIFY	R63,R63,#1,LSLILAB	 ;
LSLILAB:	LSL R63,R3,R2     ; //3<<2=12
	VERIFY	R63,R63,#12,ORRLAB	;


/*****************************************/
/*LOGICAL                                */
/*****************************************/
ORRLAB:	ORR R5,R3,R4     ; //0x3 | 0x4 -->0x7
	VERIFY	R63,R5,#7,ANDLAB	 ;
ANDLAB:	AND R5,R3,R4     ; //0x3 & 0x4 -->0
	VERIFY	R63,R5,#0,EORLAB	 ;
EORLAB:	EOR R5,R3,R2     ; //0x3 ^ 0x2 -->1
	VERIFY	R63,R5,#1,ADD3LAB	 ;


/****************************************/
/*3-REGISTER ADD/SUB                     */
/*****************************************/
ADD3LAB:	ADD R63,R2,R3    ; //3+2=5
	VERIFY	R63,R63,#5,SUB3LAB	 ;
SUB3LAB:	SUB R63,R6,R4    ; //6-4=2
	VERIFY	R63,R63,#2,MOVRLAB	 ;

/*****************************************/
/*MOVE REGISTER                          */
/*****************************************/
MOVRLAB:	MOV R63,R2	;
	VERIFY	R63,R63,#2,NOPLAB	 ;

/*****************************************/
/*MOVE TO/FROM SPECIAL REGISTER          */
/*****************************************/
MOVTFLAB:	MOVTS status,R0	;
	MOVFS R63,status	;
	VERIFY	R63,R63,R0,MOVTFLAB	;


/*****************************************/
/*NOP                                    */
/*****************************************/
NOPLAB:	NOP		;
	NOP		;
	NOP		;
	NOP		;

/*****************************************/
/*PASS INDICATOR                         */
/*****************************************/
PASSED:	PASS;
	IDLE;
/*****************************************/
/*FAIL INDICATOR                         */
/*****************************************/
FAILED:	FAIL;
	IDLE;

/*****************************************/
/*LONG JUMP INDICATOR                    */
/*****************************************/
LONGJUMP:	B RETURN; //jump back to next
/*****************************************/
/*SUBROUTINE                             */
/*****************************************/
FUNCTION:	RTS;      //return from subroutine
