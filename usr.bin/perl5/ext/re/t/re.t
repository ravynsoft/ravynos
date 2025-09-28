#!./perl

BEGIN {
	require Config;
	if (($Config::Config{'extensions'} !~ /\bre\b/) ){
        	print "1..0 # Skip -- Perl configured without re module\n";
		exit 0;
	}
}

use strict;

my $re_taint_bit = 0x00100000;
my $re_eval_bit = 0x00200000;

use Test::More tests => 16;
require_ok( 're' );

# setcolor
$INC{ 'Term/Cap.pm' } = 1;
local $ENV{PERL_RE_TC};
re::setcolor();
is( $ENV{PERL_RE_COLORS}, "md\tme\tso\tse\tus\tue", 
	'setcolor() should provide default colors' );
$ENV{PERL_RE_TC} = 'su,n,ny';
re::setcolor();
is( $ENV{PERL_RE_COLORS}, "su\tn\tny", '... or use $ENV{PERL_RE_COLORS}' );

# bits
# get on
my $warn;
local $SIG{__WARN__} = sub {
	$warn = shift;
};
#eval { re::bits(1) };
#like( $warn, qr/Useless use/, 'bits() should warn with no args' );

delete $ENV{PERL_RE_COLORS};
re::bits(0, 'debug');
is( $ENV{PERL_RE_COLORS}, undef,
	"... should not set regex colors given 'debug'" );
re::bits(0, 'debugcolor');
isnt( $ENV{PERL_RE_COLORS}, '', 
	"... should set regex colors given 'debugcolor'" );
re::bits(0, 'nosuchsubpragma');
like( $warn, qr/Unknown "re" subpragma/, 
	'... should warn about unknown subpragma' );
ok( re::bits(0, 'taint') & $re_taint_bit, '... should set taint bits' );
ok( re::bits(0, 'eval')  & $re_eval_bit, '... should set eval bits' );

undef $warn;
eval "use re qw(debug ALL)";
like( $warn, qr/"Debug" not "debug"/, 'debug with debugging type should warn');

local $^H;

# import
re->import('taint', 'eval');
ok( $^H & $re_taint_bit, 'import should set taint bits in $^H when requested' );
ok( $^H & $re_eval_bit, 'import should set eval bits in $^H when requested' );

re->unimport('taint');
ok( !( $^H & $re_taint_bit ), 'unimport should clear bits in $^H when requested' );
re->unimport('eval');
ok( !( $^H & $re_eval_bit ), '... and again' );
my $reg=qr/(foo|bar|baz|blah)/;
close STDERR;
eval"use re Debug=>'ALL'";
my $ok='foo'=~/$reg/;
eval"no re Debug=>'ALL'";
ok( $ok, 'No segv!' );

my $message = "Don't tread on me";
$_ = $message;
re->import("/aa");
is($_, $message, "re doesn't clobber \$_");

package Term::Cap;

sub Tgetent {
	bless({}, $_[0]);
}

sub Tputs {
	return $_[1];
}

package main;

{
  my $w;
  local $SIG{__WARN__} = sub { warn shift; ++$w };
  re->import();
  is $w, undef, 'no warning for "use re;" (which is not useless)';
}
