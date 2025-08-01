.include <src.opts.mk>

SUBDIR=	alias \
	apply \
	asa \
	aslutil \
	awk \
	backlight \
	banner \
	basename \
	beep \
	bintrans \
	brandelf \
	bsdcat \
	bsddialog \
	bsdiff \
	bzip2 \
	bzip2recover \
	cap_mkdb \
	chat \
	chpass \
	cksum \
        cmake \
	cmp \
	col \
	colrm \
	column \
	comm \
	compress \
	csplit \
	ctlstat \
	cut \
	diff \
	dirname \
	dtc \
	du \
	elfctl \
	elfdump \
	enigma \
	env \
	etdump \
	expand \
	false \
	fetch \
	find \
	fmt \
	fold \
	fstat \
	fsync \
	gcore \
	gencat \
	getaddrinfo \
	getconf \
	getent \
	getopt \
	grep \
	gzip \
	head \
	hexdump \
	id \
	ident \
	ipcrm \
	ipcs \
	join \
	jot \
	killall \
	ktrace \
	ktrdump \
	lam \
	ldd \
	leave \
	less \
	lessecho \
	lesskey \
	limits \
	locale \
	localedef \
	lock \
	lockf \
	logger \
	login \
	logins \
	logname \
	look \
	lsvfs \
	lzmainfo \
	m4 \
	mandoc \
	mdo \
	mesg \
	migcom \
	mididump \
	ministat \
	mkdep \
	mkfifo \
	mkimg \
	mktemp \
	mkuzip \
	mt \
        nasm \
	ncal \
	ncurses \
	netstat \
	newgrp \
	nfsstat \
	nice \
        ninja \
	nl \
	nohup \
	notifyutil \
	pagesize \
	passwd \
	paste \
	patch \
	pathchk \
	perror \
	posixmqcontrol \
	posixshmcontrol \
	pr \
	printenv \
	printf \
	proccontrol \
	procstat \
	protect \
	rctl \
	renice \
	resizewin \
	rev \
	revoke \
	rpcinfo \
	rs \
	runat \
	rup \
	ruptime \
	rusers \
	rwall \
	rwho \
	script \
	sdiff \
	sed \
	seq \
	showmount \
	sockstat \
	soelim \
	sort \
	split \
	stat \
	stdbuf \
	strings \
	su \
	systat \
	tail \
	tar \
	tcopy \
	tee \
	time \
	tip \
	top \
	touch \
	tr \
	true \
	truncate \
	ts \
	tsort \
	tty \
	uname \
	unexpand \
	uniq \
	unzip \
	units \
	unvis \
	vis \
	vmstat \
	w \
	wall \
	wc \
	wg \
	what \
	whereis \
	which \
	whois \
	write \
	xargs \
	xinstall \
	xo \
	xpc-tests \
	xz \
	xzdec \
	yes \
        zsh \
	zstd

# NB: keep these sorted by MK_* knobs

SUBDIR.${MK_ACCT}+=	lastcomm
SUBDIR.${MK_AT}+=	at
SUBDIR.${MK_BLUETOOTH}+=	bluetooth
SUBDIR.${MK_BSD_CPIO}+=	cpio
SUBDIR.${MK_CALENDAR}+=	calendar
.if ${MK_CLANG} != "no" || ${MK_LLVM_BINUTILS} != "no" || \
    ${MK_LLD} != "no" || ${MK_LLDB} != "no"
SUBDIR+=	clang
.endif
SUBDIR.${MK_DIALOG}+=	dpv
SUBDIR.${MK_EE}+=	ee
SUBDIR.${MK_FILE}+=	file
SUBDIR.${MK_FINGER}+=	finger
SUBDIR.${MK_FTP}+=	ftp
SUBDIR.${MK_GAMES}+=	caesar
SUBDIR.${MK_GAMES}+=	factor
SUBDIR.${MK_GAMES}+=	fortune
SUBDIR.${MK_GAMES}+=	grdc
SUBDIR.${MK_GAMES}+=	morse
SUBDIR.${MK_GAMES}+=	number
SUBDIR.${MK_GAMES}+=	pom
SUBDIR.${MK_GAMES}+=	primes
SUBDIR.${MK_GAMES}+=	random
SUBDIR+=		gh-bc
.if ${MK_GNU_DIFF} == "no"
SUBDIR+=		diff3
.endif
SUBDIR.${MK_HESIOD}+=	hesinfo
SUBDIR.${MK_ICONV}+=	iconv
SUBDIR.${MK_ICONV}+=	mkcsmapper
SUBDIR.${MK_ICONV}+=	mkesdb
SUBDIR.${MK_ISCSI}+=	iscsictl
SUBDIR.${MK_KDUMP}+=	kdump
SUBDIR.${MK_KDUMP}+=	truss
.if ${MK_MITKRB5} == "no"
SUBDIR.${MK_KERBEROS_SUPPORT}+=	compile_et
.endif
SUBDIR.${MK_LDNS_UTILS}+=	drill
SUBDIR.${MK_LDNS_UTILS}+=	host
SUBDIR.${MK_LIB32}+=	ldd32
SUBDIR.${MK_LOCATE}+=	locate
# XXX msgs?
SUBDIR.${MK_MAIL}+=	biff
SUBDIR.${MK_MAIL}+=	from
SUBDIR.${MK_MAIL}+=	mail
SUBDIR.${MK_MAIL}+=	msgs
SUBDIR.${MK_MAKE}+=	bmake
SUBDIR.${MK_MAN_UTILS}+=	man
SUBDIR.${MK_NETCAT}+=	nc
SUBDIR.${MK_NETLINK}+=	genl
SUBDIR.${MK_NIS}+=	ypcat
SUBDIR.${MK_NIS}+=	ypmatch
SUBDIR.${MK_NIS}+=	ypwhich
SUBDIR.${MK_OPENSSH}+=	ssh-copy-id
SUBDIR.${MK_QUOTAS}+=	quota
SUBDIR.${MK_SENDMAIL}+=	vacation
SUBDIR.${MK_TALK}+=	talk
SUBDIR.${MK_TELNET}+=	telnet
SUBDIR.${MK_TESTS_SUPPORT}+=	kyua
SUBDIR.${MK_TESTS}+=	tests
SUBDIR.${MK_TEXTPROC}+=	ul
SUBDIR.${MK_TFTP}+=	tftp
.if ${MK_LLVM_BINUTILS} == "no"
# Only build the elftoolchain tools if we aren't using the LLVM ones.
SUBDIR.${MK_TOOLCHAIN}+=	addr2line
SUBDIR.${MK_TOOLCHAIN}+=	ar
SUBDIR.${MK_TOOLCHAIN}+=	nm
SUBDIR.${MK_TOOLCHAIN}+=	readelf
SUBDIR.${MK_TOOLCHAIN}+=	size
.endif
SUBDIR.${MK_TOOLCHAIN}+=	c89
SUBDIR.${MK_TOOLCHAIN}+=	c99
SUBDIR.${MK_TOOLCHAIN}+=	ctags
.if ${MK_LLVM_CXXFILT} == "no"
SUBDIR.${MK_TOOLCHAIN}+=	cxxfilt
.endif
# ELF Tool Chain elfcopy required for EFI objects (PR280771)
SUBDIR.${MK_TOOLCHAIN}+=	elfcopy
SUBDIR.${MK_TOOLCHAIN}+=	file2c
SUBDIR.${MK_TOOLCHAIN}+=	gprof
SUBDIR.${MK_TOOLCHAIN}+=	indent
SUBDIR.${MK_TOOLCHAIN}+=	lex
SUBDIR.${MK_TOOLCHAIN}+=	lorder
SUBDIR.${MK_TOOLCHAIN}+=	mkstr
SUBDIR.${MK_TOOLCHAIN}+=	rpcgen
SUBDIR.${MK_TOOLCHAIN}+=	unifdef
SUBDIR.${MK_TOOLCHAIN}+=	xstr
SUBDIR.${MK_TOOLCHAIN}+=	yacc
SUBDIR.${MK_VI}+=	vi
SUBDIR.${MK_VT}+=	vtfontcvt
SUBDIR.${MK_USB}+=	usbhidaction
SUBDIR.${MK_USB}+=	usbhidctl
SUBDIR.${MK_UTMPX}+=	last
SUBDIR.${MK_UTMPX}+=	users
SUBDIR.${MK_UTMPX}+=	who
SUBDIR.${MK_OFED}+=	ofed

SUBDIR+= \
	meson \
	turbo-editor \
	python

SUBDIR_DEPEND_meson=	python

.include <bsd.arch.inc.mk>

SUBDIR_PARALLEL=

.include <bsd.subdir.mk>
