GMAKE_ARGS+=		INSTALL=${OBJTOP}/tmp/legacy/bin/install
#distribute: gnu_distribute

.include <bsd.prog.mk>

.MAKEFLAGS=
CONFIGURE_ARGS+=        --disable-python --disable-pygobject --disable-qt4
CONFIGURE_ARGS+=        --disable-python-dbus --disable-dbus --disable-libdaemon
CONFIGURE_ARGS+=        --disable-doxygen-doc --disable-monodoc --disable-mono
CONFIGURE_ARGS+=        --enable-dbm --disable-gdbm --disable-glib --disable-qt5
CONFIGURE_ARGS+=        --disable-gobject --disable-gtk3 --disable-gtk
CONFIGURE_ARGS+=        --without-libintl-prefix --disable-nls --disable-libevent
#CONFIGURE_ARGS+=        --enable-compat-libdns_sd --enable-compat-howl
CONFIGURE_ENV+=		GOBJECT_CFLAGS="-I." GOBJECT_LIBS="-L."
CONFIGURE_ENV+=		have_pkg_config=yes PKG_CONFIG=/usr/bin/true

GMAKE=                  ${OBJTOP}/tmp/legacy/bin/gmake
GMAKE_ARGS+=            LDFLAGS="-L${OBJTOP}/gnu/usr.bin/dbus/dbus/.libs"

all: gnu_configure .WAIT build
build: gnu_build
install: gnu_install
