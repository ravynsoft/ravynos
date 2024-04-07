use strict;
use warnings FATAL => 'all';

use Test::More;

use Encode qw(encode decode find_encoding);
use Encode::Encoder qw(encoder);

local %Encode::ExtModule = %Encode::Config::ExtModule;

my @names = Encode->encodings(':all');

plan tests => 1 + 4 * @names;

my $emptyutf8;
eval { my $c = encoder($emptyutf8)->utf8; };
ok(!$@,"crashed encoding undef variable ($@)");

for my $name (@names) {
    my $enc = find_encoding($name);
    is($enc->encode(undef), undef, "find_encoding('$name')->encode(undef) returns undef");
    is($enc->decode(undef), undef, "find_encoding('$name')->decode(undef) returns undef");
    is(encode($name, undef), undef, "encode('$name', undef) returns undef");
    is(decode($name, undef), undef, "decode('$name', undef) returns undef");
}
