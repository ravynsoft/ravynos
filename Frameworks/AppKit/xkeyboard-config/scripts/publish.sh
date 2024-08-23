#!/bin/sh

if [ $# -lt 1 ] ; then
	echo "Usage: $0 filename (acc)"
	exit 1
fi

file=$1
acc=${2:-svu}

scp "$file" ${acc}@www.x.org:/home/svu/public_html
echo "Check on http://www.x.org/~${acc}"
