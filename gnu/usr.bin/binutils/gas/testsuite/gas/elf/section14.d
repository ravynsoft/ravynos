#name: array sections
#as: --no-pad-sections
#readelf: -S --wide
# The h8300 port issues a warning message for new sections created
# without atrributes.
#xfail: h8300-*

There are [0-9]+ section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Addr(ess|) +Off +Size +ES +Flg +Lk +Inf +Al
 +\[ 0\] +NULL +0+ +0+ +0+ +0+ +0 +0 +0
#pass
