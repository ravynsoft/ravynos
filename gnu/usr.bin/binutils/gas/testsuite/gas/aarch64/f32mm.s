/* The instructions with non-zero register numbers are there to ensure we have
   the correct argument positioning (i.e. check that the first argument is at
   the end of the word etc).
   The instructions with all-zero register numbers are to ensure the previous
   encoding didn't just "happen" to fit -- so that if we change the registers
   that changes the correct part of the word.
   Each of the numbered patterns begin and end with a 1, so we can replace
   them with all-zeros and see the entire range has changed. */

// SVE
fmmla	z17.s,  z21.s,  z27.s
fmmla	z0.s,  z0.s,  z0.s
