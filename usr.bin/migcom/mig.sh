#!/bin/sh
# Front-end to migcom.
# Since migcom only deals with stdin, we need to
# run things through the pre-processor.
#
# This is based on the Mac OS man page for mig.
# Which is annoying, because the options aren't
# parsable using getopts
PROG="$0"

QUIET="-Q"	# -q or -Q
VERBOSE=""	# -v or -V
EVLOG="-l"	# -l or -L
ANSI="-k"	# -k or -K
SERVSYM="-S"	# -S or -s
USERPREFIX=""	# -i <prefix>
USERRPC=""	# -user <path>
SRVRPC=""	# -server <path>
USRHDR=""	# -header <path>
SYSHDR=""	# -sheader <path>
IHDR=""		# -iheader <path>
GHDR=""		# -dheader <path>
STACKMAX=""	# -maxonstack <value>
SPLIT=""	# -split
ARCH=""		# -arch <arch>, ignored
MD=""		# -MD, passed to cc
CPP=""		# -cpp <path>, ignored
CC="/usr/bin/cc"	# -cc <path>
MIGCOM="/usr/bin/migcom"	# -migcom <path>
# This is a hack, and I'm not entirely sure it's
# valid.
SYSROOT=$(expr $(dirname $0) : "\(.*\)/usr/bin") || SYSROOT=""

while [ $# -gt 1 ]
do
    case "$1" in
	-q)	QUIET="-q" ; shift ;;
	-Q)	QUIET="-Q" ; shift ;;
	-l)	EVLOG="-l" ; shift ;;
	-L)	EVLOG="-L" ; shift ;;
	-k)	ANSI="-k" ; shift ;;
	-K)	ANSI="-K" ; shift ;;
	-s)	SERVSYM="-s" ; shift ;;
	-S)	SERVSYM="-S" ; shift ;;
	-V)	VERBOSE="-V" ; shift ;;
	-v)	VERBOSE="-v" ; shift ;;
	-split)	SPLIT="-split" ; shift ;;
	-MD)	MD="-MD" ; shift ;;
	-i)	if [ $# -le 2 ] ; then usage ; fi ; USERPREFIX="-i $2" ; shift 2 ;;
	-user)	if [ $# -le 2 ] ; then usage ; fi ; USERRPC="-user $2" ; shift 2 ;;
	-server)	if [ $# -le 2 ]; then usage ; fi ; SRVRPC="-server $2" ; shift 2 ;;
	-header)	if [ $# -le 2 ]; then usage ; fi ; USRHDR="-header $2" ; shift 2 ;;
	-sheader)	if [ $# -le 2 ]; then usage ; fi ; SYSHDR="-sheader $2" ; shift 2 ;;
	-iheader)	if [ $# -le 2 ]; then usage ; fi ; IHDR="-iheader $2" ; shift 2 ;;
	-dheader)	if [ $# -le 2 ]; then usage ; fi ; GHDR="-dheader $2" ; shift 2 ;;
	-maxonstack)	if [ $# -le 2 ]; then usage ; fi ; STACKMAX="-maxonstack $2" ; shift 2 ;;
	-cpp)	if [ $# -le 2 ]; then usage ; fi ; shift 2 ;;
	-cc)	if [ $# -le 2 ]; then usage ; fi ; CC="$2" ; shift 2 ;;
	-migcom)	if [ $# -le 2 ]; then usage ; fi ; MIGCOM="$2" ; shift 2 ;;
	-isysroot)	if [ $# -le 2 ]; then usage ; fi ; SYSROOT="$2" ; shift 2 ;;
	-*)	CFLAGS="${CFLAGS} $1" ; shift ;;
    esac
done

CC="${SYSROOT}${CC}"
MIGCOM="${SYSROOT}${MIGCOM}"

if [ $# -ne 1 ]; then
    usage
fi

input_file="$1"

tmpfile=${TMPDIR:-/tmp}/migcom-$$.c

( echo "#line 1 \"${input_file}\"" ; cat ${input_file} ) > ${tmpfile}
if ! ${CC} ${MD} ${CFLAGS} -E ${tmpfile} |
	${MIGCOM} ${QUIET} ${EVLOG} ${ANSI} ${SERVSYM} \
	    ${SPLIT} ${USERPREFIX} ${VERBOSE} \
	    ${USERRPC} ${SRVRPC} \
	    ${USRHDR} ${SYSHDR} \
	    ${IHDR} ${GHDR} \
	    ${STACKMAX}
then
    rm -f ${tmpfile}
    exit 1
fi
exit 0
