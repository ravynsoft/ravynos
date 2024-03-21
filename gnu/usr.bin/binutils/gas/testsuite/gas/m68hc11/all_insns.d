#objdump: -d --prefix-addresses --reloc
#as: -m68hc11
#name: all_insns

# Test handling of basic instructions.

.*: +file format elf32\-m68hc11

Disassembly of section .text:
0+0+ <L0> aba
0+0001 <L1> abx
0+0002 <L2> aby
0+0004 <L3> adca	#0x67
0+0006 <L4> adca	\*0x0+0+ <L0>
			7: R_M68HC11_8	Z198
0+0008 <L5> adca	0x69,x
0+000a <L6> adca	0x0+0+ <L0>
			b: R_M68HC11_16	symbol115
0+000d <L7> adca	0x51,x
0+000f <L8> adcb	#0xff
0+0011 <L9> adcb	\*0x0+0+ <L0>
			12: R_M68HC11_8	Z74
0+0013 <L10> adcb	0xec,x
0+0015 <L11> adcb	0x0+0+ <L0>
			16: R_M68HC11_16	symbol41
0+0018 <L12> adcb	0xcd,x
0+001a <L13> adda	#0xba
0+001c <L14> adda	\*0x0+0+ <L0>
			1d: R_M68HC11_8	Z171
0+001e <L15> adda	0xf2,x
0+0020 <L16> adda	0x0+0+ <L0>
			21: R_M68HC11_16	symbol251
0+0023 <L17> adda	0xe3,x
0+0025 <L18> addb	#0x46
0+0027 <L19> addb	\*0x0+0+ <L0>
			28: R_M68HC11_8	Z124
0+0029 <L20> addb	0xc2,x
0+002b <L21> addb	0x0+0+ <L0>
			2c: R_M68HC11_16	symbol84
0+002e <L22> addb	0xf8,x
0+0030 <L23> addd	#0x0+231b <L330\+0x2034>
0+0033 <L24> addd	\*0x0+0+ <L0>
			34: R_M68HC11_8	Z232
0+0035 <L25> addd	0xe7,x
0+0037 <L26> addd	0x0+0+ <L0>
			38: R_M68HC11_16	symbol141
0+003a <L27> addd	0x76,x
0+003c <L28> anda	#0x5a
0+003e <L29> anda	\*0x0+0+ <L0>
			3f: R_M68HC11_8	Z46
0+0040 <L30> anda	0x63,x
0+0042 <L31> anda	0x0+0+ <L0>
			43: R_M68HC11_16	symbol51
0+0045 <L32> anda	0x9f,x
0+0047 <L33> andb	#0xc9
0+0049 <L34> andb	\*0x0+0+ <L0>
			4a: R_M68HC11_8	Z154
0+004b <L35> andb	0x66,x
0+004d <L36> andb	0x0+0+ <L0>
			4e: R_M68HC11_16	symbol50
0+0050 <L37> andb	0xd,x
0+0052 <L38> asl	0xb7,x
0+0054 <L39> asl	0x0+0+ <L0>
			55: R_M68HC11_16	symbol49
0+0057 <L40> asl	0x58,x
0+0059 <L41> asla
0+005a <L42> aslb
0+005b <L43> asld
0+005c <L44> asr	0xa3,x
0+005e <L45> asr	0x0+0+ <L0>
			5f: R_M68HC11_16	symbol90
0+0061 <L46> asr	0x25,x
0+0063 <L47> asra
0+0064 <L48> asrb
0+0065 <L49> bcs	0x0+006a <L50>
			65: R_M68HC11_RL_JUMP	\*ABS\*
0+0067 <L49\+0x2> jmp	0x0+0+ <L0>
			68: R_M68HC11_16	L93
0+006a <L50> bclr	\*0x0+0+ <L0>, #0x00
			6b: R_M68HC11_8	Z5
			6c: R_M68HC11_8	\$17
0+006d <L51> bclr	0x58,x, #0x00
			6f: R_M68HC11_8	\$e9
0+0070 <L52> bclr	0x5e,x, #0x00
			72: R_M68HC11_8	\$d4
0+0073 <L53> bcc	0x0+0078 <L54>
			73: R_M68HC11_RL_JUMP	\*ABS\*
0+0075 <L53\+0x2> jmp	0x0+0+ <L0>
			76: R_M68HC11_16	L171
0+0078 <L54> bne	0x0+007d <L55>
			78: R_M68HC11_RL_JUMP	\*ABS\*
0+007a <L54\+0x2> jmp	0x0+0+ <L0>
			7b: R_M68HC11_16	L178
0+007d <L55> blt	0x0+0082 <L56>
			7d: R_M68HC11_RL_JUMP	\*ABS\*
0+007f <L55\+0x2> jmp	0x0+0+ <L0>
			80: R_M68HC11_16	L205
0+0082 <L56> ble	0x0+0087 <L57>
			82: R_M68HC11_RL_JUMP	\*ABS\*
0+0084 <L56\+0x2> jmp	0x0+0+ <L0>
			85: R_M68HC11_16	L198
0+0087 <L57> bls	0x0+008c <L58>
			87: R_M68HC11_RL_JUMP	\*ABS\*
0+0089 <L57\+0x2> jmp	0x0+0+ <L0>
			8a: R_M68HC11_16	L155
0+008c <L58> bcs	0x0+0091 <L59>
			8c: R_M68HC11_RL_JUMP	\*ABS\*
0+008e <L58\+0x2> jmp	0x0+0+ <L0>
			8f: R_M68HC11_16	L180
0+0091 <L59> bita	#0x54
0+0093 <L60> bita	\*0x0+0+ <L0>
			94: R_M68HC11_8	Z17
0+0095 <L61> bita	0xe,x
0+0097 <L62> bita	0x0+0+ <L0>
			98: R_M68HC11_16	symbol130
0+009a <L63> bita	0x74,x
0+009c <L64> bitb	#0x41
0+009e <L65> bitb	\*0x0+0+ <L0>
			9f: R_M68HC11_8	Z33
0+00a0 <L66> bitb	0x3d,x
0+00a2 <L67> bitb	0x0+0+ <L0>
			a3: R_M68HC11_16	symbol220
0+00a5 <L68> bitb	0x87,x
0+00a7 <L69> ble	0x0+011d <L112>
			a7: R_M68HC11_RL_JUMP	\*ABS\*
0+00a9 <L70> bcc	0x0+00ae <L71>
			a9: R_M68HC11_RL_JUMP	\*ABS\*
0+00ab <L70\+0x2> jmp	0x0+0+ <L0>
			ac: R_M68HC11_16	L233
0+00ae <L71> bls	0x0+0097 <L62>
			ae: R_M68HC11_RL_JUMP	\*ABS\*
0+00b0 <L72> bge	0x0+00b5 <L73>
			b0: R_M68HC11_RL_JUMP	\*ABS\*
0+00b2 <L72\+0x2> jmp	0x0+0+ <L0>
			b3: R_M68HC11_16	L161
0+00b5 <L73> bmi	0x0+009e <L65>
			b5: R_M68HC11_RL_JUMP	\*ABS\*
0+00b7 <L74> beq	0x0+00bc <L75>
			b7: R_M68HC11_RL_JUMP	\*ABS\*
0+00b9 <L74\+0x2> jmp	0x0+0+ <L0>
			ba: R_M68HC11_16	L225
0+00bc <L75> bmi	0x0+00c1 <L76>
			bc: R_M68HC11_RL_JUMP	\*ABS\*
0+00be <L75\+0x2> jmp	0x0+0+ <L0>
			bf: R_M68HC11_16	L252
0+00c1 <L76> bra	0x0+0106 <L103>
			c1: R_M68HC11_RL_JUMP	\*ABS\*
0+00c3 <L77> brclr	\*0x0+0+ <L0>, #0x00, 0x0+0145 <L125\+0x2>
			c3: R_M68HC11_RL_JUMP	\*ABS\*
			c4: R_M68HC11_8	Z62
			c5: R_M68HC11_8	\$01
0+00c7 <L78> brclr	0x97,x, #0x00, 0x0+0127 <L115>
			c7: R_M68HC11_RL_JUMP	\*ABS\*
			c9: R_M68HC11_8	\$ea
0+00cb <L79> brclr	0x6b,x, #0x00, 0x0+00de <L84\+0x1>
			cb: R_M68HC11_RL_JUMP	\*ABS\*
			cd: R_M68HC11_8	\$96
0+00cf <L80> brn	0x0+0082 <L56>
			cf: R_M68HC11_RL_JUMP	\*ABS\*
0+00d1 <L81> brset	\*0x0+0+ <L0>, #0x00, 0x0+0141 <L124>
			d1: R_M68HC11_RL_JUMP	\*ABS\*
			d2: R_M68HC11_8	Z92
			d3: R_M68HC11_8	\$2a
0+00d5 <L82> brset	0xb0,x, #0x00, 0x0+0154 <L132>
			d5: R_M68HC11_RL_JUMP	\*ABS\*
			d7: R_M68HC11_8	\$3b
0+00d9 <L83> brset	0x32,x, #0x00, 0x0+0119 <L110\+0x2>
			d9: R_M68HC11_RL_JUMP	\*ABS\*
			db: R_M68HC11_8	\$af
0+00dd <L84> bset	\*0x0+0+ <L0>, #0x00
			de: R_M68HC11_8	Z84
			df: R_M68HC11_8	\$ec
0+00e0 <L85> bset	0x18,x, #0x00
			e2: R_M68HC11_8	\$db
0+00e3 <L86> bset	0x5c,x, #0x00
			e5: R_M68HC11_8	\$02
0+00e6 <L87> jsr	0x0+0+ <L0>
			e6: R_M68HC11_RL_JUMP	\*ABS\*
			e7: R_M68HC11_16	L26
0+00e9 <L88> bvs	0x0+00ee <L89>
			e9: R_M68HC11_RL_JUMP	\*ABS\*
0+00eb <L88\+0x2> jmp	0x0+0+ <L0>
			ec: R_M68HC11_16	L254
0+00ee <L89> bvs	0x0+00a2 <L67>
			ee: R_M68HC11_RL_JUMP	\*ABS\*
0+00f0 <L90> cba
0+00f1 <L91> clc
0+00f2 <L92> cli
0+00f3 <L93> clr	0xfb,x
0+00f5 <L94> clr	0x0+0+ <L0>
			f6: R_M68HC11_16	symbol250
0+00f8 <L95> clr	0xaa,x
0+00fa <L96> clra
0+00fb <L97> clrb
0+00fc <L98> clv
0+00fd <L99> cmpa	#0x3a
0+00ff <L100> cmpa	\*0x0+0+ <L0>
			100: R_M68HC11_8	Z251
0+0101 <L101> cmpa	0x29,x
0+0103 <L102> cmpa	0x0+0+ <L0>
			104: R_M68HC11_16	symbol209
0+0106 <L103> cmpa	0xe6,x
0+0108 <L104> cmpb	#0x5
0+010a <L105> cmpb	\*0x0+0+ <L0>
			10b: R_M68HC11_8	Z60
0+010c <L106> cmpb	0x7c,x
0+010e <L107> cmpb	0x0+0+ <L0>
			10f: R_M68HC11_16	symbol148
0+0111 <L108> cmpb	0x75,x
0+0113 <L109> cpd	#0x0+0fd8 <L330\+0xcf1>
0+0117 <L110> cpd	\*0x0+0+ <L0>
			119: R_M68HC11_8	Z190
0+011a <L111> cpd	0x61,x
0+011d <L112> cpd	0x0+0+ <L0>
			11f: R_M68HC11_16	symbol137
0+0121 <L113> cpd	0xf9,x
0+0124 <L114> cpx	#0x0+af5c <L330\+0xac75>
0+0127 <L115> cpx	\*0x0+0+ <L0>
			128: R_M68HC11_8	Z187
0+0129 <L116> cpx	0xa8,x
0+012b <L117> cpx	0x0+0+ <L0>
			12c: R_M68HC11_16	symbol153
0+012e <L118> cpx	0xf,x
0+0130 <L119> cpy	#0x0+4095 <L330\+0x3dae>
0+0134 <L120> cpy	\*0x0+0+ <L0>
			136: R_M68HC11_8	Z177
0+0137 <L121> cpy	0xeb,x
0+013a <L122> cpy	0x0+0+ <L0>
			13c: R_M68HC11_16	symbol241
0+013e <L123> cpy	0xb3,x
0+0141 <L124> com	0x5,x
0+0143 <L125> com	0x0+0+ <L0>
			144: R_M68HC11_16	symbol239
0+0146 <L126> com	0xf7,x
0+0148 <L127> coma
0+0149 <L128> comb
0+014a <L129> cpd	#0x0+bf00 <L330\+0xbc19>
0+014e <L130> cpd	\*0x0+0+ <L0>
			150: R_M68HC11_8	Z233
0+0151 <L131> cpd	0xa1,x
0+0154 <L132> cpd	0x0+0+ <L0>
			156: R_M68HC11_16	symbol58
0+0158 <L133> cpd	0xe5,x
0+015b <L134> cpx	#0x0+8fca <L330\+0x8ce3>
0+015e <L135> cpx	\*0x0+0+ <L0>
			15f: R_M68HC11_8	Z11
0+0160 <L136> cpx	0xcb,x
0+0162 <L137> cpx	0x0+0+ <L0>
			163: R_M68HC11_16	symbol208
0+0165 <L138> cpx	0x48,x
0+0167 <L139> cpy	#0x0+0247 <L248>
0+016b <L140> cpy	\*0x0+0+ <L0>
			16d: R_M68HC11_8	Z100
0+016e <L141> cpy	0xbd,x
0+0171 <L142> cpy	0x0+0+ <L0>
			173: R_M68HC11_16	symbol31
0+0175 <L143> cpy	0x23,x
0+0178 <L144> daa
0+0179 <L145> dec	0x1e,x
0+017b <L146> dec	0x0+0+ <L0>
			17c: R_M68HC11_16	symbol168
0+017e <L147> dec	0x1c,x
0+0180 <L148> deca
0+0181 <L149> decb
0+0182 <L150> des
0+0183 <L151> dex
0+0184 <L152> dey
0+0186 <L153> eora	#0x7b
0+0188 <L154> eora	\*0x0+0+ <L0>
			189: R_M68HC11_8	Z100
0+018a <L155> eora	0xc5,x
0+018c <L156> eora	0x0+0+ <L0>
			18d: R_M68HC11_16	symbol20
0+018f <L157> eora	0x73,x
0+0191 <L158> eorb	#0x5a
0+0193 <L159> eorb	\*0x0+0+ <L0>
			194: R_M68HC11_8	Z197
0+0195 <L160> eorb	0x5e,x
0+0197 <L161> eorb	0x0+0+ <L0>
			198: R_M68HC11_16	symbol75
0+019a <L162> eorb	0x79,x
0+019c <L163> fdiv
0+019d <L164> idiv
0+019e <L165> inc	0x63,x
0+01a0 <L166> inc	0x0+0+ <L0>
			1a1: R_M68HC11_16	symbol59
0+01a3 <L167> inc	0x70,x
0+01a5 <L168> inca
0+01a6 <L169> incb
0+01a7 <L170> ins
0+01a8 <L171> inx
0+01a9 <L172> iny
0+01ab <L173> jmp	0x64,x
0+01ad <L174> jmp	0x0+0+ <L0>
			1ad: R_M68HC11_RL_JUMP	\*ABS\*
			1ae: R_M68HC11_16	symbol36
0+01b0 <L175> jmp	0x11,x
0+01b2 <L176> jsr	\*0x0+0+ <L0>
			1b2: R_M68HC11_RL_JUMP	\*ABS\*
			1b3: R_M68HC11_8	Z158
0+01b4 <L177> jsr	0x9,x
0+01b6 <L178> jsr	0x0+0+ <L0>
			1b6: R_M68HC11_RL_JUMP	\*ABS\*
			1b7: R_M68HC11_16	symbol220
0+01b9 <L179> jsr	0xaa,x
0+01bb <L180> ldaa	#0xd4
0+01bd <L181> ldaa	\*0x0+0+ <L0>
			1be: R_M68HC11_8	Z172
0+01bf <L182> ldaa	0xf2,x
0+01c1 <L183> ldaa	0x0+0+ <L0>
			1c2: R_M68HC11_16	symbol27
0+01c4 <L184> ldaa	0x10,x
0+01c6 <L185> ldab	#0xaf
0+01c8 <L186> ldab	\*0x0+0+ <L0>
			1c9: R_M68HC11_8	Z59
0+01ca <L187> ldab	0x33,x
0+01cc <L188> ldab	0x0+0+ <L0>
			1cd: R_M68HC11_16	symbol205
0+01cf <L189> ldab	0xe3,x
0+01d1 <L190> ldd	#0x0+c550 <L330\+0xc269>
0+01d4 <L191> ldd	\*0x0+0+ <L0>
			1d5: R_M68HC11_8	Z72
0+01d6 <L192> ldd	0x47,x
0+01d8 <L193> ldd	0x0+0+ <L0>
			1d9: R_M68HC11_16	symbol21
0+01db <L194> ldd	0x5c,x
0+01dd <L195> lds	#0x0+4fbb <L330\+0x4cd4>
0+01e0 <L196> lds	\*0x0+0+ <L0>
			1e1: R_M68HC11_8	Z111
0+01e2 <L197> lds	0x22,x
0+01e4 <L198> lds	0x0+0+ <L0>
			1e5: R_M68HC11_16	symbol25
0+01e7 <L199> lds	0xba,x
0+01e9 <L200> ldx	#0x0+579b <L330\+0x54b4>
0+01ec <L201> ldx	\*0x0+0+ <L0>
			1ed: R_M68HC11_8	Z125
0+01ee <L202> ldx	0xf5,x
0+01f0 <L203> ldx	0x0+0+ <L0>
			1f1: R_M68HC11_16	symbol11
0+01f3 <L204> ldx	0xe1,x
0+01f5 <L205> ldy	#0x0+ac1a <L330\+0xa933>
0+01f9 <L206> ldy	\*0x0+0+ <L0>
			1fb: R_M68HC11_8	Z28
0+01fc <L207> ldy	0x7f,x
0+01ff <L208> ldy	0x0+0+ <L0>
			201: R_M68HC11_16	symbol35
0+0203 <L209> ldy	0xf8,x
0+0206 <L210> asl	0x29,x
0+0208 <L211> asl	0x0+0+ <L0>
			209: R_M68HC11_16	symbol248
0+020b <L212> asl	0xa4,x
0+020d <L213> asla
0+020e <L214> aslb
0+020f <L215> asld
0+0210 <L216> lsr	0x1b,x
0+0212 <L217> lsr	0x0+0+ <L0>
			213: R_M68HC11_16	symbol19
0+0215 <L218> lsr	0xb5,x
0+0217 <L219> lsra
0+0218 <L220> lsrb
0+0219 <L221> lsrd
0+021a <L222> mul
0+021b <L223> neg	0xca,x
0+021d <L224> neg	0x0+0+ <L0>
			21e: R_M68HC11_16	symbol78
0+0220 <L225> neg	0xe8,x
0+0222 <L226> nega
0+0223 <L227> negb
0+0224 <L228> nop
0+0225 <L229> oraa	#0x98
0+0227 <L230> oraa	\*0x0+0+ <L0>
			228: R_M68HC11_8	Z50
0+0229 <L231> oraa	0x38,x
0+022b <L232> oraa	0x0+0+ <L0>
			22c: R_M68HC11_16	symbol224
0+022e <L233> oraa	0x79,x
0+0230 <L234> orab	#0x4d
0+0232 <L235> orab	\*0x0+0+ <L0>
			233: R_M68HC11_8	Z61
0+0234 <L236> orab	0x34,x
0+0236 <L237> orab	0x0+0+ <L0>
			237: R_M68HC11_16	symbol188
0+0239 <L238> orab	0x5f,x
0+023b <L239> psha
0+023c <L240> pshb
0+023d <L241> pshx
0+023e <L242> pshy
0+0240 <L243> pula
0+0241 <L244> pulb
0+0242 <L245> pulx
0+0243 <L246> puly
0+0245 <L247> rol	0x4e,x
0+0247 <L248> rol	0x0+0+ <L0>
			248: R_M68HC11_16	symbol119
0+024a <L249> rol	0xfa,x
0+024c <L250> rola
0+024d <L251> rolb
0+024e <L252> ror	0xcb,x
0+0250 <L253> ror	0x0+0+ <L0>
			251: R_M68HC11_16	symbol108
0+0253 <L254> ror	0x5,x
0+0255 <L255> rora
0+0256 <L256> rorb
0+0257 <L257> rti
0+0258 <L258> rts
0+0259 <L259> sba
0+025a <L260> sbca	#0xac
0+025c <L261> sbca	\*0x0+0+ <L0>
			25d: R_M68HC11_8	Z134
0+025e <L262> sbca	0x21,x
0+0260 <L263> sbca	0x0+0+ <L0>
			261: R_M68HC11_16	symbol43
0+0263 <L264> sbca	0xaa,x
0+0265 <L265> sbcb	#0x1a
0+0267 <L266> sbcb	\*0x0+0+ <L0>
			268: R_M68HC11_8	Z85
0+0269 <L267> sbcb	0xa2,x
0+026b <L268> sbcb	0x0+0+ <L0>
			26c: R_M68HC11_16	symbol190
0+026e <L269> sbcb	0x70,x
0+0270 <L270> sec
0+0271 <L271> sei
0+0272 <L272> sev
0+0273 <L273> staa	\*0x0+0+ <L0>
			274: R_M68HC11_8	Z181
0+0275 <L274> staa	0x73,x
0+0277 <L275> staa	0x0+0+ <L0>
			278: R_M68HC11_16	symbol59
0+027a <L276> staa	0x4,x
0+027c <L277> stab	\*0x0+0+ <L0>
			27d: R_M68HC11_8	Z92
0+027e <L278> stab	0xd3,x
0+0280 <L279> stab	0x0+0+ <L0>
			281: R_M68HC11_16	symbol54
0+0283 <L280> stab	0x94,x
0+0285 <L281> std	\*0x0+0+ <L0>
			286: R_M68HC11_8	Z179
0+0287 <L282> std	0xaf,x
0+0289 <L283> std	0x0+0+ <L0>
			28a: R_M68HC11_16	symbol226
0+028c <L284> std	0xf0,x
0+028e <L285> stop
0+028f <L286> sts	\*0x0+0+ <L0>
			290: R_M68HC11_8	Z228
0+0291 <L287> sts	0x9e,x
0+0293 <L288> sts	0x0+0+ <L0>
			294: R_M68HC11_16	symbol79
0+0296 <L289> sts	0x32,x
0+0298 <L290> stx	\*0x0+0+ <L0>
			299: R_M68HC11_8	Z21
0+029a <L291> stx	0x49,x
0+029c <L292> stx	0x0+0+ <L0>
			29d: R_M68HC11_16	symbol253
0+029f <L293> stx	0x82,x
0+02a1 <L294> sty	\*0x0+0+ <L0>
			2a3: R_M68HC11_8	Z78
0+02a4 <L295> sty	0xa9,x
0+02a7 <L296> sty	0x0+0+ <L0>
			2a9: R_M68HC11_16	symbol8
0+02ab <L297> sty	0x70,x
0+02ae <L298> suba	#0xd4
0+02b0 <L299> suba	\*0x0+0+ <L0>
			2b1: R_M68HC11_8	Z178
0+02b2 <L300> suba	0x8a,x
0+02b4 <L301> suba	0x0+0+ <L0>
			2b5: R_M68HC11_16	symbol41
0+02b7 <L302> suba	0x54,x
0+02b9 <L303> subb	#0x48
0+02bb <L304> subb	\*0x0+0+ <L0>
			2bc: R_M68HC11_8	Z154
0+02bd <L305> subb	0xa,x
0+02bf <L306> subb	0x0+0+ <L0>
			2c0: R_M68HC11_16	symbol188
0+02c2 <L307> subb	0xd5,x
0+02c4 <L308> subd	#0x0+f10e <L330\+0xee27>
0+02c7 <L309> subd	\*0x0+0+ <L0>
			2c8: R_M68HC11_8	Z24
0+02c9 <L310> subd	0xa8,x
0+02cb <L311> subd	0x0+0+ <L0>
			2cc: R_M68HC11_16	symbol68
0+02ce <L312> subd	0xac,x
0+02d0 <L313> swi
0+02d1 <L314> tab
0+02d2 <L315> tap
0+02d3 <L316> tba
	...
0+02d5 <L318> tpa
0+02d6 <L319> tst	0x5b,x
0+02d8 <L320> tst	0x0+0+ <L0>
			2d9: R_M68HC11_16	symbol243
0+02db <L321> tst	0x8e,x
0+02dd <L322> tsta
0+02de <L323> tstb
0+02df <L324> tsx
0+02e0 <L325> tsy
0+02e2 <L326> txs
0+02e3 <L327> tys
0+02e5 <L328> wai
0+02e6 <L329> xgdx
0+02e7 <L330> xgdy
