#as: -march=armv8.2-a+simd+fp16
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   [0-9a-f]+:	4e63c441 	fmaxnm	v1.2d, v2.2d, v3.2d
   [0-9a-f]+:	0e23c441 	fmaxnm	v1.2s, v2.2s, v3.2s
   [0-9a-f]+:	4e23c441 	fmaxnm	v1.4s, v2.4s, v3.4s
   [0-9a-f]+:	0e400400 	fmaxnm	v0.4h, v0.4h, v0.4h
  [0-9a-f]+:	0e430441 	fmaxnm	v1.4h, v2.4h, v3.4h
  [0-9a-f]+:	4e400400 	fmaxnm	v0.8h, v0.8h, v0.8h
  [0-9a-f]+:	4e430441 	fmaxnm	v1.8h, v2.8h, v3.8h
  [0-9a-f]+:	6e63c441 	fmaxnmp	v1.2d, v2.2d, v3.2d
  [0-9a-f]+:	2e23c441 	fmaxnmp	v1.2s, v2.2s, v3.2s
  [0-9a-f]+:	6e23c441 	fmaxnmp	v1.4s, v2.4s, v3.4s
  [0-9a-f]+:	2e400400 	fmaxnmp	v0.4h, v0.4h, v0.4h
  [0-9a-f]+:	2e430441 	fmaxnmp	v1.4h, v2.4h, v3.4h
  [0-9a-f]+:	6e400400 	fmaxnmp	v0.8h, v0.8h, v0.8h
  [0-9a-f]+:	6e430441 	fmaxnmp	v1.8h, v2.8h, v3.8h
  [0-9a-f]+:	4ee3c441 	fminnm	v1.2d, v2.2d, v3.2d
  [0-9a-f]+:	0ea3c441 	fminnm	v1.2s, v2.2s, v3.2s
  [0-9a-f]+:	4ea3c441 	fminnm	v1.4s, v2.4s, v3.4s
  [0-9a-f]+:	0ec00400 	fminnm	v0.4h, v0.4h, v0.4h
  [0-9a-f]+:	0ec30441 	fminnm	v1.4h, v2.4h, v3.4h
  [0-9a-f]+:	4ec00400 	fminnm	v0.8h, v0.8h, v0.8h
  [0-9a-f]+:	4ec30441 	fminnm	v1.8h, v2.8h, v3.8h
  [0-9a-f]+:	6ee3c441 	fminnmp	v1.2d, v2.2d, v3.2d
  [0-9a-f]+:	2ea3c441 	fminnmp	v1.2s, v2.2s, v3.2s
  [0-9a-f]+:	6ea3c441 	fminnmp	v1.4s, v2.4s, v3.4s
  [0-9a-f]+:	2ec00400 	fminnmp	v0.4h, v0.4h, v0.4h
  [0-9a-f]+:	2ec30441 	fminnmp	v1.4h, v2.4h, v3.4h
  [0-9a-f]+:	6ec00400 	fminnmp	v0.8h, v0.8h, v0.8h
  [0-9a-f]+:	6ec30441 	fminnmp	v1.8h, v2.8h, v3.8h
  [0-9a-f]+:	4e63cc41 	fmla	v1.2d, v2.2d, v3.2d
  [0-9a-f]+:	0e23cc41 	fmla	v1.2s, v2.2s, v3.2s
  [0-9a-f]+:	4e23cc41 	fmla	v1.4s, v2.4s, v3.4s
  [0-9a-f]+:	0e400c00 	fmla	v0.4h, v0.4h, v0.4h
  [0-9a-f]+:	0e430c41 	fmla	v1.4h, v2.4h, v3.4h
  [0-9a-f]+:	4e400c00 	fmla	v0.8h, v0.8h, v0.8h
  [0-9a-f]+:	4e430c41 	fmla	v1.8h, v2.8h, v3.8h
  [0-9a-f]+:	4ee3cc41 	fmls	v1.2d, v2.2d, v3.2d
  [0-9a-f]+:	0ea3cc41 	fmls	v1.2s, v2.2s, v3.2s
  [0-9a-f]+:	4ea3cc41 	fmls	v1.4s, v2.4s, v3.4s
  [0-9a-f]+:	0ec00c00 	fmls	v0.4h, v0.4h, v0.4h
  [0-9a-f]+:	0ec30c41 	fmls	v1.4h, v2.4h, v3.4h
  [0-9a-f]+:	4ec00c00 	fmls	v0.8h, v0.8h, v0.8h
  [0-9a-f]+:	4ec30c41 	fmls	v1.8h, v2.8h, v3.8h
  [0-9a-f]+:	4e63d441 	fadd	v1.2d, v2.2d, v3.2d
  [0-9a-f]+:	0e23d441 	fadd	v1.2s, v2.2s, v3.2s
  [0-9a-f]+:	4e23d441 	fadd	v1.4s, v2.4s, v3.4s
  [0-9a-f]+:	0e401400 	fadd	v0.4h, v0.4h, v0.4h
  [0-9a-f]+:	0e431441 	fadd	v1.4h, v2.4h, v3.4h
  [0-9a-f]+:	4e401400 	fadd	v0.8h, v0.8h, v0.8h
  [0-9a-f]+:	4e431441 	fadd	v1.8h, v2.8h, v3.8h
  [0-9a-f]+:	6e63d441 	faddp	v1.2d, v2.2d, v3.2d
  [0-9a-f]+:	2e23d441 	faddp	v1.2s, v2.2s, v3.2s
  [0-9a-f]+:	6e23d441 	faddp	v1.4s, v2.4s, v3.4s
  [0-9a-f]+:	2e401400 	faddp	v0.4h, v0.4h, v0.4h
  [0-9a-f]+:	2e431441 	faddp	v1.4h, v2.4h, v3.4h
  [0-9a-f]+:	6e401400 	faddp	v0.8h, v0.8h, v0.8h
  [0-9a-f]+:	6e431441 	faddp	v1.8h, v2.8h, v3.8h
  [0-9a-f]+:	4ee3d441 	fsub	v1.2d, v2.2d, v3.2d
  [0-9a-f]+:	0ea3d441 	fsub	v1.2s, v2.2s, v3.2s
  [0-9a-f]+:	4ea3d441 	fsub	v1.4s, v2.4s, v3.4s
  [0-9a-f]+:	0ec01400 	fsub	v0.4h, v0.4h, v0.4h
  [0-9a-f]+:	0ec31441 	fsub	v1.4h, v2.4h, v3.4h
  [0-9a-f]+:	4ec01400 	fsub	v0.8h, v0.8h, v0.8h
  [0-9a-f]+:	4ec31441 	fsub	v1.8h, v2.8h, v3.8h
  [0-9a-f]+:	4e63dc41 	fmulx	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	0e23dc41 	fmulx	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	4e23dc41 	fmulx	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	0e401c00 	fmulx	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	0e431c41 	fmulx	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	4e401c00 	fmulx	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	4e431c41 	fmulx	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	6e63dc41 	fmul	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	2e23dc41 	fmul	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	6e23dc41 	fmul	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	2e401c00 	fmul	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	2e431c41 	fmul	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	6e401c00 	fmul	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	6e431c41 	fmul	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	4e63e441 	fcmeq	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	0e23e441 	fcmeq	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	4e23e441 	fcmeq	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	0e402400 	fcmeq	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	0e432441 	fcmeq	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	4e402400 	fcmeq	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	4e432441 	fcmeq	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	6e63e441 	fcmge	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	2e23e441 	fcmge	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	6e23e441 	fcmge	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	2e402400 	fcmge	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	2e432441 	fcmge	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	6e402400 	fcmge	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	6e432441 	fcmge	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	6ee3e441 	fcmgt	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	2ea3e441 	fcmgt	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	6ea3e441 	fcmgt	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	2ec02400 	fcmgt	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	2ec32441 	fcmgt	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	6ec02400 	fcmgt	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	6ec32441 	fcmgt	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	6e63ec41 	facge	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	2e23ec41 	facge	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	6e23ec41 	facge	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	2e402c00 	facge	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	2e432c41 	facge	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	6e402c00 	facge	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	6e432c41 	facge	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	6ee3ec41 	facgt	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	2ea3ec41 	facgt	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	6ea3ec41 	facgt	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	2ec02c00 	facgt	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	2ec32c41 	facgt	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	6ec02c00 	facgt	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	6ec32c41 	facgt	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	4e63f441 	fmax	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	0e23f441 	fmax	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	4e23f441 	fmax	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	0e403400 	fmax	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	0e433441 	fmax	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	4e403400 	fmax	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	4e433441 	fmax	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	6e63f441 	fmaxp	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	2e23f441 	fmaxp	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	6e23f441 	fmaxp	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	2e403400 	fmaxp	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	2e433441 	fmaxp	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	6e403400 	fmaxp	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	6e433441 	fmaxp	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	4ee3f441 	fmin	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	0ea3f441 	fmin	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	4ea3f441 	fmin	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	0ec03400 	fmin	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	0ec33441 	fmin	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	4ec03400 	fmin	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	4ec33441 	fmin	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	6ee3f441 	fminp	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	2ea3f441 	fminp	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	6ea3f441 	fminp	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	2ec03400 	fminp	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	2ec33441 	fminp	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	6ec03400 	fminp	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	6ec33441 	fminp	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	4e63fc41 	frecps	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	0e23fc41 	frecps	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	4e23fc41 	frecps	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	0e403c00 	frecps	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	0e433c41 	frecps	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	4e403c00 	frecps	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	4e433c41 	frecps	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	6e63fc41 	fdiv	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	2e23fc41 	fdiv	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	6e23fc41 	fdiv	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	2e403c00 	fdiv	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	2e433c41 	fdiv	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	6e403c00 	fdiv	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	6e433c41 	fdiv	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	4ee3fc41 	frsqrts	v1.2d, v2.2d, v3.2d
 [0-9a-f]+:	0ea3fc41 	frsqrts	v1.2s, v2.2s, v3.2s
 [0-9a-f]+:	4ea3fc41 	frsqrts	v1.4s, v2.4s, v3.4s
 [0-9a-f]+:	0ec03c00 	frsqrts	v0.4h, v0.4h, v0.4h
 [0-9a-f]+:	0ec33c41 	frsqrts	v1.4h, v2.4h, v3.4h
 [0-9a-f]+:	4ec03c00 	frsqrts	v0.8h, v0.8h, v0.8h
 [0-9a-f]+:	4ec33c41 	frsqrts	v1.8h, v2.8h, v3.8h
 [0-9a-f]+:	7ee2d420 	fabd	d0, d1, d2
 [0-9a-f]+:	7ea2d420 	fabd	s0, s1, s2
 [0-9a-f]+:	7ec21420 	fabd	h0, h1, h2
 [0-9a-f]+:	7ec01400 	fabd	h0, h0, h0
 [0-9a-f]+:	5e62dc20 	fmulx	d0, d1, d2
 [0-9a-f]+:	5e22dc20 	fmulx	s0, s1, s2
 [0-9a-f]+:	5e421c20 	fmulx	h0, h1, h2
 [0-9a-f]+:	5e401c00 	fmulx	h0, h0, h0
 [0-9a-f]+:	5e62e420 	fcmeq	d0, d1, d2
 [0-9a-f]+:	5e22e420 	fcmeq	s0, s1, s2
 [0-9a-f]+:	5e422420 	fcmeq	h0, h1, h2
 [0-9a-f]+:	5e402400 	fcmeq	h0, h0, h0
 [0-9a-f]+:	7ee2e420 	fcmgt	d0, d1, d2
 [0-9a-f]+:	7ea2e420 	fcmgt	s0, s1, s2
 [0-9a-f]+:	7ec22420 	fcmgt	h0, h1, h2
 [0-9a-f]+:	7ec02400 	fcmgt	h0, h0, h0
 [0-9a-f]+:	7e62e420 	fcmge	d0, d1, d2
 [0-9a-f]+:	7e22e420 	fcmge	s0, s1, s2
 [0-9a-f]+:	7e422420 	fcmge	h0, h1, h2
 [0-9a-f]+:	7e402400 	fcmge	h0, h0, h0
 [0-9a-f]+:	7e62ec20 	facge	d0, d1, d2
 [0-9a-f]+:	7e22ec20 	facge	s0, s1, s2
 [0-9a-f]+:	7e422c20 	facge	h0, h1, h2
 [0-9a-f]+:	7e402c00 	facge	h0, h0, h0
 [0-9a-f]+:	7ee2ec20 	facgt	d0, d1, d2
 [0-9a-f]+:	7ea2ec20 	facgt	s0, s1, s2
 [0-9a-f]+:	7ec22c20 	facgt	h0, h1, h2
 [0-9a-f]+:	7ec02c00 	facgt	h0, h0, h0
 [0-9a-f]+:	5e62fc20 	frecps	d0, d1, d2
 [0-9a-f]+:	5e22fc20 	frecps	s0, s1, s2
 [0-9a-f]+:	5e423c20 	frecps	h0, h1, h2
 [0-9a-f]+:	5e403c00 	frecps	h0, h0, h0
 [0-9a-f]+:	5ee2fc20 	frsqrts	d0, d1, d2
 [0-9a-f]+:	5ea2fc20 	frsqrts	s0, s1, s2
 [0-9a-f]+:	5ec23c20 	frsqrts	h0, h1, h2
 [0-9a-f]+:	5ec03c00 	frsqrts	h0, h0, h0
 [0-9a-f]+:	4ee0c820 	fcmgt	v0.2d, v1.2d, #0.0
 [0-9a-f]+:	0ea0c820 	fcmgt	v0.2s, v1.2s, #0.0
 [0-9a-f]+:	4ea0c820 	fcmgt	v0.4s, v1.4s, #0.0
 [0-9a-f]+:	0ef8c820 	fcmgt	v0.4h, v1.4h, #0.0
 [0-9a-f]+:	4ef8c820 	fcmgt	v0.8h, v1.8h, #0.0
 [0-9a-f]+:	6ee0c820 	fcmge	v0.2d, v1.2d, #0.0
 [0-9a-f]+:	2ea0c820 	fcmge	v0.2s, v1.2s, #0.0
 [0-9a-f]+:	6ea0c820 	fcmge	v0.4s, v1.4s, #0.0
 [0-9a-f]+:	2ef8c820 	fcmge	v0.4h, v1.4h, #0.0
 [0-9a-f]+:	6ef8c820 	fcmge	v0.8h, v1.8h, #0.0
 [0-9a-f]+:	4ee0d820 	fcmeq	v0.2d, v1.2d, #0.0
 [0-9a-f]+:	0ea0d820 	fcmeq	v0.2s, v1.2s, #0.0
 [0-9a-f]+:	4ea0d820 	fcmeq	v0.4s, v1.4s, #0.0
 [0-9a-f]+:	0ef8d820 	fcmeq	v0.4h, v1.4h, #0.0
 [0-9a-f]+:	4ef8d820 	fcmeq	v0.8h, v1.8h, #0.0
 [0-9a-f]+:	6ee0d820 	fcmle	v0.2d, v1.2d, #0.0
 [0-9a-f]+:	2ea0d820 	fcmle	v0.2s, v1.2s, #0.0
 [0-9a-f]+:	6ea0d820 	fcmle	v0.4s, v1.4s, #0.0
 [0-9a-f]+:	2ef8d820 	fcmle	v0.4h, v1.4h, #0.0
 [0-9a-f]+:	6ef8d820 	fcmle	v0.8h, v1.8h, #0.0
 [0-9a-f]+:	4ee0e820 	fcmlt	v0.2d, v1.2d, #0.0
 [0-9a-f]+:	0ea0e820 	fcmlt	v0.2s, v1.2s, #0.0
 [0-9a-f]+:	4ea0e820 	fcmlt	v0.4s, v1.4s, #0.0
 [0-9a-f]+:	0ef8e820 	fcmlt	v0.4h, v1.4h, #0.0
 [0-9a-f]+:	4ef8e820 	fcmlt	v0.8h, v1.8h, #0.0
 [0-9a-f]+:	4ee0f820 	fabs	v0.2d, v1.2d
 [0-9a-f]+:	0ea0f820 	fabs	v0.2s, v1.2s
 [0-9a-f]+:	4ea0f820 	fabs	v0.4s, v1.4s
 [0-9a-f]+:	0ef8f820 	fabs	v0.4h, v1.4h
 [0-9a-f]+:	4ef8f820 	fabs	v0.8h, v1.8h
 [0-9a-f]+:	6ee0f820 	fneg	v0.2d, v1.2d
 [0-9a-f]+:	2ea0f820 	fneg	v0.2s, v1.2s
 [0-9a-f]+:	6ea0f820 	fneg	v0.4s, v1.4s
 [0-9a-f]+:	2ef8f820 	fneg	v0.4h, v1.4h
 [0-9a-f]+:	6ef8f820 	fneg	v0.8h, v1.8h
 [0-9a-f]+:	4e618820 	frintn	v0.2d, v1.2d
 [0-9a-f]+:	0e218820 	frintn	v0.2s, v1.2s
 [0-9a-f]+:	4e218820 	frintn	v0.4s, v1.4s
 [0-9a-f]+:	0e798820 	frintn	v0.4h, v1.4h
 [0-9a-f]+:	4e798820 	frintn	v0.8h, v1.8h
 [0-9a-f]+:	6e618820 	frinta	v0.2d, v1.2d
 [0-9a-f]+:	2e218820 	frinta	v0.2s, v1.2s
 [0-9a-f]+:	6e218820 	frinta	v0.4s, v1.4s
 [0-9a-f]+:	2e798820 	frinta	v0.4h, v1.4h
 [0-9a-f]+:	6e798820 	frinta	v0.8h, v1.8h
 [0-9a-f]+:	4ee18820 	frintp	v0.2d, v1.2d
 [0-9a-f]+:	0ea18820 	frintp	v0.2s, v1.2s
 [0-9a-f]+:	4ea18820 	frintp	v0.4s, v1.4s
 [0-9a-f]+:	0ef98820 	frintp	v0.4h, v1.4h
 [0-9a-f]+:	4ef98820 	frintp	v0.8h, v1.8h
 [0-9a-f]+:	4e619820 	frintm	v0.2d, v1.2d
 [0-9a-f]+:	0e219820 	frintm	v0.2s, v1.2s
 [0-9a-f]+:	4e219820 	frintm	v0.4s, v1.4s
 [0-9a-f]+:	0e799820 	frintm	v0.4h, v1.4h
 [0-9a-f]+:	4e799820 	frintm	v0.8h, v1.8h
 [0-9a-f]+:	6e619820 	frintx	v0.2d, v1.2d
 [0-9a-f]+:	2e219820 	frintx	v0.2s, v1.2s
 [0-9a-f]+:	6e219820 	frintx	v0.4s, v1.4s
 [0-9a-f]+:	2e799820 	frintx	v0.4h, v1.4h
 [0-9a-f]+:	6e799820 	frintx	v0.8h, v1.8h
 [0-9a-f]+:	4ee19820 	frintz	v0.2d, v1.2d
 [0-9a-f]+:	0ea19820 	frintz	v0.2s, v1.2s
 [0-9a-f]+:	4ea19820 	frintz	v0.4s, v1.4s
 [0-9a-f]+:	0ef99820 	frintz	v0.4h, v1.4h
 [0-9a-f]+:	4ef99820 	frintz	v0.8h, v1.8h
 [0-9a-f]+:	6ee19820 	frinti	v0.2d, v1.2d
 [0-9a-f]+:	2ea19820 	frinti	v0.2s, v1.2s
 [0-9a-f]+:	6ea19820 	frinti	v0.4s, v1.4s
 [0-9a-f]+:	2ef99820 	frinti	v0.4h, v1.4h
 [0-9a-f]+:	6ef99820 	frinti	v0.8h, v1.8h
 [0-9a-f]+:	4e61a820 	fcvtns	v0.2d, v1.2d
 [0-9a-f]+:	0e21a820 	fcvtns	v0.2s, v1.2s
 [0-9a-f]+:	4e21a820 	fcvtns	v0.4s, v1.4s
 [0-9a-f]+:	0e79a820 	fcvtns	v0.4h, v1.4h
 [0-9a-f]+:	4e79a820 	fcvtns	v0.8h, v1.8h
 [0-9a-f]+:	6e61a820 	fcvtnu	v0.2d, v1.2d
 [0-9a-f]+:	2e21a820 	fcvtnu	v0.2s, v1.2s
 [0-9a-f]+:	6e21a820 	fcvtnu	v0.4s, v1.4s
 [0-9a-f]+:	2e79a820 	fcvtnu	v0.4h, v1.4h
 [0-9a-f]+:	6e79a820 	fcvtnu	v0.8h, v1.8h
 [0-9a-f]+:	4ee1a820 	fcvtps	v0.2d, v1.2d
 [0-9a-f]+:	0ea1a820 	fcvtps	v0.2s, v1.2s
 [0-9a-f]+:	4ea1a820 	fcvtps	v0.4s, v1.4s
 [0-9a-f]+:	0ef9a820 	fcvtps	v0.4h, v1.4h
 [0-9a-f]+:	4ef9a820 	fcvtps	v0.8h, v1.8h
 [0-9a-f]+:	6ee1a820 	fcvtpu	v0.2d, v1.2d
 [0-9a-f]+:	2ea1a820 	fcvtpu	v0.2s, v1.2s
 [0-9a-f]+:	6ea1a820 	fcvtpu	v0.4s, v1.4s
 [0-9a-f]+:	2ef9a820 	fcvtpu	v0.4h, v1.4h
 [0-9a-f]+:	6ef9a820 	fcvtpu	v0.8h, v1.8h
 [0-9a-f]+:	4e61b820 	fcvtms	v0.2d, v1.2d
 [0-9a-f]+:	0e21b820 	fcvtms	v0.2s, v1.2s
 [0-9a-f]+:	4e21b820 	fcvtms	v0.4s, v1.4s
 [0-9a-f]+:	0e79b820 	fcvtms	v0.4h, v1.4h
 [0-9a-f]+:	4e79b820 	fcvtms	v0.8h, v1.8h
 [0-9a-f]+:	6e61b820 	fcvtmu	v0.2d, v1.2d
 [0-9a-f]+:	2e21b820 	fcvtmu	v0.2s, v1.2s
 [0-9a-f]+:	6e21b820 	fcvtmu	v0.4s, v1.4s
 [0-9a-f]+:	2e79b820 	fcvtmu	v0.4h, v1.4h
 [0-9a-f]+:	6e79b820 	fcvtmu	v0.8h, v1.8h
 [0-9a-f]+:	4ee1b820 	fcvtzs	v0.2d, v1.2d
 [0-9a-f]+:	0ea1b820 	fcvtzs	v0.2s, v1.2s
 [0-9a-f]+:	4ea1b820 	fcvtzs	v0.4s, v1.4s
 [0-9a-f]+:	0ef9b820 	fcvtzs	v0.4h, v1.4h
 [0-9a-f]+:	4ef9b820 	fcvtzs	v0.8h, v1.8h
 [0-9a-f]+:	6ee1b820 	fcvtzu	v0.2d, v1.2d
 [0-9a-f]+:	2ea1b820 	fcvtzu	v0.2s, v1.2s
 [0-9a-f]+:	6ea1b820 	fcvtzu	v0.4s, v1.4s
 [0-9a-f]+:	2ef9b820 	fcvtzu	v0.4h, v1.4h
 [0-9a-f]+:	6ef9b820 	fcvtzu	v0.8h, v1.8h
 [0-9a-f]+:	4e61c820 	fcvtas	v0.2d, v1.2d
 [0-9a-f]+:	0e21c820 	fcvtas	v0.2s, v1.2s
 [0-9a-f]+:	4e21c820 	fcvtas	v0.4s, v1.4s
 [0-9a-f]+:	0e79c820 	fcvtas	v0.4h, v1.4h
 [0-9a-f]+:	4e79c820 	fcvtas	v0.8h, v1.8h
 [0-9a-f]+:	6e61c820 	fcvtau	v0.2d, v1.2d
 [0-9a-f]+:	2e21c820 	fcvtau	v0.2s, v1.2s
 [0-9a-f]+:	6e21c820 	fcvtau	v0.4s, v1.4s
 [0-9a-f]+:	2e79c820 	fcvtau	v0.4h, v1.4h
 [0-9a-f]+:	6e79c820 	fcvtau	v0.8h, v1.8h
 [0-9a-f]+:	4e61d820 	scvtf	v0.2d, v1.2d
 [0-9a-f]+:	0e21d820 	scvtf	v0.2s, v1.2s
 [0-9a-f]+:	4e21d820 	scvtf	v0.4s, v1.4s
 [0-9a-f]+:	0e79d820 	scvtf	v0.4h, v1.4h
 [0-9a-f]+:	4e79d820 	scvtf	v0.8h, v1.8h
 [0-9a-f]+:	6e61d820 	ucvtf	v0.2d, v1.2d
 [0-9a-f]+:	2e21d820 	ucvtf	v0.2s, v1.2s
 [0-9a-f]+:	6e21d820 	ucvtf	v0.4s, v1.4s
 [0-9a-f]+:	2e79d820 	ucvtf	v0.4h, v1.4h
 [0-9a-f]+:	6e79d820 	ucvtf	v0.8h, v1.8h
 [0-9a-f]+:	4ee1d820 	frecpe	v0.2d, v1.2d
 [0-9a-f]+:	0ea1d820 	frecpe	v0.2s, v1.2s
 [0-9a-f]+:	4ea1d820 	frecpe	v0.4s, v1.4s
 [0-9a-f]+:	0ef9d820 	frecpe	v0.4h, v1.4h
 [0-9a-f]+:	4ef9d820 	frecpe	v0.8h, v1.8h
 [0-9a-f]+:	6ee1d820 	frsqrte	v0.2d, v1.2d
 [0-9a-f]+:	2ea1d820 	frsqrte	v0.2s, v1.2s
 [0-9a-f]+:	6ea1d820 	frsqrte	v0.4s, v1.4s
 [0-9a-f]+:	2ef9d820 	frsqrte	v0.4h, v1.4h
 [0-9a-f]+:	6ef9d820 	frsqrte	v0.8h, v1.8h
 [0-9a-f]+:	6ee1f820 	fsqrt	v0.2d, v1.2d
 [0-9a-f]+:	2ea1f820 	fsqrt	v0.2s, v1.2s
 [0-9a-f]+:	6ea1f820 	fsqrt	v0.4s, v1.4s
 [0-9a-f]+:	2ef9f820 	fsqrt	v0.4h, v1.4h
 [0-9a-f]+:	6ef9f820 	fsqrt	v0.8h, v1.8h
 [0-9a-f]+:	5ee0c820 	fcmgt	d0, d1, #0.0
 [0-9a-f]+:	5ea0c820 	fcmgt	s0, s1, #0.0
 [0-9a-f]+:	5ef8c820 	fcmgt	h0, h1, #0.0
 [0-9a-f]+:	5ef8c800 	fcmgt	h0, h0, #0.0
 [0-9a-f]+:	7ee0c820 	fcmge	d0, d1, #0.0
 [0-9a-f]+:	7ea0c820 	fcmge	s0, s1, #0.0
 [0-9a-f]+:	7ef8c820 	fcmge	h0, h1, #0.0
 [0-9a-f]+:	7ef8c800 	fcmge	h0, h0, #0.0
 [0-9a-f]+:	5ee0d820 	fcmeq	d0, d1, #0.0
 [0-9a-f]+:	5ea0d820 	fcmeq	s0, s1, #0.0
 [0-9a-f]+:	5ef8d820 	fcmeq	h0, h1, #0.0
 [0-9a-f]+:	5ef8d800 	fcmeq	h0, h0, #0.0
 [0-9a-f]+:	7ee0d820 	fcmle	d0, d1, #0.0
 [0-9a-f]+:	7ea0d820 	fcmle	s0, s1, #0.0
 [0-9a-f]+:	7ef8d820 	fcmle	h0, h1, #0.0
 [0-9a-f]+:	7ef8d800 	fcmle	h0, h0, #0.0
 [0-9a-f]+:	5ee0e820 	fcmlt	d0, d1, #0.0
 [0-9a-f]+:	5ea0e820 	fcmlt	s0, s1, #0.0
 [0-9a-f]+:	5ef8e820 	fcmlt	h0, h1, #0.0
 [0-9a-f]+:	5ef8e800 	fcmlt	h0, h0, #0.0
 [0-9a-f]+:	5e61a820 	fcvtns	d0, d1
 [0-9a-f]+:	5e21a820 	fcvtns	s0, s1
 [0-9a-f]+:	5e79a820 	fcvtns	h0, h1
 [0-9a-f]+:	5e79a800 	fcvtns	h0, h0
 [0-9a-f]+:	7e61a820 	fcvtnu	d0, d1
 [0-9a-f]+:	7e21a820 	fcvtnu	s0, s1
 [0-9a-f]+:	7e79a820 	fcvtnu	h0, h1
 [0-9a-f]+:	7e79a800 	fcvtnu	h0, h0
 [0-9a-f]+:	5ee1a820 	fcvtps	d0, d1
 [0-9a-f]+:	5ea1a820 	fcvtps	s0, s1
 [0-9a-f]+:	5ef9a820 	fcvtps	h0, h1
 [0-9a-f]+:	5ef9a800 	fcvtps	h0, h0
 [0-9a-f]+:	7ee1a820 	fcvtpu	d0, d1
 [0-9a-f]+:	7ea1a820 	fcvtpu	s0, s1
 [0-9a-f]+:	7ef9a820 	fcvtpu	h0, h1
 [0-9a-f]+:	7ef9a800 	fcvtpu	h0, h0
 [0-9a-f]+:	5e61b820 	fcvtms	d0, d1
 [0-9a-f]+:	5e21b820 	fcvtms	s0, s1
 [0-9a-f]+:	5e79b820 	fcvtms	h0, h1
 [0-9a-f]+:	5e79b800 	fcvtms	h0, h0
 [0-9a-f]+:	7e61b820 	fcvtmu	d0, d1
 [0-9a-f]+:	7e21b820 	fcvtmu	s0, s1
 [0-9a-f]+:	7e79b820 	fcvtmu	h0, h1
 [0-9a-f]+:	7e79b800 	fcvtmu	h0, h0
 [0-9a-f]+:	5ee1b820 	fcvtzs	d0, d1
 [0-9a-f]+:	5ea1b820 	fcvtzs	s0, s1
 [0-9a-f]+:	5ef9b820 	fcvtzs	h0, h1
 [0-9a-f]+:	5ef9b800 	fcvtzs	h0, h0
 [0-9a-f]+:	7ee1b820 	fcvtzu	d0, d1
 [0-9a-f]+:	7ea1b820 	fcvtzu	s0, s1
 [0-9a-f]+:	7ef9b820 	fcvtzu	h0, h1
 [0-9a-f]+:	7ef9b800 	fcvtzu	h0, h0
 [0-9a-f]+:	5e61c820 	fcvtas	d0, d1
 [0-9a-f]+:	5e21c820 	fcvtas	s0, s1
 [0-9a-f]+:	5e79c820 	fcvtas	h0, h1
 [0-9a-f]+:	5e79c800 	fcvtas	h0, h0
 [0-9a-f]+:	7e61c820 	fcvtau	d0, d1
 [0-9a-f]+:	7e21c820 	fcvtau	s0, s1
 [0-9a-f]+:	7e79c820 	fcvtau	h0, h1
 [0-9a-f]+:	7e79c800 	fcvtau	h0, h0
 [0-9a-f]+:	5e61d820 	scvtf	d0, d1
 [0-9a-f]+:	5e21d820 	scvtf	s0, s1
 [0-9a-f]+:	5e79d820 	scvtf	h0, h1
 [0-9a-f]+:	5e79d800 	scvtf	h0, h0
 [0-9a-f]+:	7e61d820 	ucvtf	d0, d1
 [0-9a-f]+:	7e21d820 	ucvtf	s0, s1
 [0-9a-f]+:	7e79d820 	ucvtf	h0, h1
 [0-9a-f]+:	7e79d800 	ucvtf	h0, h0
 [0-9a-f]+:	5ee1d820 	frecpe	d0, d1
 [0-9a-f]+:	5ea1d820 	frecpe	s0, s1
 [0-9a-f]+:	5ef9d820 	frecpe	h0, h1
 [0-9a-f]+:	5ef9d800 	frecpe	h0, h0
 [0-9a-f]+:	7ee1d820 	frsqrte	d0, d1
 [0-9a-f]+:	7ea1d820 	frsqrte	s0, s1
 [0-9a-f]+:	7ef9d820 	frsqrte	h0, h1
 [0-9a-f]+:	7ef9d800 	frsqrte	h0, h0
 [0-9a-f]+:	5ee1f820 	frecpx	d0, d1
 [0-9a-f]+:	5ea1f820 	frecpx	s0, s1
 [0-9a-f]+:	5ef9f820 	frecpx	h0, h1
 [0-9a-f]+:	5ef9f800 	frecpx	h0, h0
 [0-9a-f]+:	4fc31841 	fmla	v1.2d, v2.2d, v3.d\[1\]
 [0-9a-f]+:	0f831841 	fmla	v1.2s, v2.2s, v3.s\[2\]
 [0-9a-f]+:	4fa31041 	fmla	v1.4s, v2.4s, v3.s\[1\]
 [0-9a-f]+:	0f001000 	fmla	v0.4h, v0.4h, v0.h\[0\]
 [0-9a-f]+:	0f031041 	fmla	v1.4h, v2.4h, v3.h\[0\]
 [0-9a-f]+:	4f001000 	fmla	v0.8h, v0.8h, v0.h\[0\]
 [0-9a-f]+:	4f031041 	fmla	v1.8h, v2.8h, v3.h\[0\]
 [0-9a-f]+:	4fca10a1 	fmla	v1.2d, v5.2d, v10.d\[0\]
 [0-9a-f]+:	0fab1808 	fmla	v8.2s, v0.2s, v11.s\[3\]
 [0-9a-f]+:	0f3f1920 	fmla	v0.4h, v9.4h, v15.h\[7\]
 [0-9a-f]+:	4fc35841 	fmls	v1.2d, v2.2d, v3.d\[1\]
 [0-9a-f]+:	0f835841 	fmls	v1.2s, v2.2s, v3.s\[2\]
 [0-9a-f]+:	4fa35041 	fmls	v1.4s, v2.4s, v3.s\[1\]
 [0-9a-f]+:	0f005000 	fmls	v0.4h, v0.4h, v0.h\[0\]
 [0-9a-f]+:	0f035041 	fmls	v1.4h, v2.4h, v3.h\[0\]
 [0-9a-f]+:	4f005000 	fmls	v0.8h, v0.8h, v0.h\[0\]
 [0-9a-f]+:	4f035041 	fmls	v1.8h, v2.8h, v3.h\[0\]
 [0-9a-f]+:	4fca50a1 	fmls	v1.2d, v5.2d, v10.d\[0\]
 [0-9a-f]+:	0fab5808 	fmls	v8.2s, v0.2s, v11.s\[3\]
 [0-9a-f]+:	0f3f5920 	fmls	v0.4h, v9.4h, v15.h\[7\]
 [0-9a-f]+:	4fc39841 	fmul	v1.2d, v2.2d, v3.d\[1\]
 [0-9a-f]+:	0f839841 	fmul	v1.2s, v2.2s, v3.s\[2\]
 [0-9a-f]+:	4fa39041 	fmul	v1.4s, v2.4s, v3.s\[1\]
 [0-9a-f]+:	0f009000 	fmul	v0.4h, v0.4h, v0.h\[0\]
 [0-9a-f]+:	0f039041 	fmul	v1.4h, v2.4h, v3.h\[0\]
 [0-9a-f]+:	4f009000 	fmul	v0.8h, v0.8h, v0.h\[0\]
 [0-9a-f]+:	4f039041 	fmul	v1.8h, v2.8h, v3.h\[0\]
 [0-9a-f]+:	4fca90a1 	fmul	v1.2d, v5.2d, v10.d\[0\]
 [0-9a-f]+:	0fab9808 	fmul	v8.2s, v0.2s, v11.s\[3\]
 [0-9a-f]+:	0f3f9920 	fmul	v0.4h, v9.4h, v15.h\[7\]
 [0-9a-f]+:	6fc39841 	fmulx	v1.2d, v2.2d, v3.d\[1\]
 [0-9a-f]+:	2f839841 	fmulx	v1.2s, v2.2s, v3.s\[2\]
 [0-9a-f]+:	6fa39041 	fmulx	v1.4s, v2.4s, v3.s\[1\]
 [0-9a-f]+:	2f009000 	fmulx	v0.4h, v0.4h, v0.h\[0\]
 [0-9a-f]+:	2f039041 	fmulx	v1.4h, v2.4h, v3.h\[0\]
 [0-9a-f]+:	6f009000 	fmulx	v0.8h, v0.8h, v0.h\[0\]
 [0-9a-f]+:	6f039041 	fmulx	v1.8h, v2.8h, v3.h\[0\]
 [0-9a-f]+:	6fca90a1 	fmulx	v1.2d, v5.2d, v10.d\[0\]
 [0-9a-f]+:	2fab9808 	fmulx	v8.2s, v0.2s, v11.s\[3\]
 [0-9a-f]+:	2f3f9920 	fmulx	v0.4h, v9.4h, v15.h\[7\]
 [0-9a-f]+:	5fc31841 	fmla	d1, d2, v3.d\[1\]
 [0-9a-f]+:	5fa31041 	fmla	s1, s2, v3.s\[1\]
 [0-9a-f]+:	5f131041 	fmla	h1, h2, v3.h\[1\]
 [0-9a-f]+:	5f001000 	fmla	h0, h0, v0.h\[0\]
 [0-9a-f]+:	5fc35841 	fmls	d1, d2, v3.d\[1\]
 [0-9a-f]+:	5fa35041 	fmls	s1, s2, v3.s\[1\]
 [0-9a-f]+:	5f135041 	fmls	h1, h2, v3.h\[1\]
 [0-9a-f]+:	5f005000 	fmls	h0, h0, v0.h\[0\]
 [0-9a-f]+:	5fc39841 	fmul	d1, d2, v3.d\[1\]
 [0-9a-f]+:	5fa39041 	fmul	s1, s2, v3.s\[1\]
 [0-9a-f]+:	5f139041 	fmul	h1, h2, v3.h\[1\]
 [0-9a-f]+:	5f009000 	fmul	h0, h0, v0.h\[0\]
 [0-9a-f]+:	7fc39841 	fmulx	d1, d2, v3.d\[1\]
 [0-9a-f]+:	7fa39041 	fmulx	s1, s2, v3.s\[1\]
 [0-9a-f]+:	7f139041 	fmulx	h1, h2, v3.h\[1\]
 [0-9a-f]+:	7f009000 	fmulx	h0, h0, v0.h\[0\]
 [0-9a-f]+:	6e30c841 	fmaxnmv	s1, v2.4s
 [0-9a-f]+:	0e30c841 	fmaxnmv	h1, v2.4h
 [0-9a-f]+:	4e30c841 	fmaxnmv	h1, v2.8h
 [0-9a-f]+:	0e30c800 	fmaxnmv	h0, v0.4h
 [0-9a-f]+:	4e30c800 	fmaxnmv	h0, v0.8h
 [0-9a-f]+:	6e30f841 	fmaxv	s1, v2.4s
 [0-9a-f]+:	0e30f841 	fmaxv	h1, v2.4h
 [0-9a-f]+:	4e30f841 	fmaxv	h1, v2.8h
 [0-9a-f]+:	0e30f800 	fmaxv	h0, v0.4h
 [0-9a-f]+:	4e30f800 	fmaxv	h0, v0.8h
 [0-9a-f]+:	6eb0c841 	fminnmv	s1, v2.4s
 [0-9a-f]+:	0eb0c841 	fminnmv	h1, v2.4h
 [0-9a-f]+:	4eb0c841 	fminnmv	h1, v2.8h
 [0-9a-f]+:	0eb0c800 	fminnmv	h0, v0.4h
 [0-9a-f]+:	4eb0c800 	fminnmv	h0, v0.8h
 [0-9a-f]+:	6eb0f841 	fminv	s1, v2.4s
 [0-9a-f]+:	0eb0f841 	fminv	h1, v2.4h
 [0-9a-f]+:	4eb0f841 	fminv	h1, v2.8h
 [0-9a-f]+:	0eb0f800 	fminv	h0, v0.4h
 [0-9a-f]+:	4eb0f800 	fminv	h0, v0.8h
 [0-9a-f]+:	6f00f401 	fmov	v1.2d, #2.000000000000000000e\+00
 [0-9a-f]+:	0f00f401 	fmov	v1.2s, #2.000000000000000000e\+00
 [0-9a-f]+:	4f00f401 	fmov	v1.4s, #2.000000000000000000e\+00
 [0-9a-f]+:	0f00fc01 	fmov	v1.4h, #2.000000000000000000e\+00
 [0-9a-f]+:	4f00fc01 	fmov	v1.8h, #2.000000000000000000e\+00
 [0-9a-f]+:	0f03fe00 	fmov	v0.4h, #1.000000000000000000e\+00
 [0-9a-f]+:	4f03fe00 	fmov	v0.8h, #1.000000000000000000e\+00
 [0-9a-f]+:	7e70c841 	fmaxnmp	d1, v2.2d
 [0-9a-f]+:	7e30c841 	fmaxnmp	s1, v2.2s
 [0-9a-f]+:	5e30c841 	fmaxnmp	h1, v2.2h
 [0-9a-f]+:	5e30c800 	fmaxnmp	h0, v0.2h
 [0-9a-f]+:	7e70d841 	faddp	d1, v2.2d
 [0-9a-f]+:	7e30d841 	faddp	s1, v2.2s
 [0-9a-f]+:	5e30d841 	faddp	h1, v2.2h
 [0-9a-f]+:	5e30d800 	faddp	h0, v0.2h
 [0-9a-f]+:	7e70f841 	fmaxp	d1, v2.2d
 [0-9a-f]+:	7e30f841 	fmaxp	s1, v2.2s
 [0-9a-f]+:	5e30f841 	fmaxp	h1, v2.2h
 [0-9a-f]+:	5e30f800 	fmaxp	h0, v0.2h
 [0-9a-f]+:	7ef0c841 	fminnmp	d1, v2.2d
 [0-9a-f]+:	7eb0c841 	fminnmp	s1, v2.2s
 [0-9a-f]+:	5eb0c841 	fminnmp	h1, v2.2h
 [0-9a-f]+:	5eb0c800 	fminnmp	h0, v0.2h
 [0-9a-f]+:	7ef0f841 	fminp	d1, v2.2d
 [0-9a-f]+:	7eb0f841 	fminp	s1, v2.2s
 [0-9a-f]+:	5eb0f841 	fminp	h1, v2.2h
 [0-9a-f]+:	5eb0f800 	fminp	h0, v0.2h
 [0-9a-f]+:	4f7de441 	scvtf	v1.2d, v2.2d, #3
 [0-9a-f]+:	0f3de441 	scvtf	v1.2s, v2.2s, #3
 [0-9a-f]+:	4f3de441 	scvtf	v1.4s, v2.4s, #3
 [0-9a-f]+:	0f1de441 	scvtf	v1.4h, v2.4h, #3
 [0-9a-f]+:	4f1de441 	scvtf	v1.8h, v2.8h, #3
 [0-9a-f]+:	0f1fe400 	scvtf	v0.4h, v0.4h, #1
 [0-9a-f]+:	4f1fe400 	scvtf	v0.8h, v0.8h, #1
 [0-9a-f]+:	4f7dfc41 	fcvtzs	v1.2d, v2.2d, #3
 [0-9a-f]+:	0f3dfc41 	fcvtzs	v1.2s, v2.2s, #3
 [0-9a-f]+:	4f3dfc41 	fcvtzs	v1.4s, v2.4s, #3
 [0-9a-f]+:	0f1dfc41 	fcvtzs	v1.4h, v2.4h, #3
 [0-9a-f]+:	4f1dfc41 	fcvtzs	v1.8h, v2.8h, #3
 [0-9a-f]+:	0f1ffc00 	fcvtzs	v0.4h, v0.4h, #1
 [0-9a-f]+:	4f1ffc00 	fcvtzs	v0.8h, v0.8h, #1
 [0-9a-f]+:	6f7de441 	ucvtf	v1.2d, v2.2d, #3
 [0-9a-f]+:	2f3de441 	ucvtf	v1.2s, v2.2s, #3
 [0-9a-f]+:	6f3de441 	ucvtf	v1.4s, v2.4s, #3
 [0-9a-f]+:	2f1de441 	ucvtf	v1.4h, v2.4h, #3
 [0-9a-f]+:	6f1de441 	ucvtf	v1.8h, v2.8h, #3
 [0-9a-f]+:	2f1fe400 	ucvtf	v0.4h, v0.4h, #1
 [0-9a-f]+:	6f1fe400 	ucvtf	v0.8h, v0.8h, #1
 [0-9a-f]+:	6f7dfc41 	fcvtzu	v1.2d, v2.2d, #3
 [0-9a-f]+:	2f3dfc41 	fcvtzu	v1.2s, v2.2s, #3
 [0-9a-f]+:	6f3dfc41 	fcvtzu	v1.4s, v2.4s, #3
 [0-9a-f]+:	2f1dfc41 	fcvtzu	v1.4h, v2.4h, #3
 [0-9a-f]+:	6f1dfc41 	fcvtzu	v1.8h, v2.8h, #3
 [0-9a-f]+:	2f1ffc00 	fcvtzu	v0.4h, v0.4h, #1
 [0-9a-f]+:	6f1ffc00 	fcvtzu	v0.8h, v0.8h, #1
 [0-9a-f]+:	5f7de441 	scvtf	d1, d2, #3
 [0-9a-f]+:	5f3de441 	scvtf	s1, s2, #3
 [0-9a-f]+:	5f1de441 	scvtf	h1, h2, #3
 [0-9a-f]+:	5f1fe400 	scvtf	h0, h0, #1
 [0-9a-f]+:	5f7dfc41 	fcvtzs	d1, d2, #3
 [0-9a-f]+:	5f3dfc41 	fcvtzs	s1, s2, #3
 [0-9a-f]+:	5f1dfc41 	fcvtzs	h1, h2, #3
 [0-9a-f]+:	5f1ffc00 	fcvtzs	h0, h0, #1
 [0-9a-f]+:	7f7de441 	ucvtf	d1, d2, #3
 [0-9a-f]+:	7f3de441 	ucvtf	s1, s2, #3
 [0-9a-f]+:	7f1de441 	ucvtf	h1, h2, #3
 [0-9a-f]+:	7f1fe400 	ucvtf	h0, h0, #1
 [0-9a-f]+:	7f7dfc41 	fcvtzu	d1, d2, #3
 [0-9a-f]+:	7f3dfc41 	fcvtzu	s1, s2, #3
 [0-9a-f]+:	7f1dfc41 	fcvtzu	h1, h2, #3
 [0-9a-f]+:	7f1ffc00 	fcvtzu	h0, h0, #1
