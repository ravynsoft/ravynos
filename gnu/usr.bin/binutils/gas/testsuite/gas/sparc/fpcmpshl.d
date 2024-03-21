#as: -Av9m8
#objdump: -dr
#name: FPCMPSHL OSA2017 instructions

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	83 b0 72 00 	fpcmpule8shl  %f32, %f48, 0, %g1
   4:	85 b0 f2 23 	fpcmpugt8shl  %f34, %f50, 1, %g2
   8:	87 b1 72 54 	fpcmpeq8shl  %f36, %f52, 2, %g3
   c:	89 b1 f2 7f 	fpcmpne8shl  %f38, %f62, 3, %g4
  10:	83 b0 72 80 	fpcmpule16shl  %f32, %f48, 0, %g1
  14:	85 b0 f2 a3 	fpcmpugt16shl  %f34, %f50, 1, %g2
  18:	87 b1 72 d4 	fpcmpeq16shl  %f36, %f52, 2, %g3
  1c:	89 b1 f2 ff 	fpcmpne16shl  %f38, %f62, 3, %g4$
  20:	83 b0 73 00 	fpcmpule32shl  %f32, %f48, 0, %g1
  24:	85 b0 f3 23 	fpcmpugt32shl  %f34, %f50, 1, %g2
  28:	87 b1 73 54 	fpcmpeq32shl  %f36, %f52, 2, %g3
  2c:	89 b1 f3 7f 	fpcmpne32shl  %f38, %f62, 3, %g4
  30:	83 b0 48 a0 	fpcmpde8shl  %f32, %f48, 0, %g1
  34:	85 b0 c8 e3 	fpcmpde16shl  %f34, %f50, 1, %g2
  38:	87 b1 49 54 	fpcmpde32shl  %f36, %f52, 2, %g3
  3c:	83 b0 73 80 	fpcmpur8shl  %f32, %f48, 0, %g1
  40:	85 b0 f3 a3 	fpcmpur16shl  %f34, %f50, 1, %g2
  44:	87 b1 73 d4 	fpcmpur32shl  %f36, %f52, 2, %g3
