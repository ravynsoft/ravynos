#!./perl

BEGIN {
    require Config;
    if ($Config::Config{'extensions'} !~ m!\bList/Util\b!){
	print "1..0 # Skip -- Perl configured without List::Util module\n";
	exit 0;
    }
}

my ($ok1, $ok2);
BEGIN {
    require autouse;
    eval {
        "autouse"->import('Scalar::Util' => 'Scalar::Util::set_prototype(&$)');
    };
    $ok1 = !$@;

    eval {
        "autouse"->import('Scalar::Util' => 'Foo::min');
    };
    $ok2 = $@;

    "autouse"->import('Scalar::Util' => qw(isdual set_prototype(&$)));
}

use Test::More tests => 15;

ok( $ok1, "Function from package with custom 'import()' correctly imported" );
like( $ok2, qr/^autouse into different package attempted/, "Catch autouse into different package" );

ok( isdual($!),
    "Function imported via 'autouse' performs as expected");


# set_prototype() has a prototype of &$.  Make sure that's preserved.
sub sum { return $_[0] + $_[1] };
is( (set_prototype \&sum, '$$'), \&sum,
    "Subroutine prototype preserved after import via 'autouse'");


# Example from the docs.
use autouse 'Carp' => qw(carp croak);

{
    my @warning;
    local $SIG{__WARN__} = sub { push @warning, @_ };
    carp "this carp was predeclared and autoused\n";
    is( scalar @warning, 1, "Expected number of warnings received" );
    like( $warning[0], qr/^this carp was predeclared and autoused\n/,
        "Warning received as expected" );

    eval { croak "It is but a scratch!" };
    like( $@, qr/^It is but a scratch!/,
        "Failure message received as expected" );
}


# Test that autouse's lazy module loading works.
use autouse 'Errno' => qw(EPERM);

my $mod_file = 'Errno.pm';   # just fine and portable for %INC
ok( !exists $INC{$mod_file}, "Module not yet loaded" );
ok( EPERM, "Access a constant from that module" ); # test if non-zero
ok( exists $INC{$mod_file}, "Module has been lazily loaded" );

use autouse Env => "something";
eval { something() };
like( $@, qr/^\Qautoused module Env has unique import() method/,
    "Module with unique import() method detected and error reported" );

# Check that UNIVERSAL.pm doesn't interfere with modules that don't use
# Exporter and have no import() of their own.
require UNIVERSAL;
require File::Spec;
unshift @INC, File::Spec->catdir('t', 'lib'), 'lib';
autouse->import("MyTestModule" => 'test_function');
my $ret = test_function();
is( $ret, 'works', "No interference from UNIVERSAL.pm" );

# Test that autouse is exempt from all methods of triggering the subroutine
# redefinition warning.
SKIP: {
    skip "Fails in 5.15.5 and below (perl bug)", 2 if $] < 5.0150051;
    use warnings; local $^W = 1; no warnings 'once';
    my $w;
    local $SIG{__WARN__} = sub { $w .= shift };
    use autouse MyTestModule2 => 'test_function2';
    *MyTestModule2::test_function2 = \&test_function2;
    require MyTestModule2;
    is $w, undef,
       'no redefinition warning when clobbering autouse stub with new sub';
    undef $w;
    import MyTestModule2 'test_function2';
    is $w, undef,
       'no redefinition warning when clobbering autouse stub via *a=\&b';
}
SKIP: {
    skip "Fails in 5.15.5 and below (perl bug)", 1 if $] < 5.0150051;
    use Config;
    skip "no Hash::Util", 1 unless $Config{extensions} =~ /\bHash::Util\b/;
    use warnings; local $^W = 1; no warnings 'once';
    my $w;
    local $SIG{__WARN__} = sub { $w .= shift };
    # any old XS sub from any old module which uses Exporter
    use autouse 'Hash::Util' => "all_keys";
    *Hash::Util::all_keys = \&all_keys;
    require Hash::Util;
    is $w, undef,
      'no redefinition warning when clobbering autouse stub with new XSUB';
}
