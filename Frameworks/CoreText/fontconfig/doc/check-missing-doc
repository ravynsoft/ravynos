#!/bin/sh
header=fontconfig-header
doc=fontconfig-doc
trap "rm $header $doc"  0 1 15
top_srcdir=${top_srcdir-".."}
(
cat $top_srcdir/fontconfig/*.h  | grep '^Fc' | 
 grep -v FcPublic | sed 's/[^a-zA-Z0-9].*//';
 cat $top_srcdir/fontconfig/*.h  | 
 sed -n 's/#define \(Fc[a-zA-Z]*\)(.*$/\1/p') |
 sort -u > $header

grep '@FUNC[+]*@' $top_srcdir/doc/*.fncs |
awk '{print $2}' |
sort -u > $doc

if cmp $doc $header > /dev/null; then
	exit 0
fi

echo \
'Library Export							Documentation'
diff -y $header $doc | grep '[<>]'
exit 1
