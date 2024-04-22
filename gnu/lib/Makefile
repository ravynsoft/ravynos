distribute:
	@echo Nothing to distribute

.include <src.opts.mk>

SUBDIR+=		fake-autotools
SUBDIR+=                libobjc2
SUBDIR+=                gettext
SUBDIR.${MK_DIALOG}+=	libdialog
SUBDIR.${MK_TESTS}+=	tests

SUBDIR_PARALLEL=

.include <bsd.subdir.mk>
