#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc(qw '../lib ../cpan/Text-ParseWords/lib');
    require Config; # load these before we mess with *CORE::GLOBAL::require
    require 'Config_heavy.pl'; # since runperl will need them
}

plan tests => 36;

#
# This file tries to test builtin override using CORE::GLOBAL
#
my $dirsep = "/";

BEGIN { package Foo; *main::getlogin = sub { "kilroy"; } }

is( getlogin, "kilroy" );

my $t = 42;
BEGIN { *CORE::GLOBAL::time = sub () { $t; } }

is( 45, time + 3 );

#
# require has special behaviour
#
my $r;
BEGIN { *CORE::GLOBAL::require = sub { $r = shift; 1; } }

require Foo;
is( $r, "Foo.pm" );

require Foo::Bar;
is( $r, join($dirsep, "Foo", "Bar.pm") );

require 'Foo';
is( $r, "Foo" );

require 5.006;
is( $r, "5.006" );

require v5.6;
ok( abs($r - 5.006) < 0.001 && $r eq "\x05\x06" );

eval "use Foo";
is( $r, "Foo.pm" );

eval "use Foo::Bar";
is( $r, join($dirsep, "Foo", "Bar.pm") );

{
    my @r;
    local *CORE::GLOBAL::require = sub { push @r, shift; 1; };
    eval "use 5.006";
    like( " @r ", qr " 5\.006 " );
}

{
    local $_ = 'foo.pm';
    require;
    is( $r, 'foo.pm' );
}

# localizing *CORE::GLOBAL::foo should revert to finding CORE::foo
{
    local(*CORE::GLOBAL::require);
    $r = '';
    eval "require NoNeXiSt;";
    ok( ! ( $r or $@ !~ /^Can't locate NoNeXiSt/i ) );
}

#
# readline() has special behaviour too
#

$r = 11;
BEGIN { *CORE::GLOBAL::readline = sub (;*) { ++$r }; }
is( <FH>	, 12 );
is( <$fh>	, 13 );
my $pad_fh;
is( <$pad_fh>	, 14 );
{
    my $buf = ''; $buf .= <FH>;
    is( $buf, 15, 'rcatline' );
}

# Non-global readline() override
BEGIN { *Rgs::readline = sub (;*) { --$r }; }
{
    package Rgs;
    ::is( <FH>	, 14 );
    ::is( <$fh>	, 13 );
    ::is( <$pad_fh>	, 12 );
    my $buf = ''; $buf .= <FH>;
    ::is( $buf, 11, 'rcatline' );
}

# Global readpipe() override
BEGIN { *CORE::GLOBAL::readpipe = sub ($) { "$_[0] " . --$r }; }
is( `rm`,	    "rm 10", '``' );
is( qx/cp/,	    "cp 9", 'qx' );

# Non-global readpipe() override
BEGIN { *Rgs::readpipe = sub ($) { ++$r . " $_[0]" }; }
{
    package Rgs;
    ::is( `rm`,		  "10 rm", '``' );
    ::is( qx/cp/,	  "11 cp", 'qx' );
}

# Verify that the parsing of overridden keywords isn't messed up
# by the indirect object notation
{
    local $SIG{__WARN__} = sub {
	::like( $_[0], qr/^ok overriden at/ );
    };
    BEGIN { *OverridenWarn::warn = sub { CORE::warn "@_ overriden"; }; }
    package OverridenWarn;
    sub foo { "ok" }
    warn( OverridenWarn->foo() );
    warn OverridenWarn->foo();
}
BEGIN { *OverridenPop::pop = sub { ::is( $_[0][0], "ok" ) }; }
{
    package OverridenPop;
    sub foo { [ "ok" ] }
    pop( OverridenPop->foo() );
    pop OverridenPop->foo();
}

{
    eval {
        local *CORE::GLOBAL::require = sub {
            CORE::require($_[0]);
        };
        require 5;
        require Text::ParseWords;
    };
    is $@, '';
}

# Constant inlining should not countermand "use subs" overrides
BEGIN { package other; *::caller = \&::caller }
sub caller() { 42 }
caller; # inline the constant
is caller, 42, 'constant inlining does not undo "use subs" on keywords';

is runperl(prog => 'sub CORE::GLOBAL::do; do file; print qq-ok\n-'),
  "ok\n",
  'no crash with CORE::GLOBAL::do stub';
is runperl(prog => 'sub CORE::GLOBAL::glob; glob; print qq-ok\n-'),
  "ok\n",
  'no crash with CORE::GLOBAL::glob stub';
is runperl(prog => 'sub CORE::GLOBAL::require; require re; print qq-o\n-'),
  "o\n",
  'no crash with CORE::GLOBAL::require stub';

like runperl(prog => 'use constant foo=>1; '
                    .'BEGIN { *{q|CORE::GLOBAL::readpipe|} = \&{q|foo|};1}'
                    .'warn ``',
             stderr => 1),
     qr/Too many arguments/,
    '`` does not ignore &CORE::GLOBAL::readpipe aliased to a constant';
like runperl(prog => 'use constant foo=>1; '
                    .'BEGIN { *{q|CORE::GLOBAL::readline|} = \&{q|foo|};1}'
                    .'warn <a>',
             stderr => 1),
     qr/Too many arguments/,
    '<> does not ignore &CORE::GLOBAL::readline aliased to a constant';

is runperl(prog => 'use constant t=>42; '
                  .'BEGIN { *{q|CORE::GLOBAL::time|} = \&{q|t|};1}'
                  .'print time, chr utf8::unicode_to_native(10)',
          stderr => 1),
   "42\n",
   'keywords respect global constant overrides';
