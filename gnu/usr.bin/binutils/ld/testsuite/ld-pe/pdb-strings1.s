.equ CV_SIGNATURE_C13, 4
.equ DEBUG_S_STRINGTABLE, 0xf3

.section ".debug$S", "rn"
.long CV_SIGNATURE_C13
.long DEBUG_S_STRINGTABLE
.long .strings_end - .strings_start

.strings_start:

.asciz ""
.asciz "foo"
.asciz "bar"
.asciz "baz"
.asciz "qux"

.strings_end:

.balign 4
