# source file to test assembly of mips64 instructions

        .set noreorder
      .set noat

      .globl text_label .text
text_label:

      # unprivileged CPU instructions

      dclo    $1, $2
      dclz    $3, $4
