# The Alpha has its own version of .set.
# hppa SOM does not output non-global absolute symbols.
#xfail: alpha-*-* [is_som_format]
#readelf: -s
#name: .set with expression

#...
.*ABS.*shift.*
#pass
