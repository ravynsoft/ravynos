.include <src.opts.mk>

MODULES=	Archive-Tar \
		AutoLoader \
		CPAN \
		CPAN-Meta \
		CPAN-Meta-Requirements \
		CPAN-Meta-YAML \
		Compress-Raw-Bzip2 \
		Compress-Raw-Zlib \
		Config-Perl-V \
		DB_File \
		Digest \
		Digest-MD5 \
		Digest-SHA \
		Encode \
		ExtUtils-Constant \
		ExtUtils-Install \
		ExtUtils-MakeMaker \
		ExtUtils-Manifest \
		ExtUtils-PL2Bat \
		File-Fetch \
		File-Path \
		File-Temp \
		Filter-Util-Call \
		Getopt-Long \
		HTTP-Tiny \
		IO-Compress \
		IO-Socket-IP \
		IO-Zlib \
		IPC-Cmd \
		IPC-SysV \
		JSON-PP \
		Locale-Maketext-Simple \
		MIME-Base64 \
		Math-BigInt \
		Math-BigInt-FastCalc \
		Math-BigRat \
		Memoize \
		Module-Load \
		Module-Load-Conditional \
		Module-Loaded \
		Module-Metadata \
		NEXT \
		Params-Check \
		Perl-OSType \
		PerlIO-via-QuotedPrint \
		Pod-Checker \
		Pod-Escapes \
		Pod-Perldoc \
		Pod-Simple \
		Pod-Usage \
		Scalar-List-Utils \
		Socket \
		Sys-Syslog \
		Term-ANSIColor \
		Term-Cap \
		Test-Harness \
		Test-Simple \
		Text-Balanced \
		Text-ParseWords \
		Text-Tabs \
		Tie-RefHash \
		Time-Local \
		Time-Piece \
		Unicode-Collate \
		Win32 \
		Win32API-File \
		autodie \
		bignum \
		experimental \
		libnet \
		parent \
		perlfaq \
		podlators \
		version

all: build
build:
.for module in ${MODULES}
	if [ -d ${.CURDIR}/${module}/lib ]; then \
		cp -Rv ${.CURDIR}/${module}/lib/* ${PERL5LIB}/; \
	else \
		mkdir -pv ${PERL5LIB}/${module:S/-/\//g}; \
		cp -Rv ${.CURDIR}/${module}/* ${PERL5LIB}/${module:S/-/\//g}; \
	fi
.endfor

install:
includes: .PHONY

