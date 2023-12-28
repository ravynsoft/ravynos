# from linux kernel entry.s
# test line 10 ".byte \type", BFD_RELOC_8 -> BFD_RELOC_RLARCH_ADD8 -> R_LARCH_ADD8

.macro UNWIND_HINT type:req sp_reg=0 sp_offset=0 end=0
.Lunwind_hint_ip_\@:
 .pushsection .discard.unwind_hints
  .long .Lunwind_hint_ip_\@ - .
  .short \sp_offset
  .byte \sp_reg
  .byte \type
  .byte \end
  .balign 4
 .popsection
.endm

UNWIND_HINT type=ORC_TYPE_CALL sp_reg=2
