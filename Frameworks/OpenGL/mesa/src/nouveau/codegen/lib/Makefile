ENVYAS ?= envyas

all: gf100.asm.h gk104.asm.h gk110.asm.h gm107.asm.h

gf100.asm.h: %.asm.h: %.asm
	$(ENVYAS) -a -W -mgf100 -Vgf100 $< -o $@
gk104.asm.h: %.asm.h: %.asm
	$(ENVYAS) -a -W -mgf100 -Vgk104 $< -o $@
gk110.asm.h: %.asm.h: %.asm
	$(ENVYAS) -a -W -mgk110 $< -o $@
gm107.asm.h: %.asm.h: %.asm
	$(ENVYAS) -a -W -mgm107 $< -o $@
