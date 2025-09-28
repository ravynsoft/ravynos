#
# $Id: isa.t,v 1.1 2015/04/02 12:08:24 dankogai Exp $
#
use strict;
use Encode qw/find_encoding/;
use Test::More;
my @enc = Encode->encodings(":all");
plan tests => 0+@enc;
isa_ok find_encoding($_), "Encode::Encoding" for @enc;

