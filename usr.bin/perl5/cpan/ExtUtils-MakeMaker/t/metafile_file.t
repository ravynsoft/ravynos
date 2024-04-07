#!/usr/bin/perl -w

# This is a test of the fake YAML dumper implemented by EUMM:
#   ExtUtils::MM_Any::metafile_file

BEGIN {
    unshift @INC, 't/lib';
}

use strict;
use warnings;
use Test::More tests => 16;

require ExtUtils::MM_Any;

my $mm = bless {}, 'ExtUtils::MM_Any';

{
    my @meta = ( a => 1, b => 2 );
    my $expected = <<YAML;
--- #YAML:1.0
a:  1
b:  2
YAML

    is($mm->metafile_file(@meta), $expected, "dump for flat hashes works ok");
}

{
    my @meta = ( k1 => 'some key and value', k2 => undef, k3 => 1 );
    my $expected = <<YAML;
--- #YAML:1.0
k1:  some key and value
k2:  ~
k3:  1
YAML

    is($mm->metafile_file(@meta), $expected, "dumping strings and undefs is ok");
}

{
    my @meta = ( a => 1, b => 2, h => { hh => 1 } );
    my $expected = <<YAML;
--- #YAML:1.0
a:  1
b:  2
h:
    hh:  1
YAML

    is($mm->metafile_file(@meta), $expected, "dump for nested hashes works ok");
}

{
    my @meta = ( a => 1, b => 2, h => { h1 => 'x', h2 => 'z' } );
    my $expected = <<YAML;
--- #YAML:1.0
a:  1
b:  2
h:
    h1:  x
    h2:  z
YAML

    is($mm->metafile_file(@meta), $expected, "nested hashes sort ascii-betically");
    # to tell the truth, they sort case-insensitively
    # that's hard to test for Perl with unstable sort's
}

{
    my @meta = ( a => 1, b => 2, h => { hh => { hhh => 1 } } );
    my $expected = <<YAML;
--- #YAML:1.0
a:  1
b:  2
h:
    hh:
        hhh:  1
YAML

    is($mm->metafile_file(@meta), $expected, "dump for hashes (with more nesting) works ok");
}

{
    my @meta = ( a => 1, k => [ qw(w y z) ] );
    my $expected = <<YAML;
--- #YAML:1.0
a:  1
k:
    - w
    - y
    - z
YAML

    is($mm->metafile_file(@meta), $expected, "array of strings are handled ok");
}

is($mm->metafile_file( a => {}, b => [], c => undef ), <<'YAML', 'empty hashes and arrays');
--- #YAML:1.0
a:  {}
b:  []
c:  ~
YAML


{
    my @meta = (
        name => 'My-Module',
        version => '0.1',
        version_from => 'lib/My/Module.pm',
        installdirs => 'site',
        abstract => 'A does-it-all module',
        license => 'perl',
        generated_by => 'myself',
        author => 'John Doe <doe@doeland.org>',
        distribution_type => 'module',
        requires => {
            'My::Module::Helper' => 0,
            'Your::Module' => '1.5',
        },
        'meta-spec' => {
            version => '1.1',
            url => 'http://module-build.sourceforge.net/META-spec-new.html',
        },
    );
    my $expected = <<'YAML';
--- #YAML:1.0
name:               My-Module
version:            0.1
version_from:       lib/My/Module.pm
installdirs:        site
abstract:           A does-it-all module
license:            perl
generated_by:       myself
author:             John Doe <doe@doeland.org>
distribution_type:  module
requires:
    My::Module::Helper:  0
    Your::Module:        1.5
meta-spec:
    url:      http://module-build.sourceforge.net/META-spec-new.html
    version:  1.1
YAML

    is($mm->metafile_file(@meta), $expected, "dump for something like META.yml works");
}

{
    my @meta = (
        name => 'My-Module',
        version => '0.1',
        version_from => 'lib/My/Module.pm',
        installdirs => 'site',
        abstract => 'A does-it-all module',
        license => 'perl',
        generated_by => 'myself',
        author => 'John Doe <doe@doeland.org>',
        distribution_type => 'module',
        requires => {
            'My::Module::Helper' => 0,
            'Your::Module' => '1.5',
        },
        recommends => {
            'Test::More' => 0,
            'Test::Pod'  => 1.18,
            'Test::Pod::Coverage' => 1
        },
        'meta-spec' => {
            version => '1.1',
            url => 'http://module-build.sourceforge.net/META-spec-new.html',
        },
    );
    my $expected = <<'YAML';
--- #YAML:1.0
name:               My-Module
version:            0.1
version_from:       lib/My/Module.pm
installdirs:        site
abstract:           A does-it-all module
license:            perl
generated_by:       myself
author:             John Doe <doe@doeland.org>
distribution_type:  module
requires:
    My::Module::Helper:  0
    Your::Module:        1.5
recommends:
    Test::More:           0
    Test::Pod:            1.18
    Test::Pod::Coverage:  1
meta-spec:
    url:      http://module-build.sourceforge.net/META-spec-new.html
    version:  1.1
YAML

    is($mm->metafile_file(@meta), $expected, "META.yml with extra 'recommends' works");
}

{
    my @meta = (
        name => 'My-Module',
        version => '0.1',
        version_from => 'lib/My/Module.pm',
        installdirs => 'site',
        abstract => 'A does-it-all module',
        license => 'perl',
        generated_by => 'myself',
        author => 'John Doe <doe@doeland.org>',
        distribution_type => 'module',
        requires => {
            'My::Module::Helper' => 0,
            'Your::Module' => '1.5',
        },
        recommends => {
            'Test::More' => 0,
            'Test::Pod'  => 1.18,
            'Test::Pod::Coverage' => 1
        },
        no_index => {
            dir => [ qw(inc) ],
            file => [ qw(TODO NOTES) ],
        },
        'meta-spec' => {
            version => '1.1',
            url => 'http://module-build.sourceforge.net/META-spec-new.html',
        },
    );
    my $expected = <<'YAML';
--- #YAML:1.0
name:               My-Module
version:            0.1
version_from:       lib/My/Module.pm
installdirs:        site
abstract:           A does-it-all module
license:            perl
generated_by:       myself
author:             John Doe <doe@doeland.org>
distribution_type:  module
requires:
    My::Module::Helper:  0
    Your::Module:        1.5
recommends:
    Test::More:           0
    Test::Pod:            1.18
    Test::Pod::Coverage:  1
no_index:
    dir:
        - inc
    file:
        - TODO
        - NOTES
meta-spec:
    url:      http://module-build.sourceforge.net/META-spec-new.html
    version:  1.1
YAML

    is($mm->metafile_file(@meta), $expected, "META.yml with extra 'no_index' works");


    # Make sure YAML.pm can ready our output
    SKIP: {
        skip "Need YAML.pm to test if it can load META.yml", 1
          unless eval { require YAML };

        my $yaml_load = YAML::Load($mm->metafile_file(@meta));
        is_deeply( $yaml_load, {@meta}, "META.yml can be read by YAML.pm" );
    }


    SKIP: {
        # Load() behaves diffrently in versions prior to 1.06
        skip "Need YAML::Tiny to test if it can load META.yml", 2
          unless eval { require YAML::Tiny } and $YAML::Tiny::VERSION >= 1.06;

        my @yaml_load = YAML::Tiny::Load($mm->metafile_file(@meta));
        is @yaml_load, 1,               "YAML::Tiny saw one document in META.yml";
        is_deeply( $yaml_load[0], {@meta}, "META.yml can be read by YAML::Tiny" );
    }
}


{
    my @meta = ( k => 'a : b', 'x : y' => 1 );
    my $expected = <<YAML;
--- #YAML:1.0
k:      a : b
x : y:  1
YAML
    # NOTE: the output is not YAML-equivalent to the input

    is($mm->metafile_file(@meta), $expected, "no quoting is done");
}

{
    my @meta = ( k => \*STDOUT );
    eval { $mm->metafile_file(@meta) };

    like($@, qr/^only nested hashes, arrays and objects are supported/,
         "we don't like but hash/array refs");
}

{
    my @meta = ( k => [ [] ] );
    eval { $mm->metafile_file(@meta) };

    like($@, qr/^only nested arrays of non-refs are supported/,
         "we also don't like but array of strings");
}

# recursive data structures: don't even think about it - endless recursion
