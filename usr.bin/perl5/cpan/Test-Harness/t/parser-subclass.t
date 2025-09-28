#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
our (%INIT, %CUSTOM);

use Test::More tests => 14;
use File::Spec::Functions qw( catfile updir );

use_ok('TAP::Parser::SubclassTest');

# TODO: for my $source ( ... ) ?
my @t_path = ();

{    # perl source
    %INIT = %CUSTOM = ();
    my $source = catfile( @t_path, 't', 'subclass_tests', 'perl_source' );
    my $p = TAP::Parser::SubclassTest->new( { source => $source } );

    # The grammar is lazily constructed so we need to ask for it to
    # trigger it's creation.
    my $grammer = $p->_grammar;

    ok( $p->{initialized}, 'new subclassed parser' );

    is( $p->grammar_class => 'MyGrammar', 'grammar_class' );
    is( $p->result_factory_class => 'MyResultFactory',
        'result_factory_class'
    );

    is( $INIT{MyGrammar},   1, 'initialized MyGrammar' );
    is( $CUSTOM{MyGrammar}, 1, '... and it was customized' );

    # make sure overrided make_* methods work...
    %CUSTOM = ();

    $p->make_grammar;
    is( $CUSTOM{MyGrammar}, 1, 'make custom grammar' );
    $p->make_result;
    is( $CUSTOM{MyResult}, 1, 'make custom result' );

    # make sure parser helpers use overrided classes too (the parser should
    # be the central source of configuration/overriding functionality)
    # The source is already tested above (parser doesn't keep a copy of the
    # source currently).  So only one to check is the Grammar:
    %INIT = %CUSTOM = ();
    my $r = $p->_grammar->tokenize;
    isa_ok( $r, 'MyResult', 'i has results' );
    is( $INIT{MyResult},        1, 'initialized MyResult' );
    is( $CUSTOM{MyResult},      1, '... and it was customized' );
    is( $INIT{MyResultFactory}, 1, '"initialized" MyResultFactory' );
}

SKIP: {    # non-perl source
    %INIT = %CUSTOM = ();
    my $cat = '/bin/cat';
    unless ( -e $cat ) {
        skip "no '$cat'", 2;
    }
    my $file = catfile( @t_path, 't', 'data', 'catme.1' );
    my $p = TAP::Parser::SubclassTest->new(
        {   exec => [ $cat => $file ],
            sources => { MySourceHandler => { accept_all => 1 } },
        }
    );

    is( $CUSTOM{MySourceHandler}, 1, 'customized a MySourceHandler' );
    is( $INIT{MyIterator},        1, 'initialized MyIterator subclass' );
}
