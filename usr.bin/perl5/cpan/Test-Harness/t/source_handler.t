#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;

use Test::More tests => 79;

use Config;
use IO::File;
use IO::Handle;
use File::Spec;

use TAP::Parser::Source;
use TAP::Parser::SourceHandler;

my $IS_WIN32 = ( $^O =~ /^(MS)?Win32$/ );
my $HAS_SH   = -x '/bin/sh';
my $HAS_ECHO = -x '/bin/echo';

my $dir = File::Spec->catdir(
    't',
    'source_tests'
);

my $perl = $^X;

my %file = map { $_ => File::Spec->catfile( $dir, $_ ) }
  qw( source source.1 source.bat source.pl source.sh source_args.sh source.t
  source.tap );

# Abstract base class tests
{
    my $class  = 'TAP::Parser::SourceHandler';
    my $source = TAP::Parser::Source->new;
    my $error;

    can_ok $class, 'can_handle';
    eval { $class->can_handle($source) };
    $error = $@;
    like $error, qr/^Abstract method 'can_handle'/,
      '... with an appropriate error message';

    can_ok $class, 'make_iterator';
    eval { $class->make_iterator($source) };
    $error = $@;
    like $error, qr/^Abstract method 'make_iterator'/,
      '... with an appropriate error message';
}

# Executable source tests
{
    my $class = 'TAP::Parser::SourceHandler::Executable';
    my $tests = {
        default_vote => 0,
        can_handle   => [
            {   name => '.sh',
                meta => {
                    is_file => 1,
                    file    => { lc_ext => '.sh' }
                },
                vote => 0,
            },
            {   name => '.bat',
                meta => {
                    is_file => 1,
                    file    => { lc_ext => '.bat' }
                },
                vote => 0.8,
            },
            {   name => 'executable bit',
                meta => {
                    is_file => 1,
                    file    => { lc_ext => '', execute => 1 }
                },
                vote => 0.25,
            },
            {   name => 'exec hash',
                raw  => { exec => 'foo' },
                meta => { is_hash => 1 },
                vote => 0.9,
            },
        ],
        make_iterator => [
            {   name => "valid executable",
                raw  => [
                    $perl, ( $ENV{PERL_CORE} ? '-I../../lib' : () ),
                    (map { "-I$_" } split /$Config{path_sep}/, $ENV{PERL5LIB} || ''),
                    '-It/lib', '-T', $file{source}
                ],
                iclass        => 'TAP::Parser::Iterator::Process',
                output        => [ '1..1', 'ok 1 - source' ],
                assemble_meta => 1,
            },
            {   name  => "invalid source->raw",
                raw   => "$perl -It/lib $file{source}",
                error => qr/^No command found/,
            },
            {   name  => "non-existent source->raw",
                raw   => [],
                error => qr/^No command found/,
            },
            {   name        => $file{'source.sh'},
                raw         => \$file{'source.sh'},
                skip        => $HAS_SH && $HAS_ECHO ? 0 : 1,
                skip_reason => 'no /bin/sh, /bin/echo',
                iclass      => 'TAP::Parser::Iterator::Process',
                output        => [ '1..1', 'ok 1 - source.sh' ],
                assemble_meta => 1,
            },
            {   name        => $file{'source_args.sh'},
                raw         => { exec => [ $file{'source_args.sh'} ] },
                test_args   => ['foo'],
                skip        => $HAS_SH && $HAS_ECHO ? 0 : 1,
                skip_reason => 'no /bin/sh, /bin/echo',
                iclass      => 'TAP::Parser::Iterator::Process',
                output        => [ '1..1', 'ok 1 - source_args.sh foo' ],
                assemble_meta => 1,
            },
            {   name        => $file{'source.bat'},
                raw         => \$file{'source.bat'},
                skip        => $IS_WIN32 ? 0 : 1,
                skip_reason => 'not running Win32',
                iclass      => 'TAP::Parser::Iterator::Process',
                output        => [ '1..1', 'ok 1 - source.bat' ],
                assemble_meta => 1,
            },
        ],
    };

    test_handler( $class, $tests );
}

# Perl source tests
{
    my $class = 'TAP::Parser::SourceHandler::Perl';
    my $tests = {
        default_vote => 0,
        can_handle   => [
            {   name => '.t',
                meta => {
                    is_file => 1,
                    file    => { lc_ext => '.t', dir => '' }
                },
                vote => 0.8,
            },
            {   name => '.pl',
                meta => {
                    is_file => 1,
                    file    => { lc_ext => '.pl', dir => '' }
                },
                vote => 0.9,
            },
            {   name => 't/.../file',
                meta => {
                    is_file => 1,
                    file    => { lc_ext => '', dir => 't' }
                },
                vote => 0.75,
            },
            {   name => '#!...perl',
                meta => {
                    is_file => 1,
                    file    => {
                        lc_ext => '', dir => '', shebang => '#!/usr/bin/perl'
                    }
                },
                vote => 0.9,
            },
            {   name => 'file default',
                meta => {
                    is_file => 1,
                    file    => { lc_ext => '', dir => '' }
                },
                vote => 0.25,
            },
        ],
        make_iterator => [
            {   name          => $file{source},
                raw           => \$file{source},
                iclass        => 'TAP::Parser::Iterator::Process',
                output        => [ '1..1', 'ok 1 - source' ],
                assemble_meta => 1,
            },
        ],
    };

    test_handler( $class, $tests );

    # internals tests!
    {
        my $source = TAP::Parser::Source->new->raw( \$file{source} );
        $source->assemble_meta;
        my $iterator = $class->make_iterator($source);
        my @command  = @{ $iterator->{command} };
        ok( grep( $_ =~ /^['"]?-T['"]?$/, @command ),
            '... and it should find the taint switch'
        );
    }
}

# Raw TAP source tests
{
    my $class = 'TAP::Parser::SourceHandler::RawTAP';
    my $tests = {
        default_vote => 0,
        can_handle   => [
            {   name => 'file',
                meta => { is_file => 1 },
                raw  => \'',
                vote => 0,
            },
            {   name          => 'scalar w/newlines',
                raw           => \"hello\nworld\n",
                vote          => 0.3,
                assemble_meta => 1,
            },
            {   name          => '1..10',
                raw           => \"1..10\n",
                vote          => 0.9,
                assemble_meta => 1,
            },
            {   name          => 'array',
                raw           => [ '1..1', 'ok 1' ],
                vote          => 0.5,
                assemble_meta => 1,
            },
        ],
        make_iterator => [
            {   name          => 'valid scalar',
                raw           => \"1..1\nok 1 - raw\n",
                iclass        => 'TAP::Parser::Iterator::Array',
                output        => [ '1..1', 'ok 1 - raw' ],
                assemble_meta => 1,
            },
            {   name          => 'valid array',
                raw           => [ '1..1', 'ok 1 - raw' ],
                iclass        => 'TAP::Parser::Iterator::Array',
                output        => [ '1..1', 'ok 1 - raw' ],
                assemble_meta => 1,
            },
        ],
    };

    test_handler( $class, $tests );
}

# Text file TAP source tests
{
    my $class = 'TAP::Parser::SourceHandler::File';
    my $tests = {
        default_vote => 0,
        can_handle   => [
            {   name => '.tap',
                meta => {
                    is_file => 1,
                    file    => { lc_ext => '.tap' }
                },
                vote => 0.9,
            },
            {   name => '.foo with config',
                meta => {
                    is_file => 1,
                    file    => { lc_ext => '.foo' }
                },
                config => { File => { extensions => ['.foo'] } },
                vote   => 0.9,
            },
        ],
        make_iterator => [
            {   name          => $file{'source.tap'},
                raw           => \$file{'source.tap'},
                iclass        => 'TAP::Parser::Iterator::Stream',
                output        => [ '1..1', 'ok 1 - source.tap' ],
                assemble_meta => 1,
            },
            {   name   => $file{'source.1'},
                raw    => \$file{'source.1'},
                config => { File => { extensions => ['.1'] } },
                iclass => 'TAP::Parser::Iterator::Stream',
                output        => [ '1..1', 'ok 1 - source.1' ],
                assemble_meta => 1,
            },
        ],
    };

    test_handler( $class, $tests );
}

# IO::Handle TAP source tests
{
    my $class = 'TAP::Parser::SourceHandler::Handle';
    my $tests = {
        default_vote => 0,
        can_handle   => [
            {   name => 'glob',
                meta => { is_glob => 1 },
                vote => 0.8,
            },
            {   name          => 'IO::Handle',
                raw           => IO::Handle->new,
                vote          => 0.9,
                assemble_meta => 1,
            },
        ],
        make_iterator => [
            {   name          => 'IO::Handle',
                raw           => IO::File->new( $file{'source.tap'} ),
                iclass        => 'TAP::Parser::Iterator::Stream',
                output        => [ '1..1', 'ok 1 - source.tap' ],
                assemble_meta => 1,
            },
        ],
    };

    test_handler( $class, $tests );
}

###############################################################################
# helper sub

sub test_handler {
    my ( $class, $tests ) = @_;
    my ($short_class) = ( $class =~ /\:\:(\w+)$/ );

    use_ok $class;
    can_ok $class, 'can_handle', 'make_iterator';

    {
        my $default_vote = $tests->{default_vote} || 0;
        my $source = TAP::Parser::Source->new;
        is( $class->can_handle($source), $default_vote,
            '... can_handle default vote'
        );
    }

    for my $test ( @{ $tests->{can_handle} } ) {
        my $source = TAP::Parser::Source->new;
        $source->raw( $test->{raw} )       if $test->{raw};
        $source->meta( $test->{meta} )     if $test->{meta};
        $source->config( $test->{config} ) if $test->{config};
        $source->assemble_meta             if $test->{assemble_meta};
        my $vote = $test->{vote} || 0;
        my $name = $test->{name} || 'unnamed test';
        $name = "$short_class->can_handle( $name )";
        is( $class->can_handle($source), $vote, $name );
    }

    for my $test ( @{ $tests->{make_iterator} } ) {
        my $name = $test->{name} || 'unnamed test';
        $name = "$short_class->make_iterator( $name )";

        SKIP:
        {
            my $planned = 1;
            $planned += 1 + scalar @{ $test->{output} } if $test->{output};
            skip $test->{skip_reason}, $planned if $test->{skip};

            my $source = TAP::Parser::Source->new;
            $source->raw( $test->{raw} )             if $test->{raw};
            $source->test_args( $test->{test_args} ) if $test->{test_args};
            $source->meta( $test->{meta} )           if $test->{meta};
            $source->config( $test->{config} )       if $test->{config};
            $source->assemble_meta if $test->{assemble_meta};

            my $iterator = eval { $class->make_iterator($source) };
            my $e = $@;
            if ( my $error = $test->{error} ) {
                $e = '' unless defined $e;
                like $e, $error, "$name threw expected error";
                next;
            }
            elsif ($e) {
                fail("$name threw an unexpected error");
                diag($e);
                next;
            }

            isa_ok $iterator, $test->{iclass}, $name;
            if ( $test->{output} ) {
                my $i = 1;
                for my $line ( @{ $test->{output} } ) {
                    is $iterator->next, $line, "... line $i";
                    $i++;
                }
                ok !$iterator->next, '... and we should have no more results';
            }
        }
    }
}
