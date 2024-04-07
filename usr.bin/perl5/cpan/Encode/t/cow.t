#
# $Id: cow.t,v 1.2 2016/08/04 03:15:58 dankogai Exp $
#
use strict;
use Encode ();
use Test::More tests => 4;


my %a = ( "L\x{c3}\x{a9}on" => "acme" );
my ($k) = ( keys %a );
Encode::_utf8_on($k);
my %h = ( $k => "acme" );
is $h{"L\x{e9}on"} => 'acme';
($k) = ( keys %h );
Encode::_utf8_off($k);
%a = ( $k => "acme" );
is $h{"L\x{e9}on"} => 'acme';
# use Devel::Peek;
# Dump(\%h);

{ # invalid input to encode/decode/from_to should not affect COW-shared scalars
	my $x = Encode::decode('UTF-8', "\303\244" x 4);
	my $orig = "$x"; # non-COW copy
	is($x, $orig, "copy of original string matches");
	{ my $y = $x; Encode::from_to($y, "UTF-8", "iso-8859-1"); }
	is($x, $orig, "original scalar unmodified after from_to() call");
}
