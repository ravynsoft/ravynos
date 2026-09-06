#!/bin/sh
outputfile=
outputdir=
domain=messages

spliteq() {
	arg=$1
	echo "${arg#*=}"
	#alternatives echo "$arg" | cut -d= -f2-
	# or echo "$arg" | sed 's/[^=]*=//'
}

syntax() {
	printf "%s\n" "Usage: xgettext [OPTION] [INPUTFILE]..."
	exit 1
}

show_version() {
	printf "%s\n" "xgettext (GNU gettext-tools-compatible) 99.99"
	exit 0
}

while [ $# -gt 0 ] ; do
	case $1 in
	#--files-from=*) readfile `spliteq "$1"`;;
	#-f) expectfilefrom=1;;
	--version) show_version;;
	-V) show_version;;
	--default-domain=*) domain=`spliteq "$1"` ;;
	-d) shift ; domain="$1" ;;
	--files-from=*) : ;;
	-f) shift ;;
	--directory=*) : ;;
	-D) shift ;;
	-o) shift ; outputfile="$1" ;;
	--output=*) outputfile=`spliteq "$1"` ;;
	--output-dir=*) outputdir=`spliteq "$1"` ;;
	-p) shift ; outputdir=`spliteq "$1"` ;;
	--language=*) : ;;
	-L) shift ;;
	--C) : ;;
	--c++) : ;;
	--from-code=*) : ;;
	--join-existing) : ;;
	-j) : ;;
	--exclude-file=*) : ;;
	-x) shift;;
	--add-comments=*) : ;;
	-cTAG) shift;;
	--add-comments) : ;;
	-c) : ;;
	--extract-all) : ;;
	-a) : ;;
	--keyword=*) : ;;
	-k*) : ;;
	--keyword) : ;;
	-k) : ;;
	--flag=*) : ;;
	--trigraphs) : ;;
	-T) : ;;
	--qt) : ;;
	--kde) : ;;
	--boost) : ;;
	--debug) : ;;
	--color) : ;;
	--color=*) : ;;
	--style=*) : ;;
	--no-escape) : ;;
	-e) : ;;
	--escape) : ;;
	-E) : ;;
	--force-po) force=1 ;;
	--indent) : ;;
	-i) : ;;
	--no-location) : ;;
	--add-location) : ;;
	-n) : ;;
	--strict) : ;;
	--properties-output) : ;;
	--stringtable-output) : ;;
	--width=*) : ;;
	-w) : ;;
	--no-wrap) : ;;
	--sort-output) : ;;
	-s) : ;;
	--sort-by-file) : ;;
	-F) : ;;
	--omit-header) : ;;
	--copyright-holder=*) : ;;
	--foreign-user) : ;;
	--package-name=*) : ;;
	--package-version=*) : ;;
	--msgid-bugs-address=*) : ;;
	--msgstr-prefix*) : ;;
	-m*) : ;;
	--msgstr-suffix*) : ;;
	-M*) : ;;
	--help) syntax ;;
	-h) syntax ;;
	esac
	shift
done

[ "$outputfile" = "-" ] && exit 0
[ -z "$outputfile" ] && outputfile=${domain}.po

if [ -z "$outputdir" ]; then
	touch $outputfile
else
	touch $outputdir/$outputfile
fi
