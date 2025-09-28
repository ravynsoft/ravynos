#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;

use Test::More tests => 45;
use File::Spec;

my $dir = 't/source_tests';

use_ok('TAP::Parser::Source');

sub ct($) {
    my $hash = shift;
    if ( $ENV{PERL_CORE} ) {
        delete $hash->{is_symlink};
        delete $hash->{lstat};
    }
    return $hash;
}

# Basic tests
{
    my $source = TAP::Parser::Source->new;
    isa_ok( $source, 'TAP::Parser::Source', 'new source' );
    can_ok(
        $source,
        qw( raw meta config merge switches test_args assemble_meta )
    );

    is_deeply( $source->config, {}, 'config empty by default' );
    $source->config->{Foo} = { bar => 'baz' };
    is_deeply(
        $source->config_for('Foo'), { bar => 'baz' },
        'config_for( Foo )'
    );
    is_deeply(
        $source->config_for('TAP::Parser::SourceHandler::Foo'),
        { bar => 'baz' }, 'config_for( ...::SourceHandler::Foo )'
    );

    ok( !$source->merge, 'merge not set by default' );
    $source->merge(1);
    ok( $source->merge, '... merge now set' );

    is( $source->switches, undef, 'switches not set by default' );
    $source->switches( ['-Ilib'] );
    is_deeply( $source->switches, ['-Ilib'], '... switches now set' );

    is( $source->test_args, undef, 'test_args not set by default' );
    $source->test_args( ['foo'] );
    is_deeply( $source->test_args, ['foo'], '... test_args now set' );

    $source->raw( \'hello world' );
    my $meta = $source->assemble_meta;
    is_deeply(
        $meta,
        {   is_scalar    => 1,
            is_object    => 0,
            has_newlines => 0,
            length       => 11,
        },
        'assemble_meta for scalar that isnt a file'
    );

    is( $source->meta, $meta, '... and caches meta' );
}

# array check
{
    my $source = TAP::Parser::Source->new;
    $source->raw( [ 'hello', 'world' ] );
    my $meta = $source->assemble_meta;
    is_deeply(
        $meta,
        {   is_array  => 1,
            is_object => 0,
            size      => 2,
        },
        'assemble_meta for array'
    );
}

# hash check
{
    my $source = TAP::Parser::Source->new;
    $source->raw( { hello => 'world' } );
    my $meta = $source->assemble_meta;
    is_deeply(
        $meta,
        {   is_hash   => 1,
            is_object => 0,
        },
        'assemble_meta for array'
    );
}

# glob check
{
    my $source = TAP::Parser::Source->new;
    $source->raw( \*__DATA__ );
    my $meta = $source->assemble_meta;
    is_deeply(
        $meta,
        {   is_glob   => 1,
            is_object => 0,
        },
        'assemble_meta for array'
    );
}

# object check
{
    my $source = TAP::Parser::Source->new;
    $source->raw( bless {}, 'Foo::Bar' );
    my $meta = $source->assemble_meta;
    is_deeply(
        $meta,
        {   is_object => 1,
            class     => 'Foo::Bar',
        },
        'assemble_meta for array'
    );
}

# file test
{
    my $test = File::Spec->catfile( $dir, 'source.t' );
    my $source = TAP::Parser::Source->new;

    $source->raw( \$test );
    my $meta = $source->assemble_meta;

    # separate meta->file to break up the test
    my $file = delete $meta->{file};
    is_deeply(
        ct $meta,
        ct {is_scalar    => 1,
            has_newlines => 0,
            length       => length($test),
            is_object    => 0,
            is_file      => 1,
            is_dir       => 0,
            is_symlink   => 0,
        },
        'assemble_meta for file'
    );

    # now check file meta - remove things that will vary between platforms
    my $stat = delete $file->{stat};
    is( @$stat, 13, '... file->stat set' );
    ok( delete $file->{size}, '... file->size set' );
    ok( delete $file->{dir},  '... file->dir set' );
    isnt( delete $file->{read},    undef, '... file->read set' );
    isnt( delete $file->{write},   undef, '... file->write set' );
    isnt( delete $file->{execute}, undef, '... file->execute set' );
    is_deeply(
        ct $file,
        ct {basename   => 'source.t',
            ext        => '.t',
            lc_ext     => '.t',
            shebang    => '#!/usr/bin/perl',
            binary     => 0,
            text       => 1,
            empty      => 0,
            exists     => 1,
            is_dir     => 0,
            is_file    => 1,
            is_symlink => 0,

            # Fix for bizarre -k bug in Strawberry Perl
            sticky => ( -k $test )[-1] ? 1 : 0,
            setgid => -g $test         ? 1 : 0,
            setuid => -u $test         ? 1 : 0,
        },
        '... file->* set'
    );
}

# dir test
{
    my $test   = $dir;
    my $source = TAP::Parser::Source->new;

    $source->raw( \$test );
    my $meta = $source->assemble_meta;

    # separate meta->file to break up the test
    my $file = delete $meta->{file};
    is_deeply(
        ct $meta,
        ct {is_scalar    => 1,
            has_newlines => 0,
            length       => length($test),
            is_object    => 0,
            is_file      => 0,
            is_dir       => 1,
            is_symlink   => 0,
        },
        'assemble_meta for directory'
    );

    # now check file meta - remove things that will vary between platforms
    my $stat = delete $file->{stat};
    is( @$stat, 13, '... file->stat set' );
    ok( delete $file->{dir}, '... file->dir set' );
    isnt( delete $file->{size},    undef, '... file->size set' );
    isnt( delete $file->{binary},  undef, '... file->binary set' );
    isnt( delete $file->{empty},   undef, '... file->empty set' );
    isnt( delete $file->{read},    undef, '... file->read set' );
    isnt( delete $file->{write},   undef, '... file->write set' );
    isnt( delete $file->{execute}, undef, '... file->execute set' );
    is_deeply(
        ct $file,
        ct {basename   => 'source_tests',
            ext        => '',
            lc_ext     => '',
            text       => 0,
            exists     => 1,
            is_dir     => 1,
            is_file    => 0,
            is_symlink => 0,
            sticky     => ( -k $test )[-1] ? 1 : 0,
            setgid     => -g $test ? 1 : 0,
            setuid     => -u $test ? 1 : 0,
        },
        '... file->* set'
    );
}

# symlink test
SKIP: {
    my $symlink_exists = eval { symlink( '', '' ); 1 };
    $symlink_exists = 0 if $^O eq 'VMS'; # exists but not ready for prime time
    skip 'symlink not supported on this platform', 9 unless $symlink_exists;

    my $test    = File::Spec->catfile( $dir, 'source.t' );
    my $symlink = File::Spec->catfile( $dir, 'source_link.T' );
    my $source  = TAP::Parser::Source->new;

    my $did_symlink = eval { symlink( File::Spec->rel2abs($test), $symlink ) };
    if ( my $e = $@ ) {
        diag($@);
        die "aborting test";
    }
    skip "symlink not successful: $!", 9 unless $did_symlink;

    $source->raw( \$symlink );
    my $meta = $source->assemble_meta;

    # separate meta->file to break up the test
    my $file = delete $meta->{file};
    is_deeply(
        ct $meta,
        ct {is_scalar    => 1,
            has_newlines => 0,
            length       => length($symlink),
            is_object    => 0,
            is_file      => 1,
            is_dir       => 0,
            is_symlink   => 1,
        },
        'assemble_meta for symlink'
    );

    # now check file meta - remove things that will vary between platforms
    my $stat = delete $file->{stat};
    is( @$stat, 13, '... file->stat set' );
    my $lstat = delete $file->{lstat};
    is( @$lstat, 13, '... file->lstat set' );
    ok( delete $file->{size}, '... file->size set' );
    ok( delete $file->{dir},  '... file->dir set' );
    isnt( delete $file->{read},    undef, '... file->read set' );
    isnt( delete $file->{write},   undef, '... file->write set' );
    isnt( delete $file->{execute}, undef, '... file->execute set' );
    is_deeply(
        ct $file,
        ct {basename   => 'source_link.T',
            ext        => '.T',
            lc_ext     => '.t',
            shebang    => '#!/usr/bin/perl',
            binary     => 0,
            text       => 1,
            empty      => 0,
            exists     => 1,
            is_dir     => 0,
            is_file    => 1,
            is_symlink => 1,
            sticky     => ( -k $symlink )[-1] ? 1 : 0,
            setgid     => -g $symlink ? 1 : 0,
            setuid     => -u $symlink ? 1 : 0,
        },
        '... file->* set'
    );

    unlink $symlink;
}

