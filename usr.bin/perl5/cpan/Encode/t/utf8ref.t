#
# $Id: utf8ref.t,v 1.2 2016/10/28 05:03:52 dankogai Exp $
#

use strict;
use warnings;
use Encode;
use Test::More;
plan tests => 12;
#plan 'no_plan';

# my $a = find_encoding('ASCII');
my $u = find_encoding('UTF-8');
my $r = [];
no warnings 'uninitialized';
is encode_utf8($r), ''.$r;
is $u->encode($r), ''.$r;
$r = {};
is decode_utf8($r), ''.$r;
is $u->decode($r), ''.$r;
use warnings 'uninitialized';

is encode_utf8(undef), undef;
is decode_utf8(undef), undef;

is encode_utf8(''), '';
is decode_utf8(''), '';

is Encode::encode('utf8', undef), undef;
is Encode::decode('utf8', undef), undef;

is Encode::encode('utf8', ''), '';
is Encode::decode('utf8', ''), '';
