use strict;
use warnings;

use Config;

use IPC::Open3 1.0103 qw(open3);
use Test::More tests => 68;

sub runperl {
    my(%args) = @_;
    my($w, $r);

    local $ENV{PERL5LIB} = join ($Config::Config{path_sep}, @INC);

    my $pid = open3($w, $r, undef, $^X, "-e", $args{prog});
    close $w;
    my $output = "";
    while(<$r>) { $output .= $_; }
    waitpid($pid, 0);
    return $output;
}

my $Is_VMS = $^O eq 'VMS';

use Carp qw(carp cluck croak confess);

BEGIN {
    # This test must be run at BEGIN time, because code later in this file
    # sets CORE::GLOBAL::caller
    ok !exists $CORE::GLOBAL::{caller},
        "Loading doesn't create CORE::GLOBAL::caller";
}

{
  my $line = __LINE__; my $str = Carp::longmess("foo");
  is(
    $str,
    "foo at $0 line $line.\n",
    "we don't overshoot the top stack frame",
  );
}

package MyClass;

sub new { return bless +{ field => ['value1', 'SecondVal'] }; }

package main;

{
    my $err = Carp::longmess(MyClass->new);

    # See:
    # https://rt.cpan.org/Public/Bug/Display.html?id=107225
    is_deeply(
        $err->{field},
        ['value1', 'SecondVal',],
        "longmess returns sth meaningful in scalar context when passed a ref.",
    );
}

{
    local $SIG{__WARN__} = sub {
        like $_[0], qr/ok (\d+)\n at.+\b(?i:carp\.t) line \d+\.$/, 'ok 2\n';
    };

    carp "ok 2\n";
}

{
    local $SIG{__WARN__} = sub {
        like $_[0], qr/(\d+) at.+\b(?i:carp\.t) line \d+\.$/, 'carp 3';
    };

    carp 3;
}

sub sub_4 {
    local $SIG{__WARN__} = sub {
        like $_[0],
            qr/^(\d+) at.+\b(?i:carp\.t) line \d+\.\n\tmain::sub_4\(\) called at.+\b(?i:carp\.t) line \d+$/,
            'cluck 4';
    };

    cluck 4;
}

sub_4;

{
    local $SIG{__DIE__} = sub {
        like $_[0],
            qr/^(\d+) at.+\b(?i:carp\.t) line \d+\.\n\teval \Q{...}\E called at.+\b(?i:carp\.t) line \d+$/,
            'croak 5';
    };

    eval { croak 5 };
}

sub sub_6 {
    local $SIG{__DIE__} = sub {
        like $_[0],
            qr/^(\d+) at.+\b(?i:carp\.t) line \d+\.\n\teval \Q{...}\E called at.+\b(?i:carp\.t) line \d+\n\tmain::sub_6\(\) called at.+\b(?i:carp\.t) line \d+$/,
            'confess 6';
    };

    eval { confess 6 };
}

sub_6;

ok(1);

# test for caller_info API
my $eval = "use Carp; return Carp::caller_info(0);";
my %info = eval($eval);
is( $info{sub_name}, "eval '$eval'", 'caller_info API' );

# test for '...::CARP_NOT used only once' warning from Carp
my $warning;
eval { do {
    BEGIN {
        local $SIG{__WARN__} = sub {
            if   ( defined $^S ) { warn $_[0] }
            else                 { $warning = $_[0] }
            }
    }

    package Z;

    BEGIN {
        eval { Carp::croak() };
    }
} };
ok !$warning, q/'...::CARP_NOT used only once' warning from Carp/;

# Test the location of error messages.
like( XA::short(), qr/^Error at XC/, "Short messages skip carped package" );

{
    local @XC::ISA = "XD";
    like( XA::short(), qr/^Error at XB/, "Short messages skip inheritance" );
}

{
    local @XD::ISA = "XC";
    like( XA::short(), qr/^Error at XB/, "Short messages skip inheritance" );
}

{
    local @XD::ISA = "XB";
    local @XB::ISA = "XC";
    like( XA::short(), qr/^Error at XA/, "Inheritance is transitive" );
}

{
    local @XB::ISA = "XD";
    local @XC::ISA = "XB";
    like( XA::short(), qr/^Error at XA/, "Inheritance is transitive" );
}

{
    local @XC::CARP_NOT = "XD";
    like( XA::short(), qr/^Error at XB/, "Short messages see \@CARP_NOT" );
}

{
    local @XD::CARP_NOT = "XC";
    like( XA::short(), qr/^Error at XB/, "Short messages see \@CARP_NOT" );
}

{
    local @XD::CARP_NOT = "XB";
    local @XB::CARP_NOT = "XC";
    like( XA::short(), qr/^Error at XA/, "\@CARP_NOT is transitive" );
}

{
    local @XB::CARP_NOT = "XD";
    local @XC::CARP_NOT = "XB";
    like( XA::short(), qr/^Error at XA/, "\@CARP_NOT is transitive" );
}

{
    local @XD::ISA      = "XC";
    local @XD::CARP_NOT = "XB";
    like( XA::short(), qr/^Error at XC/, "\@CARP_NOT overrides inheritance" );
}

{
    local @XD::ISA      = "XB";
    local @XD::CARP_NOT = "XC";
    like( XA::short(), qr/^Error at XB/, "\@CARP_NOT overrides inheritance" );
}

# %Carp::Internal
{
    local $Carp::Internal{XC} = 1;
    like( XA::short(), qr/^Error at XB/, "Short doesn't report Internal" );
}

{
    local $Carp::Internal{XD} = 1;
    like( XA::long(), qr/^Error at XC/, "Long doesn't report Internal" );
}

# %Carp::CarpInternal
{
    local $Carp::CarpInternal{XD} = 1;
    like(
        XA::short(), qr/^Error at XB/,
        "Short doesn't report calls to CarpInternal"
    );
}

{
    local $Carp::CarpInternal{XD} = 1;
    like( XA::long(), qr/^Error at XC/, "Long doesn't report CarpInternal" );
}

# tests for global variables
sub x { carp @_ }
sub w { cluck @_ }

# $Carp::Verbose;
{
    my $aref = [
        qr/t at \S*(?i:carp.t) line \d+\./,
        qr/t at \S*(?i:carp.t) line \d+\.\n\s*main::x\("t"\) called at \S*(?i:carp.t) line \d+/
    ];
    my $i = 0;

    for my $re (@$aref) {
        local $Carp::Verbose = $i++;
        local $SIG{__WARN__} = sub {
            like $_[0], $re, 'Verbose';
        };

        package Z;
        main::x('t');
    }
}

# $Carp::MaxEvalLen
{
    my $test_num = 1;
    for ( 0, 4 ) {
        my $txt = "Carp::cluck($test_num)";
        local $Carp::MaxEvalLen = $_;
        local $SIG{__WARN__} = sub {
            "@_" =~ /'(.+?)(?:\n|')/s;
            is length($1),
                length( $_ ? substr( $txt, 0, $_ ) : substr( $txt, 0 ) ),
                'MaxEvalLen';
        };
        eval "$txt";
        $test_num++;
    }
}

# $Carp::MaxArgNums
{
    my $aref = [
        [ -1            => '(...)' ],
        [ 0             => '(1, 2, 3, 4)' ],
        [ '0 but true'  => '(...)' ],
        [ 1             => '(1, ...)' ],
        [ 3             => '(1, 2, 3, ...)' ],
        [ 4             => '(1, 2, 3, 4)' ],
        [ 5             => '(1, 2, 3, 4)' ],
    ];

    for (@$aref) {
        my ($arg_count, $expected_signature) = @$_;

        my $expected = join('',
            '1234 at \S*(?i:carp.t) line \d+\.\n\s*main::w',
            quotemeta $expected_signature,
            ' called at \S*(?i:carp.t) line \d+'
        );

        local $Carp::MaxArgNums = $arg_count;
        local $SIG{__WARN__} = sub {
            like "@_", qr/$expected/, "MaxArgNums=$arg_count";
        };

        package Z;
        main::w( 1 .. 4 );
    }
}

# $Carp::CarpLevel
{
    my $i    = 0;
    my $aref = [
        qr/1 at \S*(?i:carp.t) line \d+\.\n\s*main::w\(1\) called at \S*(?i:carp.t) line \d+/,
        qr/1 at \S*(?i:carp.t) line \d+\.$/,
    ];

    for (@$aref) {
        local $Carp::CarpLevel = $i++;
        local $SIG{__WARN__} = sub {
            like "@_", $_, 'CarpLevel';
        };

        package Z;
        main::w(1);
    }
}

SKIP:
{
    skip "IPC::Open3::open3 needs porting", 2 if $Is_VMS;

    # Check that croak() and confess() don't clobber $!
    runperl(
        prog   => 'use Carp; $@=q{Phooey}; $!=42; croak(q{Dead})',
        stderr => 1
    );

    is( $? >> 8, 42, 'croak() doesn\'t clobber $!' );

    runperl(
        prog   => 'use Carp; $@=q{Phooey}; $!=42; confess(q{Dead})',
        stderr => 1
    );

    is( $? >> 8, 42, 'confess() doesn\'t clobber $!' );
}

# undef used to be incorrectly reported as the string "undef"
sub cluck_undef {

    local $SIG{__WARN__} = sub {
        like $_[0],
            qr/^Bang! at.+\b(?i:carp\.t) line \d+\.\n\tmain::cluck_undef\(0, "undef", 2, undef, 4\) called at.+\b(?i:carp\.t) line \d+$/,
            "cluck doesn't quote undef";
    };

    cluck "Bang!"

}

cluck_undef( 0, "undef", 2, undef, 4 );

# check that Carp respects CORE::GLOBAL::caller override after Carp
# has been compiled
for my $bodge_job ( 2, 1, 0 ) { SKIP: {
    skip "can't safely detect incomplete caller override on perl $]", 6
	if $bodge_job && !Carp::CALLER_OVERRIDE_CHECK_OK;
    print '# ', ( $bodge_job ? 'Not ' : '' ),
        "setting \@DB::args in caller override\n";
    if ( $bodge_job == 1 ) {
        require B;
        print "# required B\n";
    }
    my $accum = '';
    no warnings 'once';
    local *CORE::GLOBAL::caller = sub {
        local *__ANON__ = "fakecaller";
        my @c = CORE::caller(@_);
        $c[0] ||= 'undef';
        $accum .= "@c[0..3]\n";
        if ( !$bodge_job && CORE::caller() eq 'DB' ) {

            package DB;
            return CORE::caller( ( $_[0] || 0 ) + 1 );
        }
        else {
            return CORE::caller( ( $_[0] || 0 ) + 1 );
        }
    };
    eval "scalar caller()";
    like( $accum, qr/main::fakecaller/,
        "test CORE::GLOBAL::caller override in eval" );
    $accum = '';
    my $got = XA::long(42);
    like( $accum, qr/main::fakecaller/,
        "test CORE::GLOBAL::caller override in Carp" );
    my $package = 'XA';
    my $where = $bodge_job == 1 ? ' in &main::__ANON__' : '';
    my $warning
        = $bodge_job
        ? "\Q** Incomplete caller override detected$where; \@DB::args were not set **\E"
        : '';

    for ( 0 .. 2 ) {
        my $previous_package = $package;
        ++$package;
        like( $got,
            qr/${package}::long\($warning\) called at $previous_package line \d+/,
            "Correct arguments for $package" );
    }
    my $arg = $bodge_job ? $warning : 42;
    like(
        $got, qr!XA::long\($arg\) called at.+\b(?i:carp\.t) line \d+!,
        'Correct arguments for XA'
    );
} }

SKIP: {
    skip "can't safely detect incomplete caller override on perl $]", 1
	unless Carp::CALLER_OVERRIDE_CHECK_OK;
    eval q{
	no warnings 'redefine';
	sub CORE::GLOBAL::caller {
	    my $height = $_[0];
	    $height++;
	    return CORE::caller($height);
	}
    };

    my $got = XA::long(42);

    like(
	$got,
	qr!XA::long\(\Q** Incomplete caller override detected; \E\@DB::args\Q were not set **\E\) called at.+\b(?i:carp\.t) line \d+!,
	'Correct arguments for XA'
    );
}

# UTF8-flagged strings should not cause Carp to try to load modules (even
# implicitly via utf8_heavy.pl) after a syntax error [perl #82854].
SKIP:
{
    skip "IPC::Open3::open3 needs porting", 1 if $Is_VMS;
    like(
      runperl(
        prog => q<
          use utf8; use strict; use Carp;
          BEGIN { $SIG{__DIE__} = sub { Carp::croak qq(aaaaa$_[0]) } }
          $c
        >,
        stderr=>1,
      ),
      qr/aaaaa/,
      'Carp can handle UTF8-flagged strings after a syntax error',
    );
}

# [perl #96672]
<XD::DATA> for 1..2;
eval { croak 'heek' };
$@ =~ s/\n.*//; # just check first line
is $@, "heek at ".__FILE__." line ".(__LINE__-2).", <DATA> line 2.\n",
    'last handle line num is mentioned';

# [cpan #100183]
{
    local $/ = \6;
    <XD::DATA>;
    eval { croak 'jeek' };
    $@ =~ s/\n.*//; # just check first line
    is $@, "jeek at ".__FILE__." line ".(__LINE__-2).", <DATA> chunk 3.\n",
        'last handle chunk num is mentioned';
}

SKIP:
{
    skip "IPC::Open3::open3 needs porting", 1 if $Is_VMS;
    like(
      runperl(
        prog => q<
          open FH, q-Makefile.PL-;
          <FH>;  # set PL_last_in_gv
          BEGIN { *CORE::GLOBAL::die = sub { die Carp::longmess(@_) } };
          use Carp;
          die fumpts;
        >,
      ),
      qr 'fumpts',
      'Carp::longmess works inside CORE::GLOBAL::die',
    );
}

{
    package Foo::No::CARP_NOT;
    eval { Carp::croak(1) };
    ::is_deeply(
        [ keys %Foo::No::CARP_NOT:: ],
        [],
        "Carp doesn't create CARP_NOT or ISA in the caller if they don't exist"
    );

    package Foo::No::Autovivify;
    our $CARP_NOT = 1;
    eval { Carp::croak(1) };
    ::ok(
        !defined *{$Foo::No::Autovivify::{CARP_NOT}}{ARRAY},
        "Carp doesn't autovivify the CARP_NOT or ISA arrays if the globs exists but they lack the ARRAY slot"
    );
}

{
    package Mpar;
    sub f { Carp::croak "tun syn" }

    package Phou;
    $Phou::{ISA} = \42;
    eval { Mpar::f };
}
like $@, qr/tun syn/, 'Carp can handle non-glob ISA stash elems';


# New tests go here

# line 1 "XA"
package XA;

sub short {
    XB::short();
}

sub long {
    XB::long();
}

# line 1 "XB"
package XB;

sub short {
    XC::short();
}

sub long {
    XC::long();
}

# line 1 "XC"
package XC;

sub short {
    XD::short();
}

sub long {
    XD::long();
}

# line 1 "XD"
package XD;

sub short {
    eval { Carp::croak("Error") };
    return $@;
}

sub long {
    eval { Carp::confess("Error") };
    return $@;
}

# Put new tests at "new tests go here"
__DATA__
1
2
3
abcdefghijklmnopqrstuvwxyz
