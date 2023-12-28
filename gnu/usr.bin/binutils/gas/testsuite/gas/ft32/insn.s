        # Used for all instructions that have a 3-address form
	.macro TERNARY insn
        # reg-reg
        \insn   $r31, $r0, $r0
        \insn   $r0, $r31, $r0
        \insn   $r0, $r0, $r31
        \insn   $r1, $r2, $r4
        \insn   $r8, $r16, $r0

        # immediate
        \insn   $r31, $r0, -512
        \insn   $r0, $r31, 0
        \insn   $r0, $r31, 1
        \insn   $r0, $r31, 511

        # short and byte
        \insn\().s $r0, $r31, $r1
        \insn\().s $r0, $r31, 77
        \insn\().b $r0, $r31, $r1
        \insn\().b $r0, $r31, 77

        .endm

        .macro RegUImm insn
        \insn   r0, r0, 0
        \insn   r0, r0, 65535
        \insn   r0, r31, 0
        \insn   r0, r31, 65535
        \insn   r31, r0, 0
        \insn   r31, r0, 65535
        .endm

	.macro CMPOP insn
        # reg-reg
        \insn   $r0, $r0
        \insn   $r31, $r0
        \insn   $r0, $r31

        # immediate
        \insn   $r0, -512
        \insn   $r31, 0
        \insn   $r31, 1
        \insn   $r31, 511

        # short and byte
        \insn\().s $r31, $r1
        \insn\().s $r31, 77
        \insn\().b $r31, $r1
        \insn\().b $r31, 77

        .endm

        .section .data
dalabel:
        .long   0

        .section .text
pmlabel:

        TERNARY add
        TERNARY sub
        TERNARY and
        TERNARY or
        TERNARY xor
        TERNARY xnor
        TERNARY ashl
        TERNARY lshr
        TERNARY ashr
        TERNARY ror
        TERNARY ldl
        TERNARY bins
        TERNARY bexts
        TERNARY bextu
        TERNARY flip

        CMPOP   addcc
        CMPOP   cmp
        CMPOP   tst
        CMPOP   btst

        # LDI, STI, EXI
        ldi.l   $r0,$r31,-128
        ldi.l   $r31,$r0,127
        ldi.s   $r0,$r31,-128
        ldi.s   $r0,$r31,127
        ldi.b   $r31,$r0,-128
        ldi.b   $r31,$r0,127
        sti.l   $r31,-128,$r0
        sti.l   $r0,127,$r31
        sti.s   $r31,-128,$r0
        sti.s   $r31,127,$r0
        sti.b   $r0,-128,$r31
        sti.b   $r0,127,$r31
        exi.l   $r0,$r31,-128
        exi.l   $r31,$r0,127
        exi.s   $r0,$r31,-128
        exi.s   $r0,$r31,127
        exi.b   $r31,$r0,-128
        exi.b   $r31,$r0,127

        # LPM, LPMI
        lpm.l   $r0,pmlabel
        lpm.s   $r16,pmlabel
        lpm.b   $r31,pmlabel
        lpmi.l  $r0,$r1,-128
        lpmi.s  $r16,$r1,127
        lpmi.b  $r31,$r1,-128

        # JMP
        jmp     pmlabel
        jmpi    $r16
        jmpx    31,$r28,1,pmlabel
        jmpc    nz,pmlabel

        # CALL
        call    pmlabel
        calli   $r16
        callx   31,$r28,1,pmlabel
        callc   nz,pmlabel

        # PUSH, POP
        push    $r0
        push    $r16
        push    $r31
        pop     $r0
        pop     $r16
        pop     $r31

        # LINK,UNLINK
        link    $r0,0
        link    $r16,65535
        link    $r31,1017
        unlink  $r0
        unlink  $r16
        unlink  $r31

        # RETURN,RETI
        return
        reti

        # LDA,STA,EXA
        lda.l   $r0,dalabel
        lda.s   $r16,dalabel
        lda.b   $r31,dalabel
        sta.l   dalabel,$r0
        sta.s   dalabel,$r16
        sta.b   dalabel,$r31
        exa.l   $r0,dalabel
        exa.s   $r16,dalabel
        exa.b   $r31,dalabel

        # LDK
        ldk     $r0,-524288
        ldk     $r0,524287
        ldk     $r0,0

        move    $r0,$r31
        move    $r31,$r0

        TERNARY udiv
        TERNARY umod
        TERNARY div
        TERNARY mod
        TERNARY strcmp
        TERNARY memcpy
        TERNARY memset
        TERNARY mul
        TERNARY muluh
        TERNARY streamin
        TERNARY streamini
        TERNARY streamout
        TERNARY streamouti

        strlen.l $r0,$r31
        strlen.l $r31,$r0
        strlen.s $r0,$r31
        strlen.s $r31,$r0
        strlen.b $r0,$r31
        strlen.b $r31,$r0
        stpcpy.l $r0,$r31
        stpcpy.l $r31,$r0
        stpcpy.s $r0,$r31
        stpcpy.s $r31,$r0
        stpcpy.b $r0,$r31
        stpcpy.b $r31,$r0
