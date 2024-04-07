# $Id: from_to.t,v 1.1 2006/01/15 15:06:36 dankogai Exp $
use strict;
use Test::More tests => 3;
use Encode qw(encode from_to);

my $foo = encode("utf-8", "\x{5abe}");
from_to($foo, "utf-8" => "latin1", Encode::FB_HTMLCREF);
ok !Encode::is_utf8($foo);
is $foo, '&#23230;';

my $bar = encode("latin-1", "\x{5abe}", Encode::FB_HTMLCREF);
is $bar, '&#23230;';
