# Source file used to test -mips5 instructions.

text_label:
	cvt.s.pl	$f16, $f18
	cvt.s.pu	$f18, $f20

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
      .space  8
