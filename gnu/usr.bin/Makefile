.include <src.opts.mk>

SUBDIR+=                dbus
SUBDIR.${MK_DIALOG}+=	dialog
SUBDIR.${MK_GNU_DIFF}+=	diff3
SUBDIR.${MK_TESTS}+=	tests
SUBDIR+=                avahi
SUBDIR+=                binutils
SUBDIR+=                gmake
#SUBDIR+=                libtool

SUBDIR_PARALLEL=

SUBDIR_DEPEND_avahi=   dbus

.include <bsd.subdir.mk>
