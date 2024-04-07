#!./perl

BEGIN {
	require Config;
	if (($Config::Config{'extensions'} !~ m!\bList/Util\b!) ){
	    print "1..0 # Skip -- Perl configured without List::Util module\n";
	    exit 0;
	}

	# `make test` in the CPAN version of this module runs us with -w, but
	# Dumpvalue.pm relies on all sorts of things that can cause warnings. I
	# don't think that's worth fixing, so we just turn off all warnings
	# during testing.
	$^W = 0;
}

use lib ("./t/lib");
use TieOut;
use Test::More tests => 88;

use_ok( 'Dumpvalue' );

my $d;
ok( $d = Dumpvalue->new(), 'create a new Dumpvalue object' );

$d->set( globPrint => 1, dumpReused => 1 );
is( $d->{globPrint}, 1, 'set an option correctly' );
is( $d->get('globPrint'), 1, 'get an option correctly' );
is_deeply( [ $d->get('globPrint', 'dumpReused') ],
    [ 1, 1 ],
    'get multiple options'
);

# check to see if unctrl works
is( ref( Dumpvalue::unctrl(*FOO) ), 'GLOB', 'unctrl should not modify GLOB' );
is( Dumpvalue::unctrl('donotchange'), 'donotchange', "unctrl shouldn't modify");
like( Dumpvalue::unctrl("bo\007nd"), qr/bo\^.nd/, 'unctrl should escape' );

# check to see if stringify works
is( $d->stringify(), 'undef', 'stringify handles undef okay' );

# the default is 1, but we want two single quotes
$d->{printUndef} = 0;
is( $d->stringify(), "''", 'stringify skips undef when asked nicely' );

is( $d->stringify(*FOO), *FOO . "", 'stringify stringifies globs alright' );

# check for double-quotes if there's an unprintable character
$d->{tick} = 'auto';
like( $d->stringify("hi\005"), qr/^"hi/, 'added double-quotes when necessary' );

# if no unprintable character, escape ticks or backslashes
is( $d->stringify('hi'), "'hi'", 'used single-quotes when appropriate' );

# if 'unctrl' is set
$d->{unctrl} = 'unctrl';
like( $d->stringify('double and whack:\ "'), qr!\\ \"!, 'escaped with unctrl' );
like( $d->stringify("a\005"), qr/^"a\^/, 'escaped ASCII value in unctrl' );
like( $d->stringify("b\xb6"), qr!^'b.'$!, 'no high-bit escape value in unctrl');

$d->{quoteHighBit} = 1;
like( $d->stringify("b\266"), qr!^'b\\266!, 'high-bit now escaped in unctrl');

# if 'quote' is set
$d->{unctrl} = 'quote';
is( $d->stringify('5@ $1'), "'5\@ \$1'", 'quoted $ and @ fine' );
is( $d->stringify("5@\e\$1"), '"5\@\e\$1"', 'quoted $ and @ and \e fine' );
like( $d->stringify("\037"), qr/^"\\c/, 'escaped ASCII value okay' );

# add ticks, if necessary
is( $d->stringify("no ticks", 1), 'no ticks', 'avoid ticks if asked' );

my $out = tie *OUT, 'TieOut';
select(OUT);

# test DumpElem, it does its magic with veryCompact set
$d->{veryCompact} = 1;
$d->DumpElem([1, 2, 3]);
is( $out->read, "0..2  1 2 3\n", 'DumpElem worked on array ref');
$d->DumpElem({ one => 1, two => 2 });
is( $out->read, "'one' => 1, 'two' => 2\n", 'DumpElem worked on hash ref' );
$d->DumpElem('hi');
is( $out->read, "'hi'\n", 'DumpElem worked on simple scalar' );
$d->{veryCompact} = 0;
$d->DumpElem([]);
like( $out->read, qr/ARRAY/, 'DumpElem okay with reference and no veryCompact');

# should compact simple arrays just fine
$d->{veryCompact} = 1;
$d->DumpElem([1, 2, 3]);
is( $out->read, "0..2  1 2 3\n", 'dumped array fine' );
$d->{arrayDepth} = 2;
$d->DumpElem([1, 2, 3]);
is( $out->read, "0..2  1 2 ...\n", 'dumped limited array fine' );

# should compact simple hashes just fine
$d->DumpElem({ a => 1, b => 2, c => 3 });
is( $out->read, "'a' => 1, 'b' => 2, 'c' => 3\n", 'dumped hash fine' );
$d->{hashDepth} = 2;
$d->DumpElem({ a => 1, b => 2, c => 3 });
is( $out->read, "'a' => 1, 'b' => 2 ...\n", 'dumped limited hash fine' );

# should just stringify what it is
$d->{veryCompact} = 0;
$d->DumpElem([]);
like( $out->read, qr/ARRAY.+empty array/s, 'stringified empty array ref' );
$d->DumpElem({});
like( $out->read, qr/HASH.+empty hash/s, 'stringified empty hash ref' );
$d->DumpElem(1);
is( $out->read, "1\n", 'stringified simple scalar' );

# test unwrap
$DB::signal = $d->{stopDbSignal} = 1;
is( $d->unwrap(), undef, 'unwrap returns if DB signal is set' );
undef $DB::signal;

my $foo = 7;
$d->{dumpReused} = 0;
$d->unwrap(\$foo);
is( $out->read, "-> 7\n", 'unwrap worked on scalar' );
$d->unwrap(\$foo);
is( $out->read, "-> REUSED_ADDRESS\n", 'unwrap worked on scalar' );
$d->unwrap({ one => 1 });

# leaving this at zero may cause some subsequent tests to fail
# if they reuse an address creating an anonymous variable
$d->{dumpReused} = 1;
is( $out->read, "'one' => 1\n", 'unwrap worked on hash' );
$d->unwrap([ 2, 3 ]);
is( $out->read, "0  2\n1  3\n", 'unwrap worked on array' );
$d->unwrap(*FOO);
is( $out->read, '', 'unwrap ignored glob on first try');
$d->unwrap(*FOO);
is( $out->read, "*DUMPED_GLOB*\n", 'unwrap worked on glob');
$d->unwrap(qr/foo(.+)/);

my $modifiers = (qr// =~ /\Q(?^/) ? '^' : '-xism';
is( $out->read, "-> qr/(?${modifiers}:foo(.+))/\n", 'unwrap worked on Regexp' );

$d->unwrap( sub {} );
like( $out->read, qr/^-> &CODE/, 'unwrap worked on sub ref' );

# test matchvar
# test to see if first arg 'eq' second
ok( Dumpvalue::matchvar(1, 1), 'matchvar matched numbers fine' );
ok( Dumpvalue::matchvar('hi', 'hi'), 'matchvar matched strings fine' );
ok( !Dumpvalue::matchvar('hello', 1), 'matchvar caught failed match fine' );

# test compactDump, which doesn't do much
is( $d->compactDump(3), 3, 'set compactDump to 3' );
is( $d->compactDump(1), 479, 'compactDump reset to 6*80-1 when less than 2' );

# test veryCompact, which does slightly more, setting compactDump sometimes
$d->{compactDump} = 0;
is( $d->veryCompact(1), 1, 'set veryCompact successfully' );
ok( $d->compactDump(), 'and it set compactDump as well' );

# test set_unctrl
$d->set_unctrl('impossible value');
like( $out->read, qr/^Unknown value/, 'set_unctrl caught bad value' );
is( $d->set_unctrl('quote'), 'quote', 'set quote fine' );
is( $d->set_unctrl(), 'quote', 'retrieved quote fine' );

# test set_quote
$d->set_quote('"');
is( $d->{tick}, '"', 'set_quote set tick right' );
is( $d->{unctrl}, 'quote', 'set unctrl right too' );
$d->set_quote('auto');
is( $d->{tick}, 'auto', 'set_quote set auto right' );
$d->set_quote('foo');
is( $d->{tick}, "'", 'default value set to " correctly' );

# test dumpglob
# should do nothing if debugger signal flag is raised
$d->{stopDbSignal} = $DB::signal = 1;
is( $d->dumpglob(*DB::signal), undef, 'returned early with DB signal set' );
undef $DB::signal;

# test dumping "normal" variables, this is a nasty glob trick
$foo = 1;
$d->dumpglob( '', 2, 'foo', local *foo = \$foo );
is( $out->read, "  \$foo = 1\n", 'dumped glob for $foo correctly' );
our @bar = (1, 2);

# the key name is a little different here
$d->dumpglob( '', 0, 'boo', *bar );
is( $out->read, "\@boo = (\n   0..1  1 2\n)\n", 'dumped glob for @bar fine' );

our %baz = ( one => 1, two => 2 );
$d->dumpglob( '', 0, 'baz', *baz );
is( $out->read, "\%baz = (\n   'one' => 1, 'two' => 2\n)\n",
	'dumped glob for %baz fine' );

SKIP: {
	skip( "Couldn't open $0 for reading", 1 ) unless open(FILE, '<', $0);
	my $fileno = fileno(FILE);
	$d->dumpglob( '', 0, 'FILE', *FILE );
	is( $out->read, "FileHandle(FILE) => fileno($fileno)\n",
		'dumped filehandle from glob fine' );
}

$d->dumpglob( '', 0, 'read', *TieOut::read );
is( $out->read, '', 'no sub dumped without $all set' );
$d->dumpglob( '', 0, 'read', \&TieOut::read, 1 );
is( $out->read, "&read in ???\n", 'sub dumped when requested' );

# see if it dumps DB-like values correctly
$d->{dumpDBFiles} = 1;
$d->dumpglob( '', 0, '_<foo', *foo );
is( $out->read, "\$_<foo = 1\n", 'dumped glob for $_<foo correctly (DB)' );

# test CvGV name
SKIP: {
	if (" $Config::Config{'extensions'} " !~ m[ Devel/Peek ]) {
	    skip( 'no Devel::Peek', 2 );
	}
	use_ok( 'Devel::Peek' );
	is( $d->CvGV_name(\&TieOut::read), 'TieOut::read', 'CvGV_name found sub' );
}

# test dumpsub
$d->dumpsub( '', 'TieOut::read' );
like( $out->read, qr/&TieOut::read in/, 'dumpsub found sub fine' );

# test findsubs
is( $d->findsubs(), undef, 'findsubs returns nothing without %DB::sub' );
{
    no warnings 'once';
    $DB::sub{'TieOut::read'} = 'TieOut';
    is( $d->findsubs( \&TieOut::read ), 'TieOut::read', 'findsubs reported sub' );
}

# now that it's capable of finding the package...
$d->dumpsub( '', 'TieOut::read' );
is( $out->read, "&TieOut::read in TieOut\n", 'dumpsub found sub fine again' );

# this should print just a usage message
$d->{usageOnly} = 1;
$d->dumpvars( 'Fake', 'veryfake' );
like( $out->read, qr/^String space:/, 'printed usage message fine' );
delete $d->{usageOnly};

# this should report @INC and %INC
$d->dumpvars( 'main', 'INC' );
like( $out->read, qr/\@INC =/, 'dumped variables from a package' );

# this should report nothing
$DB::signal = 1;
$d->dumpvars( 'main', 'INC' );
is( $out->read, '', 'no dump when $DB::signal is set' );
undef $DB::signal;

is( $d->scalarUsage('12345'), 5, 'scalarUsage reports length correctly' );
is( $d->arrayUsage( [1, 2, 3], 'a' ), 3, 'arrayUsage reports correct lengths' );
is( $out->read, "\@a = 3 items (data: 3 bytes)\n", 'arrayUsage message okay' );
is( $d->hashUsage({ one => 1 }, 'b'), 4, 'hashUsage reports correct lengths' );
is( $out->read, "\%b = 1 item (keys: 3; values: 1; total: 4 bytes)\n",
	'hashUsage message okay' );
is( $d->hashUsage({ one => [ 1, 2, 3 ]}, 'c'), 6, 'complex hash okay' );
is( $out->read, "\%c = 1 item (keys: 3; values: 3; total: 6 bytes)\n",
	'hashUsage complex message okay' );

our $folly = 'one';
our @folly = ('two');
our %folly = ( three => '123' );
is( $d->globUsage(\*folly, 'folly'), 14, 'globUsage reports length correctly' );
like( $out->read, qr/\@folly =.+\%folly =/s, 'globValue message okay' );

# and now, the real show
$d->dumpValue(undef);
is( $out->read, "undef\n", 'dumpValue caught undef value okay' );
$d->dumpValue($folly);
is( $out->read, "'one'\n", 'dumpValue worked' );
$d->dumpValue(@folly);
is( $out->read, "'two'\n", 'dumpValue worked on array' );
$d->dumpValue(\$folly);
is( $out->read, "-> 'one'\n", 'dumpValue worked on scalar ref' );

# dumpValues (the rest of these should be caught by unwrap)
$d->dumpValues(undef);
is( $out->read, "undef\n", 'dumpValues caught undef value fine' );
$d->dumpValues(\@folly);
is( $out->read, "0  0..0  'two'\n", 'dumpValues worked on array ref' );
$d->dumpValues('one', 'two');
is( $out->read, "0..1  'one' 'two'\n", 'dumpValues worked on multiple values' );

