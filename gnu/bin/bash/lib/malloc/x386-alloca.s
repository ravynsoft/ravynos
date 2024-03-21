;; alloca386.s 1.2
;; GNU-compatible stack allocation function for Xenix/386.
;; Written by Chip Salzenberg at ComDev.
;; Last modified 90/01/11
;;> Is your alloca clearly better than the one in i386-alloca.s?  I haven't
;;> looked at either.
;;
;;They're different because Xenix/386 has a different assembler.  SCO
;;Xenix has the Microsoft C compiler and the Microsoft macro assembler,
;;called "masm".  MASM's assembler syntax is quite different from AT&T's
;;in all sorts of ways.  Xenix people can't use the AT&T version.
;;-- 
;;Chip Salzenberg at ComDev/TCT     <chip@tct.uucp>, <uunet!ateng!tct!chip>

	TITLE   $alloca386

	.386
DGROUP	GROUP	CONST, _BSS, _DATA
_DATA	SEGMENT  DWORD USE32 PUBLIC 'DATA'
_DATA      ENDS
_BSS	SEGMENT  DWORD USE32 PUBLIC 'BSS'
_BSS      ENDS
CONST	SEGMENT  DWORD USE32 PUBLIC 'CONST'
CONST      ENDS
_TEXT	SEGMENT  DWORD USE32 PUBLIC 'CODE'
	ASSUME   CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP

	PUBLIC  _alloca
_alloca PROC NEAR

; Get argument.
	pop     edx             ; edx -> return address
	pop     eax             ; eax = amount to allocate

; Validate allocation amount.
	add     eax,3
	and     eax,not 3
	cmp     eax,0
	jg      aa_size_ok
	mov     eax,4
aa_size_ok:

; Allocate stack space.
	mov     ecx,esp         ; ecx -> old stack pointer
	sub     esp,eax         ; perform allocation
	mov     eax,esp         ; eax -> new stack pointer

; Copy the three saved register variables from old stack top to new stack top.
; They may not be there.  So we waste twelve bytes.  Big fat hairy deal.
	push    DWORD PTR 8[ecx]
	push    DWORD PTR 4[ecx]
	push    DWORD PTR 0[ecx]

; Push something so the caller can pop it off.
	push    eax

; Return to caller.
	jmp     edx

_alloca ENDP

_TEXT   ENDS
	END
