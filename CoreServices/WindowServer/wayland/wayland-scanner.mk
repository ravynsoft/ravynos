%-protocol.c : $(wayland_protocoldir)/%.xml
	$(AM_V_GEN)$(wayland_scanner) code $< $@

%-server-protocol.h : $(wayland_protocoldir)/%.xml
	$(AM_V_GEN)$(wayland_scanner) server-header $< $@

%-client-protocol.h : $(wayland_protocoldir)/%.xml
	$(AM_V_GEN)$(wayland_scanner) client-header $< $@
