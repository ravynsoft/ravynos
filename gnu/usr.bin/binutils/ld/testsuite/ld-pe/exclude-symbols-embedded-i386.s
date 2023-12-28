.global _sym1
.global _sym2
.global _sym3
.global _sym4
.global _sym5
_sym1:
_sym2:
_sym3:
_sym4:
_sym5:
  ret

.section .drectve,"yn"
.ascii " -exclude-symbols:sym2,unknownsym"
.ascii " -exclude-symbols:unknownsym,sym4"
