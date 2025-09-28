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

ASSEMBLY = avahi-sharp.dll

CLEANFILES = $(ASSEMBLY) $(ASSEMBLY).mdb $(ASSEMBLY).config

AVAHISOURCES = 				\
	$(srcdir)/AddressResolver.cs	\
	$(srcdir)/AssemblyInfo.cs	\
	$(srcdir)/BrowserBase.cs	\
	$(srcdir)/Client.cs		\
	$(srcdir)/ClientException.cs	\
	$(srcdir)/DomainBrowser.cs	\
	$(srcdir)/EntryGroup.cs		\
	$(srcdir)/HostNameResolver.cs	\
	$(srcdir)/RecordBrowser.cs	\
	$(srcdir)/ResolverBase.cs	\
	$(srcdir)/ServiceBrowser.cs	\
	$(srcdir)/ServiceResolver.cs	\
	$(srcdir)/ServiceTypeBrowser.cs	\
	$(srcdir)/Utility.cs

EXTRA_DIST =				\
	$(AVAHISOURCES)			\
	$(srcdir)/AvahiTest.cs 		\
	$(srcdir)/avahi.snk		\
	$(srcdir)/$(ASSEMBLY).config.in	\
	$(srcdir)/avahi-sharp-docs.source	\
	$(srcdir)/en/*.xml		\
	$(srcdir)/en/*/*.xml		\
	$(srcdir)/gencfg.sh

$(ASSEMBLY): $(AVAHISOURCES)
	$(AM_V_GEN)MONO_SHARED_DIR=. mcs -keyfile:$(srcdir)/avahi.snk -target:library -out:$@ -debug $(AVAHISOURCES) -r:Mono.Posix


$(ASSEMBLY).config: $(ASSEMBLY).config.in
	$(AM_V_GEN)$(srcdir)/gencfg.sh $(top_builddir)/avahi-client/libavahi-client.la \
		$(top_builddir)/avahi-common/libavahi-common.la \
		$(top_builddir)/avahi-glib/libavahi-glib.la < $< > $@

if HAVE_MONO
if HAVE_DBUS
all: $(ASSEMBLY) $(ASSEMBLY).config

if HAVE_MONODOC
update-docs: $(ASSEMBLY)
	$(AM_V_GEN)$(MONODOCER) -assembly:$(ASSEMBLY) -path:en

avahi-sharp-docs.zip: avahi-sharp-docs.tree

avahi-sharp-docs.tree: $(srcdir)/en/*/*
	$(AM_V_GEN)$(MDASSEMBLER) --out avahi-sharp-docs --ecma $(srcdir)/en

monodocdir = $(MONODOC_DIR)
monodoc_DATA = avahi-sharp-docs.zip avahi-sharp-docs.tree avahi-sharp-docs.source

endif

install-data-hook: $(ASSEMBLY)
	$(AM_V_GEN)MONO_SHARED_DIR=. $(GACUTIL) /i $(ASSEMBLY) /package avahi-sharp /gacdir $(libdir) /root $(DESTDIR)$(libdir)

uninstall-hook: $(ASSEMBLY)
	$(AM_V_GEN)MONO_SHARED_DIR=. $(GACUTIL) /u avahi-sharp /package avahi-sharp /gacdir $(libdir) /root $(DESTDIR)$(libdir)

endif
endif
