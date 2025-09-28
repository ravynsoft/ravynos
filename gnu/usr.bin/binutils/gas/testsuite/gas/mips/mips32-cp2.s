# source file to test assembly of mips32 cop2 instructions

      .set noreorder
      .set noat

      .text
text_label:
      # unprivileged coprocessor instructions.
      # these tests use cp2 to avoid other (cp0, fpu, prefetch) opcodes.

	.ifndef r6
      bc2f    text_label
      nop
      bc2fl   text_label
      nop
      bc2t    text_label
      nop
      bc2tl   text_label
      nop
	.endif
      # XXX other BCzCond encodings not currently expressable
      cfc2    $1, $2
      cop2    0x1234567               # disassembles as c2 ...
      ctc2    $2, $3
      mfc2    $3, $4
      mfc2    $4, $5, 0               # disassembles without sel
      mfc2    $5, $6, 7
      mtc2    $6, $7
      mtc2    $7, $8, 0               # disassembles without sel
      mtc2    $8, $9, 7


	.ifndef r6
      # Cop2 branches with cond code number, like bc1t/f
      bc2f    $cc0,text_label
      nop
      bc2fl   $cc1,text_label
      nop
      bc2t    $cc6,text_label
      nop
      bc2tl   $cc7,text_label
      nop
	.endif
