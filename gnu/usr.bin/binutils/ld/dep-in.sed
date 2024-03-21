:loop
/\\$/N
s/\\\n */ /g
t loop

s! \./! !g
s!@INCDIR@!$(INCDIR)!g
s!@TOPDIR@/include!$(INCDIR)!g
s!@BFDDIR@!$(BFDDIR)!g
s!@TOPDIR@/bfd!$(BFDDIR)!g
s!@SRCDIR@/!!g
s! \.\./bfd/hosts/[^ ]*\.h! !g
s! \.\./intl/libintl\.h!!g

s/ *$//
s/  */ /g
s/^ */A/
s/ / \\\
B/g
$s/$/ \\/
