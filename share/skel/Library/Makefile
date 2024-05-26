DIR= Library

clean: .PHONY
all: .PHONY
install:
	mkdir -p ${OBJTOP}/tmp/usr/share/skel/${DIR}
.for d in Fonts LaunchAgents
	mkdir -p ${OBJTOP}/tmp/usr/share/skel/${DIR}/${d}
.endfor

distribute:
	mkdir -p ${OBJTOP}/release/dist/ravynOS/usr/share/skel/${DIR}
.for d in Fonts LaunchAgents
	mkdir -p ${OBJTOP}/release/dist/ravynOS/usr/share/skel/${DIR}/${d}
.endfor

