#!./perl -- -*- mode: cperl; cperl-indent-level: 4 -*-

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config;
    if (($Config::Config{'extensions'} !~ m!\bList/Util\b!) ){
	print "1..0 # Skip -- Perl configured without List::Util module\n";
	exit 0;
    }
}

use strict;

$|=1;

my @prgs;
{
    local $/;
    @prgs = split "########\n", <DATA>;
    close DATA;
}

use Test::More;

require "dumpvar.pl";

sub unctrl    { print dumpvar::unctrl($_[0]), "\n" }
sub uniescape { print dumpvar::uniescape($_[0]), "\n" }
sub stringify { print dumpvar::stringify($_[0]), "\n" }
sub dumpvalue { 
	# Call main::dumpValue exactly as the perl5db.pl calls it.
        local $\ = '';
        local $, = '';
        local $" = ' ';
        my @params = @_;
        &main::dumpValue(\@params,-1);
}

package Foo;

sub new { my $class = shift; bless [ @_ ], $class }

package Bar;

sub new { my $class = shift; bless [ @_ ], $class }

use overload '""' => sub { "Bar<@{$_[0]}>" };

package Tyre;

sub TIESCALAR{bless[]}
# other methods intentionally omitted

package Kerb;

sub TIEHASH{bless{}}
# other methods intentionally omitted

package main;

my $foo = Foo->new(1..5);
my $bar = Bar->new(1..5);

for (@prgs) {
    my($prog, $expected) = split(/\nEXPECT\n?/, $_);
    # TODO: dumpvar::stringify() is controlled by a pile of package
    # dumpvar variables: $printUndef, $unctrl, $quoteHighBit, $bareStringify,
    # and so forth.  We need to test with various settings of those.
    my $out = tie *STDOUT, 'TieOut';
    eval $prog;
    my $ERR = $@;
    untie $out;
    if ($ERR) {
        ok(0, "$prog - $ERR");
    } else {
	if ($expected =~ m:^/:) {
	    like($$out, $expected, $prog);
	} else {
	    is($$out, $expected, $prog);
	}
    }
}

done_testing();

package TieOut;

sub TIEHANDLE {
    bless( \(my $self), $_[0] );
}

sub PRINT {
    my $self = shift;
    $$self .= join('', @_);
}

sub read {
    my $self = shift;
    substr( $$self, 0, length($$self), '' );
}

__END__
unctrl("A");
EXPECT
A
########
unctrl("\cA");
EXPECT
^A
########
uniescape("A");
EXPECT
A
########
uniescape("\x{100}");
EXPECT
\x{0100}
########
stringify(undef);
EXPECT
undef
########
stringify("foo");
EXPECT
'foo'
########
stringify("\cA");
EXPECT
"\cA"
########
stringify(*a);
EXPECT
*main::a
########
stringify(\undef);
EXPECT
/^'SCALAR\(0x[0-9a-f]+\)'$/i
########
stringify([]);
EXPECT
/^'ARRAY\(0x[0-9a-f]+\)'$/i
########
stringify({});
EXPECT
/^'HASH\(0x[0-9a-f]+\)'$/i
########
stringify(sub{});
EXPECT
/^'CODE\(0x[0-9a-f]+\)'$/i
########
stringify(\*a);
EXPECT
/^'GLOB\(0x[0-9a-f]+\)'$/i
########
stringify($foo);
EXPECT
/^'Foo=ARRAY\(0x[0-9a-f]+\)'$/i
########
stringify($bar);
EXPECT
/^'Bar=ARRAY\(0x[0-9a-f]+\)'$/i
########
dumpValue(undef);
EXPECT
undef
########
dumpValue(1);
EXPECT
1
########
dumpValue("\cA");
EXPECT
"\cA"
########
dumpValue("\x{100}");
EXPECT
'\x{0100}'
########
dumpValue("1\n2\n3");
EXPECT
'1
2
3'
########
dumpValue([1..3],1);
EXPECT
0  1
1  2
2  3
########
dumpValue([1..3]);
EXPECT
0  1
1  2
2  3
########
dumpValue({1..4},1);
EXPECT
1 => 2
3 => 4
########
dumpValue({1..4});
EXPECT
1 => 2
3 => 4
########
dumpValue($foo,1);
EXPECT
0  1
1  2
2  3
3  4
4  5
########
dumpValue($foo);
EXPECT
0  1
1  2
2  3
3  4
4  5
########
dumpValue($bar,1);
EXPECT
0  1
1  2
2  3
3  4
4  5
########
dumpValue($bar);
EXPECT
0  1
1  2
2  3
3  4
4  5
########
dumpvalue("a");
EXPECT
0  'a'
########
dumpvalue("\cA");
EXPECT
0  "\cA"
########
dumpvalue("\x{100}");
EXPECT
0  '\x{0100}'
########
dumpvalue(undef);
EXPECT
0  undef
########
dumpvalue("foo");
EXPECT
0  'foo'
########
dumpvalue(\undef);
EXPECT
/0  SCALAR\(0x[0-9a-f]+\)\n   -> undef\n/i
########
dumpvalue(\\undef);
EXPECT
/0  REF\(0x[0-9a-f]+\)\n   -> SCALAR\(0x[0-9a-f]+\)\n         -> undef\n/i
########
dumpvalue([]);
EXPECT
/0  ARRAY\(0x[0-9a-f]+\)\n     empty array/i
########
dumpvalue({});
EXPECT
/0  HASH\(0x[0-9a-f]+\)\n\s+empty hash/i
########
dumpvalue(sub{});
EXPECT
/0  CODE\(0x[0-9a-f]+\)\n   -> &CODE\(0x[0-9a-f]+\) in /i
########
dumpvalue(\*a);
EXPECT
/0  GLOB\(0x[0-9a-f]+\)\n   -> \*main::a\n/i
########
dumpvalue($foo);
EXPECT
/0  Foo=ARRAY\(0x[0-9a-f]+\)\n   0  1\n   1  2\n   2  3\n   3  4\n   4  5\n/i
########
dumpvalue($bar);
EXPECT
/0  Bar=ARRAY\(0x[0-9a-f]+\)\n   0  1\n   1  2\n   2  3\n   3  4\n   4  5\n/i
########
dumpvalue("1\n2\n3")
EXPECT
/0  '1\n2\n3'\n/i
########
dumpvalue([1..4]);
EXPECT
/0  ARRAY\(0x[0-9a-f]+\)\n   0  1\n   1  2\n   2  3\n   3  4\n/i
########
dumpvalue({1..4});
EXPECT
/0  HASH\(0x[0-9a-f]+\)\n   1 => 2\n   3 => 4\n/i
########
dumpvalue({1=>2,3=>4});
EXPECT
/0  HASH\(0x[0-9a-f]+\)\n   1 => 2\n   3 => 4\n/i
########
dumpvalue({a=>1,b=>2});
EXPECT
/0  HASH\(0x[0-9a-f]+\)\n   'a' => 1\n   'b' => 2\n/i
########
dumpvalue([{a=>[1,2,3],b=>{c=>1,d=>2}},{e=>{f=>1,g=>2},h=>[qw(i j k)]}]);
EXPECT
/0  ARRAY\(0x[0-9a-f]+\)\n   0  HASH\(0x[0-9a-f]+\)\n      'a' => ARRAY\(0x[0-9a-f]+\)\n         0  1\n         1  2\n         2  3\n      'b' => HASH\(0x[0-9a-f]+\)\n         'c' => 1\n         'd' => 2\n   1  HASH\(0x[0-9a-f]+\)\n      'e' => HASH\(0x[0-9a-f]+\)\n         'f' => 1\n         'g' => 2\n      'h' => ARRAY\(0x[0-9a-f]+\)\n         0  'i'\n         1  'j'\n         2  'k'/i
########
dumpvalue({reverse map {$_=>1} sort qw(the quick brown fox)})
EXPECT
/0  HASH\(0x[0-9a-f]+\)\n   1 => 'brown'\n/i
########
my @x=qw(a b c); dumpvalue(\@x);
EXPECT
/0  ARRAY\(0x[0-9a-f]+\)\n   0  'a'\n   1  'b'\n   2  'c'\n/i
########
my %x=(a=>1, b=>2); dumpvalue(\%x);
EXPECT
/0  HASH\(0x[0-9a-f]+\)\n   'a' => 1\n   'b' => 2\n/i
########
dumpvalue(bless[1,2,3,4],"a=b=c");
EXPECT
/0  a=b=c=ARRAY\(0x[0-9a-f]+\)\n   0  1\n   1  2\n   2  3\n   3  4\n/i
########
local *_; tie $_, 'Tyre'; stringify('');
EXPECT
''
########
local *_; tie $_, 'Tyre'; unctrl('abc');
EXPECT
abc
########
tie my %h, 'Kerb'; my $v = { a => 1, b => \%h, c => 2 }; dumpvalue($v);
EXPECT
/'a' => 1\n.+Can't locate object method.+'c' => 2/s
