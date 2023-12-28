/* SME Extension (ZERO).  */

/* An all-zeros immediate is disassembled as an empty list { }.  */
zero { }

/* An all-ones immediate is disassembled as {ZA}.  */
zero { za }
zero { za0.b }
zero { za0.h, za1.h }
zero { za0.d, za1.d, za2.d, za3.d, za4.d, za5.d, za6.d, za7.d }
zero { za7.d, za6.d, za5.d, za4.d, za3.d, za2.d, za1.d, za0.d }

/* Set each bit individually.  */
zero { za0.d }
zero { za1.d }
zero { za2.d }
zero { za3.d }
zero { za4.d }
zero { za5.d }
zero { za6.d }
zero { za7.d }

/* Random bits.  */
zero { za0.d }
zero { za0.d, za1.d }
zero { za0.d, za1.d, za2.d }
zero { za0.d, za1.d, za2.d, za3.d }
zero { za0.d, za1.d, za2.d, za3.d, za4.d }
zero { za0.d, za1.d, za2.d, za3.d, za4.d, za5.d }
zero { za0.d, za1.d, za2.d, za3.d, za4.d, za5.d, za6.d }
zero { za0.d, za1.d, za2.d, za3.d, za4.d, za5.d, za6.d, za7.d }

zero { za7.d }
zero { za7.d, za6.d }
zero { za7.d, za6.d, za5.d }
zero { za7.d, za6.d, za5.d, za4.d }
zero { za7.d, za6.d, za5.d, za4.d, za3.d }
zero { za7.d, za6.d, za5.d, za4.d, za3.d, za2.d }
zero { za7.d, za6.d, za5.d, za4.d, za3.d, za2.d, za1.d }
zero { za7.d, za6.d, za5.d, za4.d, za3.d, za2.d, za1.d, za0.d }

zero { za1.d, za2.d, za3.d, za4.d, za5.d, za6.d, za7.d }
zero { za0.d, za2.d, za3.d, za4.d, za5.d, za6.d, za7.d }
zero { za0.d, za1.d, za3.d, za4.d, za5.d, za6.d, za7.d }
zero { za0.d, za1.d, za2.d, za4.d, za5.d, za6.d, za7.d }
zero { za0.d, za1.d, za2.d, za3.d, za5.d, za6.d, za7.d }
zero { za0.d, za1.d, za2.d, za3.d, za4.d, za6.d, za7.d }
zero { za0.d, za1.d, za2.d, za3.d, za4.d, za5.d, za7.d }
zero { za0.d, za1.d, za2.d, za3.d, za4.d, za5.d, za6.d }

/* For programmer convenience an assembler must also accept the names of
   32-bit, 16-bit and 8-bit element tiles.
*/
zero { za0.h }
zero { za1.h }
zero { za0.s }
zero { za1.s }
zero { za2.s }
zero { za3.s }

/* The preferred disassembly of this instruction uses the shortest list of tile
   names that represent the encoded immediate mask.
*/

/* To za0.h  */
zero { za0.d, za2.d, za4.d, za6.d }
zero { za0.s, za2.s }
zero { za0.h }

/* To za1.h  */
zero { za1.d, za3.d, za5.d, za7.d }
zero { za1.s, za3.s }
zero { za1.h }

/* To za[0-3].s  */
zero { za0.d, za4.d }
zero { za1.d, za5.d }
zero { za2.d, za6.d }
zero { za3.d, za7.d }

/* Mix of suffixed.  */
zero { za0.h, za7.d }
zero { za1.h, za0.d }
zero { za0.s, za2.d }
zero { za1.s, za3.d }
zero { za2.s, za4.d }
zero { za3.s, za5.d }

/* Register aliases.  */
foo .req za0
bar .req za2
baz .req za7

zero { foo.h, baz.d }
zero { za0.s, bar.d }
