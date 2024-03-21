# source file to test assembly of mips32 instructions

        .set noreorder
      .set noat

      .text
text_label:

      # unprivileged CPU instructions

      clo     $1, $2
      clz     $3, $4
	.ifndef r6
      madd    $5, $6
      maddu   $7, $8
      msub    $9, $10
      msubu   $11, $12
	.endif
      mul     $13, $14, $15
      pref    4, ($16)
	.ifndef r6
      pref    4, 2047($17)
      pref    4, -2048($18)
	.endif
      ssnop


      # privileged instructions

      cache   5, ($1)
	.ifndef r6
      cache   5, 2047($2)
      cache   5, -2048($3)
      .set at
      cache   5, 32768($4)
      cache   5, -32769($5)
      cache   5, 32768
      cache   5, -32769
      .set noat
	.endif
      eret
      tlbp
      tlbr
      tlbwi
      tlbwr
      wait
      wait    0                       # disassembles without code
      wait    0x345

      # For a while break for the mips32 ISA interpreted a single argument
      # as a 20-bit code, placing it in the opcode differently to
      # traditional ISAs.  This turned out to cause problems, so it has
      # been removed.  This test is to assure consistent interpretation.
      break
      break   0                       # disassembles without code
      break   0x345
      break   0x48,0x345              # this still specifies a 20-bit code

      # Instructions in previous ISAs or CPUs which are now slightly
      # different.
      sdbbp
      sdbbp   0                       # disassembles without code
      sdbbp   0x345

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
      .space  8
