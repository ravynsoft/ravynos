if test -n "${RELOCATING}"; then
OTHER_SECTIONS="
  /* Start of symbols and sections required to support CMEM instructions
     on NPS targets.  */

  PROVIDE (_cmem_start               = ADDR (.cmem)                                            );
  PROVIDE (_cmem_alter_start         = ADDR (.cmem_alter)                                      );
  PROVIDE (_cmem_shared_start        = ADDR (.cmem_shared)                                     );
  PROVIDE (_cmem_shared_alter_start  = ADDR (.cmem_shared_alter)                               );
  PROVIDE (_fmt_slot0_start          = ADDR (.fmt_slot0)                                       );
  PROVIDE (_fmt_slot1_start          = ADDR (.fmt_slot1)                                       );
  PROVIDE (_fmt_slot2_start          = ADDR (.fmt_slot2)                                       );
  PROVIDE (_fmt_slot3_start          = ADDR (.fmt_slot3)                                       );
  PROVIDE (_fmt_slot4_start          = ADDR (.fmt_slot4)                                       );
  PROVIDE (_fmt_slot5_start          = ADDR (.fmt_slot5)                                       );
  PROVIDE (_fmt_slot6_start          = ADDR (.fmt_slot6)                                       );
  PROVIDE (_fmt_slot7_start          = ADDR (.fmt_slot7)                                       );
  PROVIDE (_fmt_slot8_start          = ADDR (.fmt_slot8)                                       );
  PROVIDE (_fmt_slot9_start          = ADDR (.fmt_slot9)                                       );
  PROVIDE (_fmt_slot10_start         = ADDR (.fmt_slot10)                                      );
  PROVIDE (_fmt_slot11_start         = ADDR (.fmt_slot11)                                      );
  PROVIDE (_fmt_slot12_start         = ADDR (.fmt_slot12)                                      );
  PROVIDE (_fmt_slot13_start         = ADDR (.fmt_slot13)                                      );
  PROVIDE (_fmt_slot14_start         = ADDR (.fmt_slot14)                                      );
  PROVIDE (_fmt_slot15_start         = ADDR (.fmt_slot15)                                      );

  PROVIDE (_cmem_end                 = ADDR (.cmem)              + SIZEOF (.cmem)              );
  PROVIDE (_cmem_alter_end           = ADDR (.cmem_alter)        + SIZEOF (.cmem_alter)        );
  PROVIDE (_cmem_shared_end          = ADDR (.cmem_shared)       + SIZEOF (.cmem_shared)       );
  PROVIDE (_cmem_shared_alter_end    = ADDR (.cmem_shared_alter) + SIZEOF (.cmem_shared_alter) );
  PROVIDE (_fmt_slot0_end            = ADDR (.fmt_slot0)         + SIZEOF (.fmt_slot0)         );
  PROVIDE (_fmt_slot1_end            = ADDR (.fmt_slot1)         + SIZEOF (.fmt_slot1)         );
  PROVIDE (_fmt_slot2_end            = ADDR (.fmt_slot2)         + SIZEOF (.fmt_slot2)         );
  PROVIDE (_fmt_slot3_end            = ADDR (.fmt_slot3)         + SIZEOF (.fmt_slot3)         );
  PROVIDE (_fmt_slot4_end            = ADDR (.fmt_slot4)         + SIZEOF (.fmt_slot4)         );
  PROVIDE (_fmt_slot5_end            = ADDR (.fmt_slot5)         + SIZEOF (.fmt_slot5)         );
  PROVIDE (_fmt_slot6_end            = ADDR (.fmt_slot6)         + SIZEOF (.fmt_slot6)         );
  PROVIDE (_fmt_slot7_end            = ADDR (.fmt_slot7)         + SIZEOF (.fmt_slot7)         );
  PROVIDE (_fmt_slot8_end            = ADDR (.fmt_slot8)         + SIZEOF (.fmt_slot8)         );
  PROVIDE (_fmt_slot9_end            = ADDR (.fmt_slot9)         + SIZEOF (.fmt_slot9)         );
  PROVIDE (_fmt_slot10_end           = ADDR (.fmt_slot10)        + SIZEOF (.fmt_slot10)        );
  PROVIDE (_fmt_slot11_end           = ADDR (.fmt_slot11)        + SIZEOF (.fmt_slot11)        );
  PROVIDE (_fmt_slot12_end           = ADDR (.fmt_slot12)        + SIZEOF (.fmt_slot12)        );
  PROVIDE (_fmt_slot13_end           = ADDR (.fmt_slot13)        + SIZEOF (.fmt_slot13)        );
  PROVIDE (_fmt_slot14_end           = ADDR (.fmt_slot14)        + SIZEOF (.fmt_slot14)        );
  PROVIDE (_fmt_slot15_end           = ADDR (.fmt_slot15)        + SIZEOF (.fmt_slot15)        );

  OVERLAY 0x57f00000 :
    {
      .cmem       { *(.cmem)       }
      .cmem_alter { *(.cmem_alter) }
    }

  OVERLAY 0x57f08000 :
    {
      .cmem_shared       { *(.cmem_shared)       }
      .cmem_shared_alter { *(.cmem_shared_alter) }
    }

  .fmt_slot0  0x58000000 : { *(.fmt_slot0)  }
  .fmt_slot1  0x58800000 : { *(.fmt_slot1)  }
  .fmt_slot2  0x59000000 : { *(.fmt_slot2)  }
  .fmt_slot3  0x59800000 : { *(.fmt_slot3)  }
  .fmt_slot4  0x5a000000 : { *(.fmt_slot4)  }
  .fmt_slot5  0x5a800000 : { *(.fmt_slot5)  }
  .fmt_slot6  0x5b000000 : { *(.fmt_slot6)  }
  .fmt_slot7  0x5b800000 : { *(.fmt_slot7)  }
  .fmt_slot8  0x5c000000 : { *(.fmt_slot8)  }
  .fmt_slot9  0x5c800000 : { *(.fmt_slot9)  }
  .fmt_slot10 0x5d000000 : { *(.fmt_slot10) }
  .fmt_slot11 0x5d800000 : { *(.fmt_slot11) }
  .fmt_slot12 0x5e000000 : { *(.fmt_slot12) }
  .fmt_slot13 0x5e800000 : { *(.fmt_slot13) }
  .fmt_slot14 0x5f000000 : { *(.fmt_slot14) }
  .fmt_slot15 0x5f800000 : { *(.fmt_slot15) }

  /* End of nps specific sections and symbols.  */

  ${OTHER_SECTIONS}"
fi
