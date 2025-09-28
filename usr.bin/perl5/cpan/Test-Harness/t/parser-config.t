#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
our (%INIT, %CUSTOM);

use Test::More tests => 5;
use File::Spec::Functions qw( catfile updir );
use TAP::Parser;

use_ok('MyGrammar');
use_ok('MyResultFactory');

my @t_path    = ();
my $source    = catfile( @t_path, 't', 'source_tests', 'source' );
my %customize = (
    grammar_class        => 'MyGrammar',
    result_factory_class => 'MyResultFactory',
);
my $p = TAP::Parser->new(
    {   source => $source,
        %customize,
    }
);
ok( $p, 'new customized parser' );

for my $key ( keys %customize ) {
    is( $p->$key(), $customize{$key}, "customized $key" );
}

# TODO: make sure these things are propogated down through the parser...
