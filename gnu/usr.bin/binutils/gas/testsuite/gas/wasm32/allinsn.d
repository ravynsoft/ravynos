#as:
#objdump: -d
#name: allinsn

.*: +file format .*


Disassembly of section .text:

00000000 <.text>:
   0:	02 40       		block\[\]
   2:	0c 00       		br 0
   4:	0d 00       		br_if 0
   6:	0e 01 01 01 		br_table 1 1 1
   a:	10 00       		call 0x0
   c:	11 00 00    		call_indirect 0 0
   f:	1a          		drop
  10:	05          		else
  11:	0b          		end
  12:	8b          		f32.abs
  13:	92          		f32.add
  14:	8d          		f32.ceil
  15:	43 d0 0f 49 		f32.const 3.14159012
  19:	40 
  1a:	b2          		f32.convert_s/i32
  1b:	b4          		f32.convert_s/i64
  1c:	b3          		f32.convert_u/i32
  1d:	b5          		f32.convert_u/i64
  1e:	98          		f32.copysign
  1f:	b6          		f32.demote/f64
  20:	95          		f32.div
  21:	5b          		f32.eq
  22:	8e          		f32.floor
  23:	60          		f32.ge
  24:	5e          		f32.gt
  25:	5f          		f32.le
  26:	2a 00 00    		f32.load a=0 0
  29:	5d          		f32.lt
  2a:	97          		f32.max
  2b:	96          		f32.min
  2c:	94          		f32.mul
  2d:	5c          		f32.ne
  2e:	90          		f32.nearest
  2f:	8c          		f32.neg
  30:	be          		f32.reinterpret/i32
  31:	91          		f32.sqrt
  32:	38 00 00    		f32.store a=0 0
  35:	93          		f32.sub
  36:	8f          		f32.trunc
  37:	99          		f64.abs
  38:	a0          		f64.add
  39:	9b          		f64.ceil
  3a:	44 97 5f 4f 		f64.const 3.1415899999999998e\+200
  3e:	fd bc 6a 90 
  42:	69 
  43:	b7          		f64.convert_s/i32
  44:	b9          		f64.convert_s/i64
  45:	b8          		f64.convert_u/i32
  46:	ba          		f64.convert_u/i64
  47:	a6          		f64.copysign
  48:	a3          		f64.div
  49:	61          		f64.eq
  4a:	9c          		f64.floor
  4b:	66          		f64.ge
  4c:	64          		f64.gt
  4d:	65          		f64.le
  4e:	2b 00 00    		f64.load a=0 0
  51:	63          		f64.lt
  52:	a5          		f64.max
  53:	a4          		f64.min
  54:	a2          		f64.mul
  55:	62          		f64.ne
  56:	9e          		f64.nearest
  57:	9a          		f64.neg
  58:	bb          		f64.promote/f32
  59:	bf          		f64.reinterpret/i64
  5a:	9f          		f64.sqrt
  5b:	39 00 00    		f64.store a=0 0
  5e:	a1          		f64.sub
  5f:	9d          		f64.trunc
  60:	23 00       		get_global 0
  62:	20 00       		get_local 0
  64:	6a          		i32.add
  65:	71          		i32.and
  66:	67          		i32.clz
  67:	41 ef fd b6 		i32.const 3735928559
  6b:	f5 0d 
  6d:	68          		i32.ctz
  6e:	6d          		i32.div_s
  6f:	6e          		i32.div_u
  70:	46          		i32.eq
  71:	45          		i32.eqz
  72:	4e          		i32.ge_s
  73:	4f          		i32.ge_u
  74:	4a          		i32.gt_s
  75:	4b          		i32.gt_u
  76:	4c          		i32.le_s
  77:	4d          		i32.le_u
  78:	28 00 00    		i32.load a=0 0
  7b:	2e 00 00    		i32.load16_s a=0 0
  7e:	2f 00 00    		i32.load16_u a=0 0
  81:	2c 00 00    		i32.load8_s a=0 0
  84:	2d 00 00    		i32.load8_u a=0 0
  87:	48          		i32.lt_s
  88:	49          		i32.lt_u
  89:	6c          		i32.mul
  8a:	47          		i32.ne
  8b:	72          		i32.or
  8c:	69          		i32.popcnt
  8d:	bc          		i32.reinterpret/f32
  8e:	6f          		i32.rem_s
  8f:	70          		i32.rem_u
  90:	77          		i32.rotl
  91:	78          		i32.rotr
  92:	74          		i32.shl
  93:	75          		i32.shr_s
  94:	76          		i32.shr_u
  95:	36 00 00    		i32.store a=0 0
  98:	3b 00 00    		i32.store16 a=0 0
  9b:	3a 00 00    		i32.store8 a=0 0
  9e:	6b          		i32.sub
  9f:	a8          		i32.trunc_s/f32
  a0:	aa          		i32.trunc_s/f64
  a1:	a9          		i32.trunc_u/f32
  a2:	ab          		i32.trunc_u/f64
  a3:	a7          		i32.wrap/i64
  a4:	73          		i32.xor
  a5:	7c          		i64.add
  a6:	83          		i64.and
  a7:	79          		i64.clz
  a8:	42 ef fd b6 		i64.const -2401053088876216593
  ac:	f5 fd dd ef 
  b0:	d6 5e 
  b2:	7a          		i64.ctz
  b3:	7f          		i64.div_s
  b4:	80          		i64.div_u
  b5:	51          		i64.eq
  b6:	50          		i64.eqz
  b7:	ac          		i64.extend_s/i32
  b8:	ad          		i64.extend_u/i32
  b9:	59          		i64.ge_s
  ba:	5a          		i64.ge_u
  bb:	55          		i64.gt_s
  bc:	56          		i64.gt_u
  bd:	57          		i64.le_s
  be:	58          		i64.le_u
  bf:	29 00 00    		i64.load a=0 0
  c2:	32 00 00    		i64.load16_s a=0 0
  c5:	33 00 00    		i64.load16_u a=0 0
  c8:	34 00 00    		i64.load32_s a=0 0
  cb:	35 00 00    		i64.load32_u a=0 0
  ce:	30 00 00    		i64.load8_s a=0 0
  d1:	31 00 00    		i64.load8_u a=0 0
  d4:	53          		i64.lt_s
  d5:	54          		i64.lt_u
  d6:	7e          		i64.mul
  d7:	52          		i64.ne
  d8:	84          		i64.or
  d9:	7b          		i64.popcnt
  da:	bd          		i64.reinterpret/f64
  db:	81          		i64.rem_s
  dc:	82          		i64.rem_u
  dd:	89          		i64.rotl
  de:	8a          		i64.rotr
  df:	86          		i64.shl
  e0:	87          		i64.shr_s
  e1:	88          		i64.shr_u
  e2:	37 00 00    		i64.store a=0 0
  e5:	3d 00 00    		i64.store16 a=0 0
  e8:	3e 00 00    		i64.store32 a=0 0
  eb:	3c 00 00    		i64.store8 a=0 0
  ee:	7d          		i64.sub
  ef:	ae          		i64.trunc_s/f32
  f0:	b0          		i64.trunc_s/f64
  f1:	af          		i64.trunc_u/f32
  f2:	b1          		i64.trunc_u/f64
  f3:	85          		i64.xor
  f4:	04 7f       		if\[i\]
  f6:	03 7e       		loop\[l\]
  f8:	01          		nop
  f9:	0f          		return
  fa:	1b          		select
  fb:	24 00       		set_global 0
  fd:	21 00       		set_local 0
  ff:	60          		f32.ge
 100:	08          		.byte 0x08

 101:	7f          		i64.div_s
 102:	7e          		i64.mul
 103:	7c          		i64.add
 104:	7d          		i64.sub
 105:	7d          		i64.sub
 106:	7c          		i64.add
 107:	7e          		i64.mul
 108:	7f          		i64.div_s
 109:	00          		unreachable
 10a:	22 00       		tee_local 0
	...
