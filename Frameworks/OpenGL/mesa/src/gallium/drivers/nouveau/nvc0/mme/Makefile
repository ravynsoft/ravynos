ENVYAS?=envyas
TARGETS=com9097.mme.h com90c0.mme.h

all: $(TARGETS)

$(TARGETS): %.mme.h: %.mme
	$(ENVYAS) -a -w -mmacro $< -o $@
