#!./perl

BEGIN {
	chdir 't' if -d 't';
	@INC = '../lib';
	require './test.pl';
}

use strict;
use Config;

plan 16;

use_ok( 'sigtrap' );

package main;
local %SIG;

# use a version of sigtrap.pm somewhat too high
eval{ sigtrap->import(99999) };
like( $@, qr/version 99999 required,/, 'import excessive version number' );

# use an invalid signal name
eval{ sigtrap->import('abadsignal') };
like( $@, qr/^Unrecognized argument abadsignal/, 'send bad signame to import' );

eval{ sigtrap->import('handler') };
like( $@, qr/^No argument specified/, 'send handler without subref' );

sigtrap->import('AFAKE');
is( $SIG{AFAKE}, \&sigtrap::handler_traceback, 'install normal handler' );

sigtrap->import('die', 'AFAKE', 'stack-trace', 'FAKE2');
is( $SIG{AFAKE}, \&sigtrap::handler_die, 'install the die handler' );
is( $SIG{FAKE2}, \&sigtrap::handler_traceback, 'install traceback handler' );

my @normal = qw( HUP INT PIPE TERM );
@SIG{@normal} = 1 x @normal;
sigtrap->import('normal-signals');
is( (grep { ref $_ } @SIG{@normal}), @normal, 'check normal-signals set' );

my @error = qw( ABRT BUS EMT FPE ILL QUIT SEGV SYS TRAP );
@SIG{@error} = 1 x @error;
sigtrap->import('error-signals');
is( (grep { ref $_ } @SIG{@error}), @error, 'check error-signals set' );

my @old = qw( ABRT BUS EMT FPE ILL PIPE QUIT SEGV SYS TERM TRAP );
@SIG{@old} = 1 x @old;
sigtrap->import('old-interface-signals');
is( (grep { ref $_ } @SIG{@old}), @old, 'check old-interface-signals set' );

my $handler = sub {};
sigtrap->import(handler => $handler, 'FAKE3');
is( $SIG{FAKE3}, $handler, 'install custom handler' );

$SIG{FAKE} = 'IGNORE';
sigtrap->import('untrapped', 'FAKE');
is( $SIG{FAKE}, 'IGNORE', 'respect existing handler set to IGNORE' );

fresh_perl_like
  '
    BEGIN { *CORE::GLOBAL::kill = sub {} }
    require sigtrap;
    import sigtrap "INT";
    sub { $SIG{INT}->("INT") } -> (3)
  ',
   qr/\$ = main::__ANON__\(3\) called/,
  { stderr => 1 },
  "stack-trace does not try to modify read-only arguments"
;

my $out = tie *STDOUT, 'TieOut';
$SIG{FAKE} = 'DEFAULT';
$sigtrap::Verbose = 1;
sigtrap->import('any', 'FAKE');
my $read = $out->read;
untie *STDOUT;
is( $SIG{FAKE}, \&sigtrap::handler_traceback, 'should set default handler' );
like( $read, qr/^Installing handler/, 'does it talk with $Verbose set?' );

# handler_die croaks with first argument
eval { sigtrap::handler_die('FAKE') };
like( $@, qr/^Caught a SIGFAKE/, 'does handler_die() croak?' );
 
package TieOut;

sub TIEHANDLE {
	bless(\(my $scalar), $_[0]);
}

sub PRINT {
	my $self = shift;
	$$self .= join '', @_;
}

sub WRITE {
	my ($self, $msg, $length) = @_;
	$$self .= $msg;
}

sub read {
	my $self = shift;
	substr($$self, 0, length($$self), '');
}
