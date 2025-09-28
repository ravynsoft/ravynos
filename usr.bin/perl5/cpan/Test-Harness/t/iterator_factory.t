#!/usr/bin/perl -w
#
# Tests for TAP::Parser::IteratorFactory & source detection
##

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;

use Test::More tests => 44;

use IO::File;
use File::Spec;
use TAP::Parser::Source;
use TAP::Parser::IteratorFactory;

# Test generic API...
{
    can_ok 'TAP::Parser::IteratorFactory', 'new';
    my $sf = TAP::Parser::IteratorFactory->new;
    isa_ok $sf, 'TAP::Parser::IteratorFactory';
    can_ok $sf, 'config';
    can_ok $sf, 'handlers';
    can_ok $sf, 'detect_source';
    can_ok $sf, 'make_iterator';
    can_ok $sf, 'register_handler';

    # Set config
    eval { $sf->config('bad config') };
    my $e = $@;
    like $e, qr/\QArgument to &config must be a hash reference/,
      '... and calling config with bad config should fail';

    my $config = { MySourceHandler => { foo => 'bar' } };
    is( $sf->config($config), $sf, '... and set config works' );

    # Load/Register a handler
    $sf = TAP::Parser::IteratorFactory->new(
        { MySourceHandler => { accept => 'known-source' } } );
    can_ok( 'MySourceHandler', 'can_handle' );
    is_deeply( $sf->handlers, ['MySourceHandler'], '... was registered' );

    # Known source should pass
    {
        my $source   = TAP::Parser::Source->new->raw( \'known-source' );
        my $iterator = eval { $sf->make_iterator($source) };
        my $error    = $@;
        ok( !$error, 'make_iterator with known source doesnt fail' );
        diag($error) if $error;
        isa_ok( $iterator, 'MyIterator', '... and iterator class' );
    }

    # No known source should fail
    {
        my $source   = TAP::Parser::Source->new->raw( \'unknown-source' );
        my $iterator = eval { $sf->make_iterator($source) };
        my $error    = $@;
        ok( $error, 'make_iterator with unknown source fails' );
        like $error, qr/^Cannot detect source of 'unknown-source'/,
          '... with an appropriate error message';
    }
}

# Source detection
use_ok('TAP::Parser::SourceHandler::Executable');
use_ok('TAP::Parser::SourceHandler::Perl');
use_ok('TAP::Parser::SourceHandler::File');
use_ok('TAP::Parser::SourceHandler::RawTAP');
use_ok('TAP::Parser::SourceHandler::Handle');

my $test_dir = File::Spec->catdir(
    't',
    'source_tests'
);

my @sources = (
    {   file     => 'source.tap',
        handler  => 'TAP::Parser::SourceHandler::File',
        iterator => 'TAP::Parser::Iterator::Stream',
    },
    {   file     => 'source.1',
        handler  => 'TAP::Parser::SourceHandler::File',
        config   => { File => { extensions => ['.1'] } },
        iterator => 'TAP::Parser::Iterator::Stream',
    },
    {   file     => 'source.pl',
        handler  => 'TAP::Parser::SourceHandler::Perl',
        iterator => 'TAP::Parser::Iterator::Process',
    },
    {   file     => 'source.t',
        handler  => 'TAP::Parser::SourceHandler::Perl',
        iterator => 'TAP::Parser::Iterator::Process',
    },
    {   file     => 'source',
        handler  => 'TAP::Parser::SourceHandler::Perl',
        iterator => 'TAP::Parser::Iterator::Process',
    },
    {   file     => 'source.sh',
        handler  => 'TAP::Parser::SourceHandler::Perl',
        iterator => 'TAP::Parser::Iterator::Process',
    },
    {   file     => 'source.bat',
        handler  => 'TAP::Parser::SourceHandler::Executable',
        iterator => 'TAP::Parser::Iterator::Process',
    },
    {   name     => 'raw tap string',
        source   => "0..1\nok 1 - raw tap\n",
        handler  => 'TAP::Parser::SourceHandler::RawTAP',
        iterator => 'TAP::Parser::Iterator::Array',
    },
    {   name     => 'raw tap array',
        source   => [ "0..1\n", "ok 1 - raw tap\n" ],
        handler  => 'TAP::Parser::SourceHandler::RawTAP',
        iterator => 'TAP::Parser::Iterator::Array',
    },
    {   source   => \*__DATA__,
        handler  => 'TAP::Parser::SourceHandler::Handle',
        iterator => 'TAP::Parser::Iterator::Stream',
    },
    {   source   => IO::File->new('-'),
        handler  => 'TAP::Parser::SourceHandler::Handle',
        iterator => 'TAP::Parser::Iterator::Stream',
    },
    {   file     => 'test.tap',
        tie      => 1,
    },
);

for my $test (@sources) {
    local $TODO = $test->{TODO};
    if ( $test->{file} ) {
        $test->{name} = $test->{file};
        $test->{source} = File::Spec->catfile( $test_dir, $test->{file} );
    }

    my $name = $test->{name} || substr( $test->{source}, 0, 10 );
    my $sf
      = TAP::Parser::IteratorFactory->new( $test->{config} )->_testing(1);

    my $raw      = $test->{source};
    my $source   = TAP::Parser::Source->new->raw( ref($raw) ? $raw : \$raw );
    my $iterator = eval { $sf->make_iterator($source) };
    my $error    = $@;

    if( $test->{tie} ) {
        like(
            $error, qr{^There is a tie.*Both voted .* on $test->{file}}ms,
            "$name: votes tied"
        )
    }
    else {
        ok( !$error, "$name: no error on make_iterator" );
        diag($error) if $error;
    }

    is( $sf->_last_handler, $test->{handler}, $name );
}

__END__
0..1
ok 1 - TAP in the __DATA__ handle
