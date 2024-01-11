# This file is part of avahi.
#
# avahi is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# avahi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with avahi; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.

ASSEMBLY = avahi-ui-sharp.dll

CLEANFILES = $(ASSEMBLY) $(ASSEMBLY).mdb $(ASSEMBLY).config bssh.exe

AVAHISOURCES = 				\
	$(srcdir)/ServiceDialog.cs

EXTRA_DIST =					\
	$(AVAHISOURCES)				\
	$(srcdir)/avahi-ui-sharp-docs.source	\
	$(srcdir)/en/*.xml			\
	$(srcdir)/en/*/*.xml			\
	$(srcdir)/gencfg.sh                     \
	$(srcdir)/$(ASSEMBLY).config.in         \
	$(srcdir)/bssh.cs

$(ASSEMBLY): $(AVAHISOURCES)
	$(AM_V_GEN)mcs -keyfile:$(top_srcdir)/avahi-sharp/avahi.snk -target:library -out:$@ -debug $(AVAHISOURCES) -pkg:gtk-sharp-2.0 -r:$(top_builddir)/avahi-sharp/avahi-sharp.dll -r:Mono.Posix

$(ASSEMBLY).config: $(ASSEMBLY).config.in
	$(AM_V_GEN)$(srcdir)/gencfg.sh $(top_builddir)/avahi-common/libavahi-common.la < $< > $@

bssh.exe: $(srcdir)/bssh.cs $(ASSEMBLY)
	$(AM_V_GEN)mcs -out:$@ $(srcdir)/bssh.cs -r:./avahi-ui-sharp.dll -r:../avahi-sharp/avahi-sharp.dll -pkg:gtk-sharp-2.0 -r:Mono.Posix

if HAVE_MONO
if HAVE_DBUS
if HAVE_GTK
all: $(ASSEMBLY) $(ASSEMBLY).config bssh.exe

if HAVE_MONODOC
update-docs: $(ASSEMBLY)
	$(AM_V_GEN)$(MONODOCER) -assembly:$(ASSEMBLY) -path:en

avahi-ui-sharp-docs.zip: avahi-ui-sharp-docs.tree

avahi-ui-sharp-docs.tree: $(srcdir)/en/*/*
	$(AM_V_GEN)$(MDASSEMBLER) --out avahi-ui-sharp-docs --ecma $(srcdir)/en

monodocdir = $(MONODOC_DIR)
monodoc_DATA = avahi-ui-sharp-docs.zip avahi-ui-sharp-docs.tree avahi-ui-sharp-docs.source

endif

install-data-hook: $(ASSEMBLY)
	$(GACUTIL) /i $(ASSEMBLY) /package avahi-ui-sharp /gacdir $(libdir) /root $(DESTDIR)$(libdir)

uninstall-hook: $(ASSEMBLY)
	$(GACUTIL) /u avahi-ui-sharp /package avahi-ui-sharp /gacdir $(libdir) /root $(DESTDIR)$(libdir)

endif
endif
endif
