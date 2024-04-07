#!/usr/bin/perl -T
use strict;
use warnings;

use Config;
use Test::More $Config{ccflags} =~ /-DSILENT_NO_TAINT_SUPPORT/
    ? ( skip_all => 'No taint support' ) : ( tests => 2 );
use Module::Metadata;
use Carp 'croak';

# stolen liberally from Class-Tiny/t/lib/TestUtils.pm - thanks xdg!
sub exception(&) {
    my $code = shift;
    my $success = eval { $code->(); 1 };
    my $err = $@;
    return undef if $success;   # original returned ''
    croak "Execution died, but the error was lost" unless $@;
    return $@;
}

my $taint_on = ! eval { no warnings; join('',values %ENV), kill 0; 1; };
ok($taint_on, 'taint flag is set');

# without the fix, we get:
# Insecure dependency in eval while running with -T switch at lib/Module/Metadata.pm line 668, <GEN0> line 15.
is(
    exception { Module::Metadata->new_from_module( "Module::Metadata" )->version },
    undef,
    'no exception',
);

