
BINDIR?=	/bin
SBINDIR?=       /sbin
SYSCONFDIR?=    /etc
LOCALSTATEDIR?= /var

CONFIGURE_ARGS+=        --prefix=$(PREFIX)
CONFIGURE_ARGS+=        --bindir=$(BINDIR)
CONFIGURE_ARGS+=        --sbindir=$(SBINDIR)
CONFIGURE_ARGS+=        --sysconfdir=$(SYSCONFDIR)
CONFIGURE_ARGS+=        --localstatedir=$(LOCALSTATEDIR)

GMAKE=          gmake

gnu_configure:
	$(.CURDIR)/configure $(CONFIGURE_ARGS)

build:
	$(GMAKE) $(GMAKE_ARGS)

clean:
	$(GMAKE) clean

.include "../Makefile.inc"

