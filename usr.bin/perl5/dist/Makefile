.include <src.opts.mk>

MODULES= \
	Attribute-Handlers \
	Carp \
	Data-Dumper \
	Dumpvalue \
	Env \
	Exporter \
	ExtUtils-CBuilder \
	ExtUtils-ParseXS \
	Filter-Simple \
	FindBin \
	I18N-Collate \
	I18N-LangTags \
	Locale-Maketext \
	Math-Complex \
	Module-CoreList \
	Net-Ping \
	PathTools \
	Safe \
	Search-Dict \
	SelfLoader \
	Storable \
	Term-Complete \
	Term-ReadLine \
	Test \
	Text-Abbrev \
	Thread-Queue \
	Thread-Semaphore \
	Tie-File \
	Unicode-Normalize \
	autouse \
	base \
	constant \
	encoding-warnings \
	if \
	lib 

all: build
build: 
.for module in ${MODULES}
	if [ -d ${.CURDIR}/${module}/lib ]; then \
		cp -Rv ${.CURDIR}/${module}/lib/* ${PERL5LIB}/; \
		if [ -f ${.CURDIR}/${module}/*.pm ]; then \
			cp -fv ${.CURDIR}/${module}/*.pm ${PERL5LIB}/; \
		fi; \
	else \
		mkdir -pv ${PERL5LIB}/${module:S/-/\//g}; \
		cp -Rv ${.CURDIR}/${module}/* ${PERL5LIB}/${module:S/-/\//g}; \
	fi
.endfor

install: .PHONY
includes: .PHONY
