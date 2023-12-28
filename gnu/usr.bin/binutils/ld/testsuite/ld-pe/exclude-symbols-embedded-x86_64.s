.global sym1
.global sym2
.global sym3
.global sym4
.global sym5
sym1:
sym2:
sym3:
sym4:
sym5:
  ret

.section .drectve,"yn"
.ascii " -exclude-symbols:sym2,unknownsym"
.ascii " -exclude-symbols:unknownsym,sym4"
