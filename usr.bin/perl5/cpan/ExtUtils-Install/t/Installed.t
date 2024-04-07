#!/usr/bin/perl -w
use strict;

# Test ExtUtils::Installed

BEGIN {
    # For backwards compatibility, use bundled version of Test::More
    unshift @INC, 't/lib/';
}

my $Is_VMS = $^O eq 'VMS';


use Config;
use Cwd;
use File::Path;
use File::Basename;
use File::Spec;
use File::Temp qw[tempdir];

use Test::More tests => 76;

BEGIN { use_ok( 'ExtUtils::Installed' ) }

my $mandirs =  !!$Config{man1direxp} + !!$Config{man3direxp};

# saves having to qualify package name for class methods
my $ei = bless( {}, 'ExtUtils::Installed' );

# Make sure meta info is available
$ei->{':private:'}{Config} = \%Config;
$ei->{':private:'}{INC} = \@INC;

# _is_prefix
ok( $ei->_is_prefix('foo/bar', 'foo'),
        '_is_prefix() should match valid path prefix' );
ok( !$ei->_is_prefix('\foo\bar', '\bar'),
        '... should not match wrong prefix' );
ok( ! defined $ei->_is_prefix( undef, 'foo' ),
    '_is_prefix() needs two defined arguments' );
ok( ! defined $ei->_is_prefix( 'foo/bar', undef ),
    '_is_prefix() needs two defined arguments' );

# _is_type
ok( $ei->_is_type(0, 'all'), '_is_type() should be true for type of "all"' );

foreach my $path (qw( man1dir man3dir )) {
    SKIP: {
        my $dir = File::Spec->canonpath($Config{$path.'exp'});
        skip("no man directory $path on this system", 2 ) unless $dir;

        my $file = $dir . '/foo';
        ok( $ei->_is_type($file, 'doc'),   "... should find doc file in $path" );
        ok( !$ei->_is_type($file, 'prog'), "... but not prog file in $path" );
    }
}

# VMS 5.6.1 doesn't seem to have $Config{prefixexp}
my $prefix = $Config{prefix} || $Config{prefixexp};

# You can concatenate /foo but not foo:, which defaults in the current
# directory
$prefix = VMS::Filespec::unixify($prefix) if $Is_VMS;

# ActivePerl 5.6.1/631 has $Config{prefixexp} as 'p:' for some reason
$prefix = $Config{prefix} if $prefix eq 'p:' && $^O eq 'MSWin32';

ok( $ei->_is_type( File::Spec->catfile($prefix, 'bar'), 'prog'),
        "... should find prog file under $prefix" );

SKIP: {
    skip('no man directories on this system', 1) unless $mandirs;
    is( $ei->_is_type('bar', 'doc'), 0,
        '... should not find doc file outside path' );
}

ok( !$ei->_is_type('bar', 'prog'),
        '... nor prog file outside path' );
ok( !$ei->_is_type('whocares', 'someother'), '... nor other type anywhere' );

# _is_under
ok( $ei->_is_under('foo'), '_is_under() should return true with no dirs' );

my @under = qw( boo bar baz );
ok( !$ei->_is_under('foo', @under), '... should find no file not under dirs');
ok( $ei->_is_under('baz', @under),  '... should find file under dir' );

my $startdir = cwd();
END { ok(chdir $startdir, "Return to where we started"); }

{
    my $tmpdir = tempdir( CLEANUP => 1 );
    chdir $tmpdir;

    my $fakedir = 'FakeMod';
    my $fakepath = File::Spec->catdir('auto', $fakedir);
    ok( mkpath($fakepath), "Able to create directory $fakepath for testing" );

    ok(open(PACKLIST, '>', File::Spec->catfile($fakepath, '.packlist')),
        "Able to open .packlist for writing");
    print PACKLIST 'list';
    close PACKLIST;

    ok(open(FAKEMOD, '>', File::Spec->catfile($fakepath, 'FakeMod.pm')),
        "Able to open FakeMod.pm for writing");

    print FAKEMOD <<'FAKE';
package FakeMod;
our $VERSION = '1.1.1';
1;
FAKE

    close FAKEMOD;

    my $fake_mod_dir = File::Spec->catdir(cwd(), $fakepath);
    {
        # avoid warning and death by localizing glob
        local *ExtUtils::Installed::Config;
        %ExtUtils::Installed::Config = (
            %Config,
            archlibexp         => cwd(),
            sitearchexp        => $fake_mod_dir,
        );

        # should find $fake_mod_dir via '.' in @INC

        local @INC = @INC;
        push @INC, '.' if not $INC[-1] eq '.';

        my $realei = ExtUtils::Installed->new();
        isa_ok( $realei, 'ExtUtils::Installed' );
        isa_ok( $realei->{Perl}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{Perl}{version}, $Config{version},
            'new() should set Perl version from %Config' );

        ok( exists $realei->{FakeMod}, 'new() should find modules with .packlists');
        isa_ok( $realei->{FakeMod}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{FakeMod}{version}, '1.1.1',
            '... should find version in modules' );
    }

    {
        # avoid warning and death by localizing glob
        local *ExtUtils::Installed::Config;
        %ExtUtils::Installed::Config = (
            %Config,
            archlibexp         => cwd(),
            sitearchexp        => $fake_mod_dir,
        );

        # disable '.' search

        my $realei = ExtUtils::Installed->new( skip_cwd => 1 );
        isa_ok( $realei, 'ExtUtils::Installed' );
        isa_ok( $realei->{Perl}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{Perl}{version}, $Config{version},
            'new() should set Perl version from %Config' );

        ok( ! exists $realei->{FakeMod}, 'new( skip_cwd => 1 ) should fail to find modules with .packlists');
    }

    {
        # avoid warning and death by localizing glob
        local *ExtUtils::Installed::Config;
        %ExtUtils::Installed::Config = (
            %Config,
            archlibexp         => cwd(),
            sitearchexp        => $fake_mod_dir,
        );

        # necessary to fool new() since we'll disable searching '.'
        push @INC, $fake_mod_dir;

        my $realei = ExtUtils::Installed->new( skip_cwd => 1 );
        isa_ok( $realei, 'ExtUtils::Installed' );
        isa_ok( $realei->{Perl}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{Perl}{version}, $Config{version},
            'new() should set Perl version from %Config' );

        ok( exists $realei->{FakeMod}, 'new() should find modules with .packlists');
        isa_ok( $realei->{FakeMod}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{FakeMod}{version}, '1.1.1',
            '... should find version in modules' );
    }

    # Now try this using PERL5LIB
    {
        local $ENV{PERL5LIB} = join $Config{path_sep}, $fake_mod_dir;
        local *ExtUtils::Installed::Config;
        %ExtUtils::Installed::Config = (
            %Config,
            archlibexp         => cwd(),
            sitearchexp        => cwd(),
        );

        my $realei = ExtUtils::Installed->new();
        isa_ok( $realei, 'ExtUtils::Installed' );
        isa_ok( $realei->{Perl}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{Perl}{version}, $Config{version},
            'new() should set Perl version from %Config' );

        ok( exists $realei->{FakeMod},
            'new() should find modules with .packlists using PERL5LIB'
        );
        isa_ok( $realei->{FakeMod}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{FakeMod}{version}, '1.1.1',
            '... should find version in modules' );
    }

    # Do the same thing as the last block, but with overrides for
    # %Config and @INC.
    {
        my $config_override = { %Config::Config };
        $config_override->{archlibexp} = cwd();
        $config_override->{sitearchexp} = $fake_mod_dir;
        $config_override->{version} = 'fake_test_version';

        my @inc_override = (@INC, $fake_mod_dir);

        my $realei = ExtUtils::Installed->new(
            'config_override' => $config_override,
            'inc_override' => \@inc_override,
        );
        isa_ok( $realei, 'ExtUtils::Installed' );
        isa_ok( $realei->{Perl}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{Perl}{version}, 'fake_test_version',
            'new(config_override => HASH) overrides %Config' );

        ok( exists $realei->{FakeMod}, 'new() with overrides should find modules with .packlists');
        isa_ok( $realei->{FakeMod}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{FakeMod}{version}, '1.1.1',
            '... should find version in modules' );
    }

    # Check if extra_libs works.
    {
        my $realei = ExtUtils::Installed->new(
            'extra_libs' => [ cwd() ],
        );
        isa_ok( $realei, 'ExtUtils::Installed' );
        isa_ok( $realei->{Perl}{packlist}, 'ExtUtils::Packlist' );
        ok( exists $realei->{FakeMod},
            'new() with extra_libs should find modules with .packlists');

        #{ use Data::Dumper; local $realei->{':private:'}{Config};
        #  warn Dumper($realei); }

        isa_ok( $realei->{FakeMod}{packlist}, 'ExtUtils::Packlist' );
        is( $realei->{FakeMod}{version}, '1.1.1',
            '... should find version in modules' );
    }

    # modules
    $ei->{$_} = 1 for qw( abc def ghi );
    is( join(' ', $ei->modules()), 'abc def ghi',
        'modules() should return sorted keys' );

    # This didn't work for a long time due to a sort in scalar context oddity.
    is( $ei->modules, 3,    'modules() in scalar context' );

    # files
    $ei->{goodmod} = {
            packlist => {
                    ($Config{man1direxp} ?
                        (File::Spec->catdir($Config{man1direxp}, 'foo') => 1) :
                            ()),
                    ($Config{man3direxp} ?
                        (File::Spec->catdir($Config{man3direxp}, 'bar') => 1) :
                            ()),
                    File::Spec->catdir($prefix, 'foobar') => 1,
                    foobaz  => 1,
            },
    };

    eval { $ei->files('badmod') };
    like( $@, qr/badmod is not installed/,'files() should croak given bad modname');
    eval { $ei->files('goodmod', 'badtype' ) };
    like( $@, qr/type must be/,'files() should croak given bad type' );

    my @files;
    SKIP: {
        skip('no man directory man1dir on this system', 2)
          unless $Config{man1direxp};
        @files = $ei->files('goodmod', 'doc', $Config{man1direxp});
        is( scalar @files, 1, '... should find doc file under given dir' );
        is( (grep { /foo$/ } @files), 1, '... checking file name' );
    }
    SKIP: {
        skip('no man directories on this system', 1) unless $mandirs;
        @files = $ei->files('goodmod', 'doc');
        is( scalar @files, $mandirs, '... should find all doc files with no dir' );
    }

    @files = $ei->files('goodmod', 'prog', 'fake', 'fake2');
    is( scalar @files, 0, '... should find no doc files given wrong dirs' );
    @files = $ei->files('goodmod', 'prog');
    is( scalar @files, 1, '... should find doc file in correct dir' );
    like( $files[0], qr/foobar[>\]]?$/, '... checking file name' );
    @files = $ei->files('goodmod');
    is( scalar @files, 2 + $mandirs, '... should find all files with no type specified' );
    my %dirnames = map { lc($_) => dirname($_) } @files;

    # directories
    my @dirs = $ei->directories('goodmod', 'prog', 'fake');
    is( scalar @dirs, 0, 'directories() should return no dirs if no files found' );

    SKIP: {
        skip('no man directories on this system', 1) unless $mandirs;
        @dirs = $ei->directories('goodmod', 'doc');
        is( scalar @dirs, $mandirs, '... should find all files files() would' );
    }
    @dirs = $ei->directories('goodmod');
    is( scalar @dirs, 2 + $mandirs, '... should find all files files() would, again' );
    @files = sort map { exists $dirnames{lc($_)} ? $dirnames{lc($_)} : '' } @files;
    is( join(' ', @files), join(' ', @dirs), '... should sort output' );

    # directory_tree
    my $expectdirs =
           ($mandirs == 2) &&
           (dirname($Config{man1direxp}) eq dirname($Config{man3direxp}))
           ? 3 : 2;

    SKIP: {
        skip('no man directories on this system', 1) unless $mandirs;
        @dirs = $ei->directory_tree('goodmod', 'doc', $Config{man1direxp} ?
           dirname($Config{man1direxp}) : dirname($Config{man3direxp}));
        is( scalar @dirs, $expectdirs,
            'directory_tree() should report intermediate dirs to those requested' );
    }

    my $fakepak = Fakepak->new(102);

    $ei->{yesmod} = {
            version         => 101,
            packlist        => $fakepak,
    };

    # these should all croak
    foreach my $sub (qw( validate packlist version )) {
        eval { $ei->$sub('nomod') };
        like( $@, qr/nomod is not installed/,
            "$sub() should croak when asked about uninstalled module" );
    }

    # validate
    is( $ei->validate('yesmod'), 'validated',
            'validate() should return results of packlist validate() call' );

    # packlist
    is( ${ $ei->packlist('yesmod') }, 102,
            'packlist() should report installed mod packlist' );

    # version
    is( $ei->version('yesmod'), 101,
            'version() should report installed mod version' );

} # End of block enclosing tempdir

package Fakepak;

sub new {
    my $class = shift;
    bless(\(my $scalar = shift), $class);
}

sub validate {
    return 'validated'
}
