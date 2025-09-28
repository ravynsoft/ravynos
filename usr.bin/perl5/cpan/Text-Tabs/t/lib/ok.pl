use strict; use warnings;
my $_t;
sub ok { print +( $_[0] ? 'ok ' : 'not ok ' ) . ++$_t . ( $_[1] ? " - $_[1]\n" : "\n" ); !!$_[0] }
sub diag { s/^/# /gm, s/\Z.*/\n/s, print for join '', map +( defined $_ ? $_ : 'undef' ), @_ }
1;
