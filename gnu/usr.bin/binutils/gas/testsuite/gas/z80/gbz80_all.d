#as: -march=gbz80
#objdump: -d
#name: GBZ80 instruction set

.*: .*

Disassembly of section .text:

0+ <.text>:
\s+[0-9a-f]+:\s+00\s+nop
\s+[0-9a-f]+:\s+01 af be\s+ld bc,0xbeaf
\s+[0-9a-f]+:\s+02\s+ld \(bc\),a
\s+[0-9a-f]+:\s+03\s+inc bc
\s+[0-9a-f]+:\s+04\s+inc b
\s+[0-9a-f]+:\s+05\s+dec b
\s+[0-9a-f]+:\s+06 fd\s+ld b,0xfd
\s+[0-9a-f]+:\s+07\s+rlca
\s+[0-9a-f]+:\s+08 af be\s+ld \(0xbeaf\),sp
\s+[0-9a-f]+:\s+09\s+add hl,bc
\s+[0-9a-f]+:\s+0a\s+ld a,\(bc\)
\s+[0-9a-f]+:\s+0b\s+dec bc
\s+[0-9a-f]+:\s+0c\s+inc c
\s+[0-9a-f]+:\s+0d\s+dec c
\s+[0-9a-f]+:\s+0e fd\s+ld c,0xfd
\s+[0-9a-f]+:\s+0f\s+rrca
\s+[0-9a-f]+:\s+10\s+stop
\s+[0-9a-f]+:\s+11 af be\s+ld de,0xbeaf
\s+[0-9a-f]+:\s+12\s+ld \(de\),a
\s+[0-9a-f]+:\s+13\s+inc de
\s+[0-9a-f]+:\s+14\s+inc d
\s+[0-9a-f]+:\s+15\s+dec d
\s+[0-9a-f]+:\s+16 fd\s+ld d,0xfd
\s+[0-9a-f]+:\s+17\s+rla
\s+[0-9a-f]+:\s+18 0a\s+jr 0x002d
\s+[0-9a-f]+:\s+19\s+add hl,de
\s+[0-9a-f]+:\s+1a\s+ld a,\(de\)
\s+[0-9a-f]+:\s+1b\s+dec de
\s+[0-9a-f]+:\s+1c\s+inc e
\s+[0-9a-f]+:\s+1d\s+dec e
\s+[0-9a-f]+:\s+1e fd\s+ld e,0xfd
\s+[0-9a-f]+:\s+1f\s+rra
\s+[0-9a-f]+:\s+20 0a\s+jr nz,0x0037
\s+[0-9a-f]+:\s+21 af be\s+ld hl,0xbeaf
\s+[0-9a-f]+:\s+22\s+ld \(hl\+\),a
\s+[0-9a-f]+:\s+22\s+ld \(hl\+\),a
\s+[0-9a-f]+:\s+23\s+inc hl
\s+[0-9a-f]+:\s+24\s+inc h
\s+[0-9a-f]+:\s+25\s+dec h
\s+[0-9a-f]+:\s+26 fd\s+ld h,0xfd
\s+[0-9a-f]+:\s+27\s+daa
\s+[0-9a-f]+:\s+28 0a\s+jr z,0x0044
\s+[0-9a-f]+:\s+29\s+add hl,hl
\s+[0-9a-f]+:\s+2a\s+ld a,\(hl\+\)
\s+[0-9a-f]+:\s+2a\s+ld a,\(hl\+\)
\s+[0-9a-f]+:\s+2b\s+dec hl
\s+[0-9a-f]+:\s+2c\s+inc l
\s+[0-9a-f]+:\s+2d\s+dec l
\s+[0-9a-f]+:\s+2e fd\s+ld l,0xfd
\s+[0-9a-f]+:\s+2f\s+cpl
\s+[0-9a-f]+:\s+30 0a\s+jr nc,0x004f
\s+[0-9a-f]+:\s+31 af be\s+ld sp,0xbeaf
\s+[0-9a-f]+:\s+32\s+ld \(hl-\),a
\s+[0-9a-f]+:\s+32\s+ld \(hl-\),a
\s+[0-9a-f]+:\s+33\s+inc sp
\s+[0-9a-f]+:\s+34\s+inc \(hl\)
\s+[0-9a-f]+:\s+35\s+dec \(hl\)
\s+[0-9a-f]+:\s+36 fd\s+ld \(hl\),0xfd
\s+[0-9a-f]+:\s+37\s+scf
\s+[0-9a-f]+:\s+38 0a\s+jr c,0x005c
\s+[0-9a-f]+:\s+39\s+add hl,sp
\s+[0-9a-f]+:\s+3a\s+ld a,\(hl-\)
\s+[0-9a-f]+:\s+3a\s+ld a,\(hl-\)
\s+[0-9a-f]+:\s+3b\s+dec sp
\s+[0-9a-f]+:\s+3c\s+inc a
\s+[0-9a-f]+:\s+3d\s+dec a
\s+[0-9a-f]+:\s+3e fd\s+ld a,0xfd
\s+[0-9a-f]+:\s+3f\s+ccf
\s+[0-9a-f]+:\s+40\s+ld b,b
\s+[0-9a-f]+:\s+41\s+ld b,c
\s+[0-9a-f]+:\s+42\s+ld b,d
\s+[0-9a-f]+:\s+43\s+ld b,e
\s+[0-9a-f]+:\s+44\s+ld b,h
\s+[0-9a-f]+:\s+45\s+ld b,l
\s+[0-9a-f]+:\s+46\s+ld b,\(hl\)
\s+[0-9a-f]+:\s+47\s+ld b,a
\s+[0-9a-f]+:\s+48\s+ld c,b
\s+[0-9a-f]+:\s+49\s+ld c,c
\s+[0-9a-f]+:\s+4a\s+ld c,d
\s+[0-9a-f]+:\s+4b\s+ld c,e
\s+[0-9a-f]+:\s+4c\s+ld c,h
\s+[0-9a-f]+:\s+4d\s+ld c,l
\s+[0-9a-f]+:\s+4e\s+ld c,\(hl\)
\s+[0-9a-f]+:\s+4f\s+ld c,a
\s+[0-9a-f]+:\s+50\s+ld d,b
\s+[0-9a-f]+:\s+51\s+ld d,c
\s+[0-9a-f]+:\s+52\s+ld d,d
\s+[0-9a-f]+:\s+53\s+ld d,e
\s+[0-9a-f]+:\s+54\s+ld d,h
\s+[0-9a-f]+:\s+55\s+ld d,l
\s+[0-9a-f]+:\s+56\s+ld d,\(hl\)
\s+[0-9a-f]+:\s+57\s+ld d,a
\s+[0-9a-f]+:\s+58\s+ld e,b
\s+[0-9a-f]+:\s+59\s+ld e,c
\s+[0-9a-f]+:\s+5a\s+ld e,d
\s+[0-9a-f]+:\s+5b\s+ld e,e
\s+[0-9a-f]+:\s+5c\s+ld e,h
\s+[0-9a-f]+:\s+5d\s+ld e,l
\s+[0-9a-f]+:\s+5e\s+ld e,\(hl\)
\s+[0-9a-f]+:\s+5f\s+ld e,a
\s+[0-9a-f]+:\s+60\s+ld h,b
\s+[0-9a-f]+:\s+61\s+ld h,c
\s+[0-9a-f]+:\s+62\s+ld h,d
\s+[0-9a-f]+:\s+63\s+ld h,e
\s+[0-9a-f]+:\s+64\s+ld h,h
\s+[0-9a-f]+:\s+65\s+ld h,l
\s+[0-9a-f]+:\s+66\s+ld h,\(hl\)
\s+[0-9a-f]+:\s+67\s+ld h,a
\s+[0-9a-f]+:\s+68\s+ld l,b
\s+[0-9a-f]+:\s+69\s+ld l,c
\s+[0-9a-f]+:\s+6a\s+ld l,d
\s+[0-9a-f]+:\s+6b\s+ld l,e
\s+[0-9a-f]+:\s+6c\s+ld l,h
\s+[0-9a-f]+:\s+6d\s+ld l,l
\s+[0-9a-f]+:\s+6e\s+ld l,\(hl\)
\s+[0-9a-f]+:\s+6f\s+ld l,a
\s+[0-9a-f]+:\s+70\s+ld \(hl\),b
\s+[0-9a-f]+:\s+71\s+ld \(hl\),c
\s+[0-9a-f]+:\s+72\s+ld \(hl\),d
\s+[0-9a-f]+:\s+73\s+ld \(hl\),e
\s+[0-9a-f]+:\s+74\s+ld \(hl\),h
\s+[0-9a-f]+:\s+75\s+ld \(hl\),l
\s+[0-9a-f]+:\s+76\s+halt
\s+[0-9a-f]+:\s+77\s+ld \(hl\),a
\s+[0-9a-f]+:\s+78\s+ld a,b
\s+[0-9a-f]+:\s+79\s+ld a,c
\s+[0-9a-f]+:\s+7a\s+ld a,d
\s+[0-9a-f]+:\s+7b\s+ld a,e
\s+[0-9a-f]+:\s+7c\s+ld a,h
\s+[0-9a-f]+:\s+7d\s+ld a,l
\s+[0-9a-f]+:\s+7e\s+ld a,\(hl\)
\s+[0-9a-f]+:\s+7f\s+ld a,a
\s+[0-9a-f]+:\s+80\s+add a,b
\s+[0-9a-f]+:\s+81\s+add a,c
\s+[0-9a-f]+:\s+82\s+add a,d
\s+[0-9a-f]+:\s+83\s+add a,e
\s+[0-9a-f]+:\s+84\s+add a,h
\s+[0-9a-f]+:\s+85\s+add a,l
\s+[0-9a-f]+:\s+86\s+add a,\(hl\)
\s+[0-9a-f]+:\s+87\s+add a,a
\s+[0-9a-f]+:\s+88\s+adc a,b
\s+[0-9a-f]+:\s+89\s+adc a,c
\s+[0-9a-f]+:\s+8a\s+adc a,d
\s+[0-9a-f]+:\s+8b\s+adc a,e
\s+[0-9a-f]+:\s+8c\s+adc a,h
\s+[0-9a-f]+:\s+8d\s+adc a,l
\s+[0-9a-f]+:\s+8e\s+adc a,\(hl\)
\s+[0-9a-f]+:\s+8f\s+adc a,a
\s+[0-9a-f]+:\s+90\s+sub a,b
\s+[0-9a-f]+:\s+91\s+sub a,c
\s+[0-9a-f]+:\s+92\s+sub a,d
\s+[0-9a-f]+:\s+93\s+sub a,e
\s+[0-9a-f]+:\s+94\s+sub a,h
\s+[0-9a-f]+:\s+95\s+sub a,l
\s+[0-9a-f]+:\s+96\s+sub a,\(hl\)
\s+[0-9a-f]+:\s+97\s+sub a,a
\s+[0-9a-f]+:\s+98\s+sbc a,b
\s+[0-9a-f]+:\s+99\s+sbc a,c
\s+[0-9a-f]+:\s+9a\s+sbc a,d
\s+[0-9a-f]+:\s+9b\s+sbc a,e
\s+[0-9a-f]+:\s+9c\s+sbc a,h
\s+[0-9a-f]+:\s+9d\s+sbc a,l
\s+[0-9a-f]+:\s+9e\s+sbc a,\(hl\)
\s+[0-9a-f]+:\s+9f\s+sbc a,a
\s+[0-9a-f]+:\s+a0\s+and b
\s+[0-9a-f]+:\s+a1\s+and c
\s+[0-9a-f]+:\s+a2\s+and d
\s+[0-9a-f]+:\s+a3\s+and e
\s+[0-9a-f]+:\s+a4\s+and h
\s+[0-9a-f]+:\s+a5\s+and l
\s+[0-9a-f]+:\s+a6\s+and \(hl\)
\s+[0-9a-f]+:\s+a7\s+and a
\s+[0-9a-f]+:\s+a8\s+xor b
\s+[0-9a-f]+:\s+a9\s+xor c
\s+[0-9a-f]+:\s+aa\s+xor d
\s+[0-9a-f]+:\s+ab\s+xor e
\s+[0-9a-f]+:\s+ac\s+xor h
\s+[0-9a-f]+:\s+ad\s+xor l
\s+[0-9a-f]+:\s+ae\s+xor \(hl\)
\s+[0-9a-f]+:\s+af\s+xor a
\s+[0-9a-f]+:\s+b0\s+or b
\s+[0-9a-f]+:\s+b1\s+or c
\s+[0-9a-f]+:\s+b2\s+or d
\s+[0-9a-f]+:\s+b3\s+or e
\s+[0-9a-f]+:\s+b4\s+or h
\s+[0-9a-f]+:\s+b5\s+or l
\s+[0-9a-f]+:\s+b6\s+or \(hl\)
\s+[0-9a-f]+:\s+b7\s+or a
\s+[0-9a-f]+:\s+b8\s+cp b
\s+[0-9a-f]+:\s+b9\s+cp c
\s+[0-9a-f]+:\s+ba\s+cp d
\s+[0-9a-f]+:\s+bb\s+cp e
\s+[0-9a-f]+:\s+bc\s+cp h
\s+[0-9a-f]+:\s+bd\s+cp l
\s+[0-9a-f]+:\s+be\s+cp \(hl\)
\s+[0-9a-f]+:\s+bf\s+cp a
\s+[0-9a-f]+:\s+c0\s+ret nz
\s+[0-9a-f]+:\s+c1\s+pop bc
\s+[0-9a-f]+:\s+c2 af be\s+jp nz,0xbeaf
\s+[0-9a-f]+:\s+c3 af be\s+jp 0xbeaf
\s+[0-9a-f]+:\s+c4 af be\s+call nz,0xbeaf
\s+[0-9a-f]+:\s+c5\s+push bc
\s+[0-9a-f]+:\s+c6 fd\s+add a,0xfd
\s+[0-9a-f]+:\s+c7\s+rst 0x00
\s+[0-9a-f]+:\s+c8\s+ret z
\s+[0-9a-f]+:\s+c9\s+ret
\s+[0-9a-f]+:\s+ca af be\s+jp z,0xbeaf
\s+[0-9a-f]+:\s+00\s+nop
\s+[0-9a-f]+:\s+cc af be\s+call z,0xbeaf
\s+[0-9a-f]+:\s+cd af be\s+call 0xbeaf
\s+[0-9a-f]+:\s+ce fd\s+adc a,0xfd
\s+[0-9a-f]+:\s+cf\s+rst 0x08
\s+[0-9a-f]+:\s+d0\s+ret nc
\s+[0-9a-f]+:\s+d1\s+pop de
\s+[0-9a-f]+:\s+d2 af be\s+jp nc,0xbeaf
\s+[0-9a-f]+:\s+d4 af be\s+call nc,0xbeaf
\s+[0-9a-f]+:\s+d5\s+push de
\s+[0-9a-f]+:\s+d6 fd\s+sub a,0xfd
\s+[0-9a-f]+:\s+d7\s+rst 0x10
\s+[0-9a-f]+:\s+d8\s+ret c
\s+[0-9a-f]+:\s+d9\s+reti
\s+[0-9a-f]+:\s+da af be\s+jp c,0xbeaf
\s+[0-9a-f]+:\s+dc af be\s+call c,0xbeaf
\s+[0-9a-f]+:\s+de fd\s+sbc a,0xfd
\s+[0-9a-f]+:\s+df\s+rst 0x18
\s+[0-9a-f]+:\s+e0 fd\s+ldh \(0xfd\),a
\s+[0-9a-f]+:\s+e1\s+pop hl
\s+[0-9a-f]+:\s+e2\s+ldh \(c\),a
\s+[0-9a-f]+:\s+e5\s+push hl
\s+[0-9a-f]+:\s+e6 fd\s+and 0xfd
\s+[0-9a-f]+:\s+e7\s+rst 0x20
\s+[0-9a-f]+:\s+e8 f4\s+add sp,-12
\s+[0-9a-f]+:\s+e9\s+jp \(hl\)
\s+[0-9a-f]+:\s+ea af be\s+ld \(0xbeaf\),a
\s+[0-9a-f]+:\s+ee fd\s+xor 0xfd
\s+[0-9a-f]+:\s+ef\s+rst 0x28
\s+[0-9a-f]+:\s+f0 fd\s+ldh a,\(0xfd\)
\s+[0-9a-f]+:\s+f1\s+pop af
\s+[0-9a-f]+:\s+f2\s+ldh a,\(c\)
\s+[0-9a-f]+:\s+f3\s+di
\s+[0-9a-f]+:\s+f5\s+push af
\s+[0-9a-f]+:\s+f6 fd\s+or 0xfd
\s+[0-9a-f]+:\s+f7\s+rst 0x30
\s+[0-9a-f]+:\s+f8 f4\s+ldhl sp,-12
\s+[0-9a-f]+:\s+f9\s+ld sp,hl
\s+[0-9a-f]+:\s+fa af be\s+ld a,\(0xbeaf\)
\s+[0-9a-f]+:\s+fb\s+ei
\s+[0-9a-f]+:\s+fe fd\s+cp 0xfd
\s+[0-9a-f]+:\s+ff\s+rst 0x38
\s+[0-9a-f]+:\s+cb 00\s+rlc b
\s+[0-9a-f]+:\s+cb 01\s+rlc c
\s+[0-9a-f]+:\s+cb 02\s+rlc d
\s+[0-9a-f]+:\s+cb 03\s+rlc e
\s+[0-9a-f]+:\s+cb 04\s+rlc h
\s+[0-9a-f]+:\s+cb 05\s+rlc l
\s+[0-9a-f]+:\s+cb 06\s+rlc \(hl\)
\s+[0-9a-f]+:\s+cb 07\s+rlc a
\s+[0-9a-f]+:\s+cb 08\s+rrc b
\s+[0-9a-f]+:\s+cb 09\s+rrc c
\s+[0-9a-f]+:\s+cb 0a\s+rrc d
\s+[0-9a-f]+:\s+cb 0b\s+rrc e
\s+[0-9a-f]+:\s+cb 0c\s+rrc h
\s+[0-9a-f]+:\s+cb 0d\s+rrc l
\s+[0-9a-f]+:\s+cb 0e\s+rrc \(hl\)
\s+[0-9a-f]+:\s+cb 0f\s+rrc a
\s+[0-9a-f]+:\s+cb 10\s+rl b
\s+[0-9a-f]+:\s+cb 11\s+rl c
\s+[0-9a-f]+:\s+cb 12\s+rl d
\s+[0-9a-f]+:\s+cb 13\s+rl e
\s+[0-9a-f]+:\s+cb 14\s+rl h
\s+[0-9a-f]+:\s+cb 15\s+rl l
\s+[0-9a-f]+:\s+cb 16\s+rl \(hl\)
\s+[0-9a-f]+:\s+cb 17\s+rl a
\s+[0-9a-f]+:\s+cb 18\s+rr b
\s+[0-9a-f]+:\s+cb 19\s+rr c
\s+[0-9a-f]+:\s+cb 1a\s+rr d
\s+[0-9a-f]+:\s+cb 1b\s+rr e
\s+[0-9a-f]+:\s+cb 1c\s+rr h
\s+[0-9a-f]+:\s+cb 1d\s+rr l
\s+[0-9a-f]+:\s+cb 1e\s+rr \(hl\)
\s+[0-9a-f]+:\s+cb 1f\s+rr a
\s+[0-9a-f]+:\s+cb 20\s+sla b
\s+[0-9a-f]+:\s+cb 21\s+sla c
\s+[0-9a-f]+:\s+cb 22\s+sla d
\s+[0-9a-f]+:\s+cb 23\s+sla e
\s+[0-9a-f]+:\s+cb 24\s+sla h
\s+[0-9a-f]+:\s+cb 25\s+sla l
\s+[0-9a-f]+:\s+cb 26\s+sla \(hl\)
\s+[0-9a-f]+:\s+cb 27\s+sla a
\s+[0-9a-f]+:\s+cb 28\s+sra b
\s+[0-9a-f]+:\s+cb 29\s+sra c
\s+[0-9a-f]+:\s+cb 2a\s+sra d
\s+[0-9a-f]+:\s+cb 2b\s+sra e
\s+[0-9a-f]+:\s+cb 2c\s+sra h
\s+[0-9a-f]+:\s+cb 2d\s+sra l
\s+[0-9a-f]+:\s+cb 2e\s+sra \(hl\)
\s+[0-9a-f]+:\s+cb 2f\s+sra a
\s+[0-9a-f]+:\s+cb 30\s+swap b
\s+[0-9a-f]+:\s+cb 31\s+swap c
\s+[0-9a-f]+:\s+cb 32\s+swap d
\s+[0-9a-f]+:\s+cb 33\s+swap e
\s+[0-9a-f]+:\s+cb 34\s+swap h
\s+[0-9a-f]+:\s+cb 35\s+swap l
\s+[0-9a-f]+:\s+cb 36\s+swap \(hl\)
\s+[0-9a-f]+:\s+cb 37\s+swap a
\s+[0-9a-f]+:\s+cb 38\s+srl b
\s+[0-9a-f]+:\s+cb 39\s+srl c
\s+[0-9a-f]+:\s+cb 3a\s+srl d
\s+[0-9a-f]+:\s+cb 3b\s+srl e
\s+[0-9a-f]+:\s+cb 3c\s+srl h
\s+[0-9a-f]+:\s+cb 3d\s+srl l
\s+[0-9a-f]+:\s+cb 3e\s+srl \(hl\)
\s+[0-9a-f]+:\s+cb 3f\s+srl a
\s+[0-9a-f]+:\s+cb 40\s+bit 0,b
\s+[0-9a-f]+:\s+cb 41\s+bit 0,c
\s+[0-9a-f]+:\s+cb 42\s+bit 0,d
\s+[0-9a-f]+:\s+cb 43\s+bit 0,e
\s+[0-9a-f]+:\s+cb 44\s+bit 0,h
\s+[0-9a-f]+:\s+cb 45\s+bit 0,l
\s+[0-9a-f]+:\s+cb 46\s+bit 0,\(hl\)
\s+[0-9a-f]+:\s+cb 47\s+bit 0,a
\s+[0-9a-f]+:\s+cb 48\s+bit 1,b
\s+[0-9a-f]+:\s+cb 49\s+bit 1,c
\s+[0-9a-f]+:\s+cb 4a\s+bit 1,d
\s+[0-9a-f]+:\s+cb 4b\s+bit 1,e
\s+[0-9a-f]+:\s+cb 4c\s+bit 1,h
\s+[0-9a-f]+:\s+cb 4d\s+bit 1,l
\s+[0-9a-f]+:\s+cb 4e\s+bit 1,\(hl\)
\s+[0-9a-f]+:\s+cb 4f\s+bit 1,a
\s+[0-9a-f]+:\s+cb 50\s+bit 2,b
\s+[0-9a-f]+:\s+cb 51\s+bit 2,c
\s+[0-9a-f]+:\s+cb 52\s+bit 2,d
\s+[0-9a-f]+:\s+cb 53\s+bit 2,e
\s+[0-9a-f]+:\s+cb 54\s+bit 2,h
\s+[0-9a-f]+:\s+cb 55\s+bit 2,l
\s+[0-9a-f]+:\s+cb 56\s+bit 2,\(hl\)
\s+[0-9a-f]+:\s+cb 57\s+bit 2,a
\s+[0-9a-f]+:\s+cb 58\s+bit 3,b
\s+[0-9a-f]+:\s+cb 59\s+bit 3,c
\s+[0-9a-f]+:\s+cb 5a\s+bit 3,d
\s+[0-9a-f]+:\s+cb 5b\s+bit 3,e
\s+[0-9a-f]+:\s+cb 5c\s+bit 3,h
\s+[0-9a-f]+:\s+cb 5d\s+bit 3,l
\s+[0-9a-f]+:\s+cb 5e\s+bit 3,\(hl\)
\s+[0-9a-f]+:\s+cb 5f\s+bit 3,a
\s+[0-9a-f]+:\s+cb 60\s+bit 4,b
\s+[0-9a-f]+:\s+cb 61\s+bit 4,c
\s+[0-9a-f]+:\s+cb 62\s+bit 4,d
\s+[0-9a-f]+:\s+cb 63\s+bit 4,e
\s+[0-9a-f]+:\s+cb 64\s+bit 4,h
\s+[0-9a-f]+:\s+cb 65\s+bit 4,l
\s+[0-9a-f]+:\s+cb 66\s+bit 4,\(hl\)
\s+[0-9a-f]+:\s+cb 67\s+bit 4,a
\s+[0-9a-f]+:\s+cb 68\s+bit 5,b
\s+[0-9a-f]+:\s+cb 69\s+bit 5,c
\s+[0-9a-f]+:\s+cb 6a\s+bit 5,d
\s+[0-9a-f]+:\s+cb 6b\s+bit 5,e
\s+[0-9a-f]+:\s+cb 6c\s+bit 5,h
\s+[0-9a-f]+:\s+cb 6d\s+bit 5,l
\s+[0-9a-f]+:\s+cb 6e\s+bit 5,\(hl\)
\s+[0-9a-f]+:\s+cb 6f\s+bit 5,a
\s+[0-9a-f]+:\s+cb 70\s+bit 6,b
\s+[0-9a-f]+:\s+cb 71\s+bit 6,c
\s+[0-9a-f]+:\s+cb 72\s+bit 6,d
\s+[0-9a-f]+:\s+cb 73\s+bit 6,e
\s+[0-9a-f]+:\s+cb 74\s+bit 6,h
\s+[0-9a-f]+:\s+cb 75\s+bit 6,l
\s+[0-9a-f]+:\s+cb 76\s+bit 6,\(hl\)
\s+[0-9a-f]+:\s+cb 77\s+bit 6,a
\s+[0-9a-f]+:\s+cb 78\s+bit 7,b
\s+[0-9a-f]+:\s+cb 79\s+bit 7,c
\s+[0-9a-f]+:\s+cb 7a\s+bit 7,d
\s+[0-9a-f]+:\s+cb 7b\s+bit 7,e
\s+[0-9a-f]+:\s+cb 7c\s+bit 7,h
\s+[0-9a-f]+:\s+cb 7d\s+bit 7,l
\s+[0-9a-f]+:\s+cb 7e\s+bit 7,\(hl\)
\s+[0-9a-f]+:\s+cb 7f\s+bit 7,a
\s+[0-9a-f]+:\s+cb 80\s+res 0,b
\s+[0-9a-f]+:\s+cb 81\s+res 0,c
\s+[0-9a-f]+:\s+cb 82\s+res 0,d
\s+[0-9a-f]+:\s+cb 83\s+res 0,e
\s+[0-9a-f]+:\s+cb 84\s+res 0,h
\s+[0-9a-f]+:\s+cb 85\s+res 0,l
\s+[0-9a-f]+:\s+cb 86\s+res 0,\(hl\)
\s+[0-9a-f]+:\s+cb 87\s+res 0,a
\s+[0-9a-f]+:\s+cb 88\s+res 1,b
\s+[0-9a-f]+:\s+cb 89\s+res 1,c
\s+[0-9a-f]+:\s+cb 8a\s+res 1,d
\s+[0-9a-f]+:\s+cb 8b\s+res 1,e
\s+[0-9a-f]+:\s+cb 8c\s+res 1,h
\s+[0-9a-f]+:\s+cb 8d\s+res 1,l
\s+[0-9a-f]+:\s+cb 8e\s+res 1,\(hl\)
\s+[0-9a-f]+:\s+cb 8f\s+res 1,a
\s+[0-9a-f]+:\s+cb 90\s+res 2,b
\s+[0-9a-f]+:\s+cb 91\s+res 2,c
\s+[0-9a-f]+:\s+cb 92\s+res 2,d
\s+[0-9a-f]+:\s+cb 93\s+res 2,e
\s+[0-9a-f]+:\s+cb 94\s+res 2,h
\s+[0-9a-f]+:\s+cb 95\s+res 2,l
\s+[0-9a-f]+:\s+cb 96\s+res 2,\(hl\)
\s+[0-9a-f]+:\s+cb 97\s+res 2,a
\s+[0-9a-f]+:\s+cb 98\s+res 3,b
\s+[0-9a-f]+:\s+cb 99\s+res 3,c
\s+[0-9a-f]+:\s+cb 9a\s+res 3,d
\s+[0-9a-f]+:\s+cb 9b\s+res 3,e
\s+[0-9a-f]+:\s+cb 9c\s+res 3,h
\s+[0-9a-f]+:\s+cb 9d\s+res 3,l
\s+[0-9a-f]+:\s+cb 9e\s+res 3,\(hl\)
\s+[0-9a-f]+:\s+cb 9f\s+res 3,a
\s+[0-9a-f]+:\s+cb a0\s+res 4,b
\s+[0-9a-f]+:\s+cb a1\s+res 4,c
\s+[0-9a-f]+:\s+cb a2\s+res 4,d
\s+[0-9a-f]+:\s+cb a3\s+res 4,e
\s+[0-9a-f]+:\s+cb a4\s+res 4,h
\s+[0-9a-f]+:\s+cb a5\s+res 4,l
\s+[0-9a-f]+:\s+cb a6\s+res 4,\(hl\)
\s+[0-9a-f]+:\s+cb a7\s+res 4,a
\s+[0-9a-f]+:\s+cb a8\s+res 5,b
\s+[0-9a-f]+:\s+cb a9\s+res 5,c
\s+[0-9a-f]+:\s+cb aa\s+res 5,d
\s+[0-9a-f]+:\s+cb ab\s+res 5,e
\s+[0-9a-f]+:\s+cb ac\s+res 5,h
\s+[0-9a-f]+:\s+cb ad\s+res 5,l
\s+[0-9a-f]+:\s+cb ae\s+res 5,\(hl\)
\s+[0-9a-f]+:\s+cb af\s+res 5,a
\s+[0-9a-f]+:\s+cb b0\s+res 6,b
\s+[0-9a-f]+:\s+cb b1\s+res 6,c
\s+[0-9a-f]+:\s+cb b2\s+res 6,d
\s+[0-9a-f]+:\s+cb b3\s+res 6,e
\s+[0-9a-f]+:\s+cb b4\s+res 6,h
\s+[0-9a-f]+:\s+cb b5\s+res 6,l
\s+[0-9a-f]+:\s+cb b6\s+res 6,\(hl\)
\s+[0-9a-f]+:\s+cb b7\s+res 6,a
\s+[0-9a-f]+:\s+cb b8\s+res 7,b
\s+[0-9a-f]+:\s+cb b9\s+res 7,c
\s+[0-9a-f]+:\s+cb ba\s+res 7,d
\s+[0-9a-f]+:\s+cb bb\s+res 7,e
\s+[0-9a-f]+:\s+cb bc\s+res 7,h
\s+[0-9a-f]+:\s+cb bd\s+res 7,l
\s+[0-9a-f]+:\s+cb be\s+res 7,\(hl\)
\s+[0-9a-f]+:\s+cb bf\s+res 7,a
\s+[0-9a-f]+:\s+cb c0\s+set 0,b
\s+[0-9a-f]+:\s+cb c1\s+set 0,c
\s+[0-9a-f]+:\s+cb c2\s+set 0,d
\s+[0-9a-f]+:\s+cb c3\s+set 0,e
\s+[0-9a-f]+:\s+cb c4\s+set 0,h
\s+[0-9a-f]+:\s+cb c5\s+set 0,l
\s+[0-9a-f]+:\s+cb c6\s+set 0,\(hl\)
\s+[0-9a-f]+:\s+cb c7\s+set 0,a
\s+[0-9a-f]+:\s+cb c8\s+set 1,b
\s+[0-9a-f]+:\s+cb c9\s+set 1,c
\s+[0-9a-f]+:\s+cb ca\s+set 1,d
\s+[0-9a-f]+:\s+cb cb\s+set 1,e
\s+[0-9a-f]+:\s+cb cc\s+set 1,h
\s+[0-9a-f]+:\s+cb cd\s+set 1,l
\s+[0-9a-f]+:\s+cb ce\s+set 1,\(hl\)
\s+[0-9a-f]+:\s+cb cf\s+set 1,a
\s+[0-9a-f]+:\s+cb d0\s+set 2,b
\s+[0-9a-f]+:\s+cb d1\s+set 2,c
\s+[0-9a-f]+:\s+cb d2\s+set 2,d
\s+[0-9a-f]+:\s+cb d3\s+set 2,e
\s+[0-9a-f]+:\s+cb d4\s+set 2,h
\s+[0-9a-f]+:\s+cb d5\s+set 2,l
\s+[0-9a-f]+:\s+cb d6\s+set 2,\(hl\)
\s+[0-9a-f]+:\s+cb d7\s+set 2,a
\s+[0-9a-f]+:\s+cb d8\s+set 3,b
\s+[0-9a-f]+:\s+cb d9\s+set 3,c
\s+[0-9a-f]+:\s+cb da\s+set 3,d
\s+[0-9a-f]+:\s+cb db\s+set 3,e
\s+[0-9a-f]+:\s+cb dc\s+set 3,h
\s+[0-9a-f]+:\s+cb dd\s+set 3,l
\s+[0-9a-f]+:\s+cb de\s+set 3,\(hl\)
\s+[0-9a-f]+:\s+cb df\s+set 3,a
\s+[0-9a-f]+:\s+cb e0\s+set 4,b
\s+[0-9a-f]+:\s+cb e1\s+set 4,c
\s+[0-9a-f]+:\s+cb e2\s+set 4,d
\s+[0-9a-f]+:\s+cb e3\s+set 4,e
\s+[0-9a-f]+:\s+cb e4\s+set 4,h
\s+[0-9a-f]+:\s+cb e5\s+set 4,l
\s+[0-9a-f]+:\s+cb e6\s+set 4,\(hl\)
\s+[0-9a-f]+:\s+cb e7\s+set 4,a
\s+[0-9a-f]+:\s+cb e8\s+set 5,b
\s+[0-9a-f]+:\s+cb e9\s+set 5,c
\s+[0-9a-f]+:\s+cb ea\s+set 5,d
\s+[0-9a-f]+:\s+cb eb\s+set 5,e
\s+[0-9a-f]+:\s+cb ec\s+set 5,h
\s+[0-9a-f]+:\s+cb ed\s+set 5,l
\s+[0-9a-f]+:\s+cb ee\s+set 5,\(hl\)
\s+[0-9a-f]+:\s+cb ef\s+set 5,a
\s+[0-9a-f]+:\s+cb f0\s+set 6,b
\s+[0-9a-f]+:\s+cb f1\s+set 6,c
\s+[0-9a-f]+:\s+cb f2\s+set 6,d
\s+[0-9a-f]+:\s+cb f3\s+set 6,e
\s+[0-9a-f]+:\s+cb f4\s+set 6,h
\s+[0-9a-f]+:\s+cb f5\s+set 6,l
\s+[0-9a-f]+:\s+cb f6\s+set 6,\(hl\)
\s+[0-9a-f]+:\s+cb f7\s+set 6,a
\s+[0-9a-f]+:\s+cb f8\s+set 7,b
\s+[0-9a-f]+:\s+cb f9\s+set 7,c
\s+[0-9a-f]+:\s+cb fa\s+set 7,d
\s+[0-9a-f]+:\s+cb fb\s+set 7,e
\s+[0-9a-f]+:\s+cb fc\s+set 7,h
\s+[0-9a-f]+:\s+cb fd\s+set 7,l
\s+[0-9a-f]+:\s+cb fe\s+set 7,\(hl\)
\s+[0-9a-f]+:\s+cb ff\s+set 7,a
