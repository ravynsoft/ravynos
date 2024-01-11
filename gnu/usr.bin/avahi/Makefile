.include <bsd.prog.mk>

.MAKEFLAGS=
CONFIGURE_ARGS+=        --disable-python --disable-pygobject
CONFIGURE_ARGS+=        --disable-python-dbus
CONFIGURE_ARGS+=        --disable-doxygen-doc --disable-monodoc --disable-mono
CONFIGURE_ARGS+=        --enable-dbm --disable-gdbm --disable-glib
CONFIGURE_ARGS+=        --disable-gtk3 --disable-gtk
CONFIGURE_ARGS+=        --without-libintl-prefix --disable-nls
#CONFIGURE_ARGS+=        --enable-compat-libdns_sd --enable-compat-howl
GMAKE=                  ${DESTDIR}/legacy/bin/gmake
GMAKE_ARGS+=            LDFLAGS="-L${OBJTOP}/gnu/usr.bin/dbus/dbus/.libs"

all: gnu_configure .WAIT build
build: gnu_build
install: gnu_install

