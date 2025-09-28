#!./perl -T

use warnings;
our ( @warnings, $fagwoosh, $putt, $kloong );
BEGIN {				# ...and save 'em for later
    $SIG{'__WARN__'} = sub { push @warnings, @_ }
}
END { @warnings && print STDERR join "\n- ", "accumulated warnings:", @warnings }


use strict;
use Test::More tests => 109;
my $TB = Test::More->builder;

BEGIN { use_ok('constant'); }

use constant PI		=> 4 * atan2 1, 1;

ok defined PI,                          'basic scalar constant';
is substr(PI, 0, 7), '3.14159',         '    in substr()';

sub deg2rad { PI * $_[0] / 180 }

my $ninety = deg2rad 90;

cmp_ok abs($ninety - 1.5707), '<', 0.0001, '    in math expression';

use constant UNDEF1	=> undef;	# the right way
use constant UNDEF2	=>	;	# the weird way
use constant 'UNDEF3'		;	# the 'short' way
use constant EMPTY	=> ( )  ;	# the right way for lists

is UNDEF1, undef,       'right way to declare an undef';
is UNDEF2, undef,       '    weird way';
is UNDEF3, undef,       '    short way';

# XXX Why is this way different than the other ones?
my @undef = UNDEF1;
is @undef, 1;
is $undef[0], undef;

@undef = UNDEF2;
is @undef, 0;
@undef = UNDEF3;
is @undef, 0;
@undef = EMPTY;
is @undef, 0;

use constant COUNTDOWN	=> scalar reverse 1, 2, 3, 4, 5;
use constant COUNTLIST	=> reverse 1, 2, 3, 4, 5;
use constant COUNTLAST	=> (COUNTLIST)[-1];

is COUNTDOWN, '54321';
my @cl = COUNTLIST;
is @cl, 5;
is COUNTDOWN, join '', @cl;
is COUNTLAST, 1;
is((COUNTLIST)[1], 4);

use constant ABC	=> 'ABC';
is "abc${\( ABC )}abc", "abcABCabc";

use constant DEF	=> 'D', 'E', chr ord 'F';
is "d e f @{[ DEF ]} d e f", "d e f D E F d e f";

use constant SINGLE	=> "'";
use constant DOUBLE	=> '"';
use constant BACK	=> '\\';
my $tt = BACK . SINGLE . DOUBLE ;
is $tt, q(\\'");

use constant MESS	=> q('"'\\"'"\\);
is MESS, q('"'\\"'"\\);
is length(MESS), 8;

use constant LEADING	=> " \t1234";
cmp_ok LEADING, '==', 1234;
is LEADING, " \t1234";

use constant ZERO1	=> 0;
use constant ZERO2	=> 0.0;
use constant ZERO3	=> '0.0';
is ZERO1, '0';
is ZERO2, '0';
is ZERO3, '0.0';

{
    package Other;
    use constant PI	=> 3.141;
}

cmp_ok(abs(PI - 3.1416), '<', 0.0001);
is Other::PI, 3.141;

# Test that constant.pm can create a dualvar out of $!
use constant A_DUALVAR_CONSTANT => $! = 7;
cmp_ok A_DUALVAR_CONSTANT, '==', 7;
# Make sure we have an error message string.  It does not
# matter that 7 means different things on different platforms.
# If this test fails, then either constant.pm or $! is broken:
cmp_ok length(A_DUALVAR_CONSTANT), '>', 6;

is @warnings, 0 or diag join "\n- ", "unexpected warning:", @warnings;
@warnings = ();		# just in case
undef &PI;
ok @warnings && ($warnings[0] =~ /Constant sub.* undefined/) or
  diag join "\n", "unexpected warning", @warnings;
shift @warnings;

is @warnings, 0, "unexpected warning";

my $curr_test = $TB->current_test;
use constant CSCALAR	=> \"ok 35\n";
use constant CHASH	=> { foo => "ok 36\n" };
use constant CARRAY	=> [ undef, "ok 37\n" ];
use constant CCODE	=> sub { "ok $_[0]\n" };

my $output = $TB->output ;
print $output ${+CSCALAR};
print $output CHASH->{foo};
print $output CARRAY->[1];
print $output CCODE->($curr_test+4);

$TB->current_test($curr_test+4);

eval q{ CCODE->{foo} };
ok scalar($@ =~ /^Constant is not a HASH|Not a HASH reference/);


# Allow leading underscore
use constant _PRIVATE => 47;
is _PRIVATE, 47;

# Disallow doubled leading underscore
eval q{
    use constant __DISALLOWED => "Oops";
};
like $@, qr/begins with '__'/;

# Check on declared() and %declared. This sub should be EXACTLY the
# same as the one quoted in the docs!
sub declared ($) {
    use constant 1.01;              # don't omit this!
    my $name = shift;
    $name =~ s/^::/main::/;
    my $pkg = caller;
    my $full_name = $name =~ /::/ ? $name : "${pkg}::$name";
    $constant::declared{$full_name};
}

ok declared 'PI';
ok $constant::declared{'main::PI'};

ok !declared 'PIE';
ok !$constant::declared{'main::PIE'};

{
    package Other;
    use constant IN_OTHER_PACK => 42;
    ::ok ::declared 'IN_OTHER_PACK';
    ::ok $constant::declared{'Other::IN_OTHER_PACK'};
    ::ok ::declared 'main::PI';
    ::ok $constant::declared{'main::PI'};
}

ok declared 'Other::IN_OTHER_PACK';
ok $constant::declared{'Other::IN_OTHER_PACK'};

@warnings = ();
eval q{
    no warnings;
    use warnings 'constant';
    use constant 'BEGIN' => 1 ;
    use constant 'INIT' => 1 ;
    use constant 'CHECK' => 1 ;
    use constant 'END' => 1 ;
    use constant 'DESTROY' => 1 ;
    use constant 'AUTOLOAD' => 1 ;
    use constant 'STDIN' => 1 ;
    use constant 'STDOUT' => 1 ;
    use constant 'STDERR' => 1 ;
    use constant 'ARGV' => 1 ;
    use constant 'ARGVOUT' => 1 ;
    use constant 'ENV' => 1 ;
    use constant 'INC' => 1 ;
    use constant 'SIG' => 1 ;
    use constant 'UNITCHECK' => 1;
};

my @Expected_Warnings = 
  (
   qr/^Constant name 'BEGIN' is a Perl keyword at/,
   qr/^Constant subroutine BEGIN redefined at/,
   qr/^Constant name 'INIT' is a Perl keyword at/,
   qr/^Constant name 'CHECK' is a Perl keyword at/,
   qr/^Constant name 'END' is a Perl keyword at/,
   qr/^Constant name 'DESTROY' is a Perl keyword at/,
   qr/^Constant name 'AUTOLOAD' is a Perl keyword at/,
   qr/^Constant name 'STDIN' is forced into package main:: a/,
   qr/^Constant name 'STDOUT' is forced into package main:: at/,
   qr/^Constant name 'STDERR' is forced into package main:: at/,
   qr/^Constant name 'ARGV' is forced into package main:: at/,
   qr/^Constant name 'ARGVOUT' is forced into package main:: at/,
   qr/^Constant name 'ENV' is forced into package main:: at/,
   qr/^Constant name 'INC' is forced into package main:: at/,
   qr/^Constant name 'SIG' is forced into package main:: at/,
   qr/^Constant name 'UNITCHECK' is a Perl keyword at/,
);

unless ($] > 5.009) {
    # Remove the UNITCHECK warning
    pop @Expected_Warnings;
    # But keep the count the same
    push @Expected_Warnings, qr/^$/;
    push @warnings, "";
}

# when run under "make test"
if (@warnings == 16) {
    push @warnings, "";
    push @Expected_Warnings, qr/^$/;
}
# when run directly: perl -wT -Ilib t/constant.t
elsif (@warnings == 17) {
    splice @Expected_Warnings, 1, 0, 
        qr/^Prototype mismatch: sub main::BEGIN \(\) vs none at/;
}
# when run directly under 5.6.2: perl -wT -Ilib t/constant.t
elsif (@warnings == 15) {
    splice @Expected_Warnings, 1, 1;
    push @warnings, "", "";
    push @Expected_Warnings, qr/^$/, qr/^$/;
}
else {
    my $rule = " -" x 20;
    diag "/!\\ unexpected case: ", scalar @warnings, " warnings\n$rule\n";
    diag map { "  $_" } @warnings;
    diag $rule, $/;
}

is @warnings, 17;

for my $idx (0..$#warnings) {
    like $warnings[$idx], $Expected_Warnings[$idx];
}

@warnings = ();


use constant {
	THREE  => 3,
	FAMILY => [ qw( John Jane Sally ) ],
	AGES   => { John => 33, Jane => 28, Sally => 3 },
	RFAM   => [ [ qw( John Jane Sally ) ] ],
	SPIT   => sub { shift },
};

is @{+FAMILY}, THREE;
is @{+FAMILY}, @{RFAM->[0]};
is FAMILY->[2], RFAM->[0]->[2];
is AGES->{FAMILY->[1]}, 28;
is THREE**3, SPIT->(@{+FAMILY}**3);

# Allow name of digits/underscores only if it begins with underscore
{
    use warnings FATAL => 'constant';
    eval q{
        use constant _1_2_3 => 'allowed';
    };
    ok( $@ eq '' );
}

sub slotch ();

{
    my @warnings;
    local $SIG{'__WARN__'} = sub { push @warnings, @_ };
    eval 'use constant slotch => 3; 1' or die $@;

    is ("@warnings", "", "No warnings if a prototype exists");

    my $value = eval 'slotch';
    is ($@, '');
    is ($value, 3);
}

sub zit;

{
    my @warnings;
    local $SIG{'__WARN__'} = sub { push @warnings, @_ };
    eval 'use constant zit => 4; 1' or die $@;

    # empty prototypes are reported differently in different versions
    my $no_proto = $] < 5.008004 ? "" : ": none";

    is(scalar @warnings, 1, "1 warning");
    like ($warnings[0], qr/^Prototype mismatch: sub main::zit$no_proto vs \(\)/,
	  "about the prototype mismatch");

    my $value = eval 'zit';
    is ($@, '');
    is ($value, 4);
}

$fagwoosh = 'geronimo';
$putt = 'leutwein';
$kloong = 'schlozhauer';

{
    my @warnings;
    local $SIG{'__WARN__'} = sub { push @warnings, @_ };
    eval 'use constant fagwoosh => 5; 1' or die $@;

    is ("@warnings", "", "No warnings if the typeglob exists already");

    my $value = eval 'fagwoosh';
    is ($@, '');
    is ($value, 5);

    my @value = eval 'fagwoosh';
    is ($@, '');
    is_deeply (\@value, [5]);

    eval 'use constant putt => 6, 7; 1' or die $@;

    is ("@warnings", "", "No warnings if the typeglob exists already");

    @value = eval 'putt';
    is ($@, '');
    is_deeply (\@value, [6, 7]);

    eval 'use constant "klong"; 1' or die $@;

    is ("@warnings", "", "No warnings if the typeglob exists already");

    $value = eval 'klong';
    is ($@, '');
    is ($value, undef);

    @value = eval 'klong';
    is ($@, '');
    is_deeply (\@value, []);
}

{
    local $SIG{'__WARN__'} = sub { die "WARNING: $_[0]" };
    eval 'use constant undef, 5; 1';
    like $@, qr/\ACan't use undef as constant name at /;
}

# Constants created by "use constant" should be read-only

# This test will not test what we are trying to test if this glob entry
# exists already, so test that, too.
ok !exists $::{immutable};
eval q{
    use constant immutable => 23987423874;
    for (immutable) { eval { $_ = 22 } }
    like $@, qr/^Modification of a read-only value attempted at /,
	'constant created in empty stash slot is immutable';
    eval { for (immutable) { ${\$_} = 432 } };
    SKIP: {
	require Config;
	if ($Config::Config{useithreads}) {
	    skip "fails under threads", 1 if $] < 5.019003;
	}
	like $@, qr/^Modification of a read-only value attempted at /,
	    '... and immutable through refgen, too';
    }
};
() = \&{"immutable"}; # reify
eval 'for (immutable) { $_ = 42 }';
like $@, qr/^Modification of a read-only value attempted at /,
    '... and after reification';

# Use an existing stash element this time.
# This next line is sufficient to trigger a different code path in
# constant.pm.
() = \%::existing_stash_entry;
use constant existing_stash_entry => 23987423874;
for (existing_stash_entry) { eval { $_ = 22 } }
like $@, qr/^Modification of a read-only value attempted at /,
    'constant created in existing stash slot is immutable';
eval { for (existing_stash_entry) { ${\$_} = 432 } };
SKIP: {
    if ($Config::Config{useithreads}) {
	skip "fails under threads", 1 if $] < 5.019003;
    }
    like $@, qr/^Modification of a read-only value attempted at /,
	'... and immutable through refgen, too';
}

# Test that list constants are also immutable.  This only works under
# 5.19.3 and later.
SKIP: {
    skip "fails under 5.19.2 and earlier", 3 if $] < 5.019003;
    local $TODO = "disabled for now; breaks CPAN; see perl #119045";
    use constant constant_list => 1..2;
    for (constant_list) {
	my $num = $_;
	eval { $_++ };
	like $@, qr/^Modification of a read-only value attempted at /,
	    "list constant has constant elements ($num)";
    }
    undef $TODO;
    # Whether values are modifiable or no, modifying them should not affect
    # future return values.
    my @values;
    for(1..2) {
	for ((constant_list)[0]) {
	    push @values, $_;
	    eval {$_++};
	}
    }
    is $values[1], $values[0],
	'modifying list const elements does not affect future retavls';
}

use constant { "tahi" => 1, "rua::rua" => 2, "toru'toru" => 3 };
use constant "wha::wha" => 4;
is tahi, 1, 'unqualified constant declared with constants in other pkgs';
is rua::rua, 2, 'constant declared with ::';
is toru::toru, 3, "constant declared with '";
is wha::wha, 4, 'constant declared by itself with ::';
