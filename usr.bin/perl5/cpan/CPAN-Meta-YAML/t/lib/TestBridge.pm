package TestBridge;

use strict;
use warnings;
use lib 't/lib';
use Test::More 0.88;
use SubtestCompat;
use TestUtils;
use TestML::Tiny;

BEGIN {
    $|  = 1;
    binmode(Test::More->builder->$_, ":utf8")
        for qw/output failure_output todo_output/;
}

use CPAN::Meta::YAML;

use Exporter   ();
our @ISA    = qw{ Exporter };
our @EXPORT = qw{
    run_all_testml_files
    run_testml_file
    test_yaml_roundtrip
    test_perl_to_yaml
    test_dump_error
    test_load_error
    test_load_warning
    test_yaml_json
    test_code_point
    error_like
    cmp_deeply
    _testml_has_points
};

# regular expressions for checking error messages; incomplete, but more
# can be added as more error messages get test coverage
my %ERROR = (
    E_CIRCULAR => qr{\QCPAN::Meta::YAML does not support circular references},
    E_FEATURE  => qr{\QCPAN::Meta::YAML does not support a feature},
    E_PLAIN    => qr{\QCPAN::Meta::YAML found illegal characters in plain scalar},
    E_CLASSIFY => qr{\QCPAN::Meta::YAML failed to classify the line},
);

my %WARN = (
    E_DUPKEY   => qr{\QCPAN::Meta::YAML found a duplicate key},
);

# use XXX -with => 'YAML::XS';

#--------------------------------------------------------------------------#
# run_all_testml_files
#
# Iterate over all .tml files in a directory using a particular test bridge
# code # reference.  Each file is wrapped in a subtest.
#--------------------------------------------------------------------------#

sub run_all_testml_files {
    my ($label, $dir, $bridge, @args) = @_;

    my $code = sub {
        my ($file, $blocks) = @_;
        subtest "$label: $file" => sub {
            $bridge->($_, @args) for @$blocks;
        };
    };

    my @files = find_tml_files($dir);

    run_testml_file($_, $code) for sort @files;
}

sub run_testml_file {
    my ($file, $code) = @_;

    my $blocks = TestML::Tiny->new(
        testml => $file,
        version => '0.1.0',
    )->{function}{data};

    $code->($file, $blocks);
}

# retrieves all the keys in @point from the $block hash, returning them in
# order, along with $block->{Label}.
# returns false if any keys cannot be found
sub _testml_has_points {
    my ($block, @points) = @_;
    my @values;
    for my $point (@points) {
        defined $block->{$point} or return;
        push @values, $block->{$point};
    }
    push @values, $block->{Label};
    return @values;
}

#--------------------------------------------------------------------------#
# test_yaml_roundtrip
#
# two blocks: perl, yaml
#
# Tests that a YAML string loads to the expected perl data.  Also, tests
# roundtripping from perl->YAML->perl.
#
# We can't compare the YAML for roundtripping because CPAN::Meta::YAML doesn't
# preserve order and comments.  Therefore, all we can test is that given input
# YAML we can produce output YAML that produces the same Perl data as the
# input.
#
# The perl must be an array reference of data to serialize:
#
# [ $thing1, $thing2, ... ]
#
# However, if a test point called 'serializes' exists, the output YAML is
# expected to match the input YAML and will be checked for equality.
#--------------------------------------------------------------------------#

sub test_yaml_roundtrip {
    my ($block) = @_;

    my ($yaml, $perl, $label) =
      _testml_has_points($block, qw(yaml perl)) or return;

    my %options = ();
    for (qw(serializes)) {
        if (defined($block->{$_})) {
            $options{$_} = 1;
        }
    }

    my $expected = eval $perl; die $@ if $@;
    bless $expected, 'CPAN::Meta::YAML';

    subtest $label, sub {
        # Does the string parse to the structure
        my $yaml_copy = $yaml;
        my $got       = eval { CPAN::Meta::YAML->read_string( $yaml_copy ); };
        is( $@, '', "CPAN::Meta::YAML parses without error" );
        is( $yaml_copy, $yaml, "CPAN::Meta::YAML does not modify the input string" );
        SKIP: {
            skip( "Shortcutting after failure", 2 ) if $@;
            isa_ok( $got, 'CPAN::Meta::YAML' );
            cmp_deeply( $got, $expected, "CPAN::Meta::YAML parses correctly" )
                or diag "ERROR: $CPAN::Meta::YAML::errstr\n\nYAML:$yaml";
        }

        # Does the structure serialize to the string.
        # We can't test this by direct comparison, because any
        # whitespace or comments would be lost.
        # So instead we parse back in.
        my $output = eval { $expected->write_string };
        is( $@, '', "CPAN::Meta::YAML serializes without error" );
        SKIP: {
            skip( "Shortcutting after failure", 5 ) if $@;
            ok(
                !!(defined $output and ! ref $output),
                "CPAN::Meta::YAML serializes to scalar",
            );
            my $roundtrip = eval { CPAN::Meta::YAML->read_string( $output ) };
            is( $@, '', "CPAN::Meta::YAML round-trips without error" );
            skip( "Shortcutting after failure", 2 ) if $@;
            isa_ok( $roundtrip, 'CPAN::Meta::YAML' );
            cmp_deeply( $roundtrip, $expected, "CPAN::Meta::YAML round-trips correctly" );

            # Testing the serialization
            skip( "Shortcutting perfect serialization tests", 1 ) unless $options{serializes};
            is( $output, $yaml, 'Serializes ok' );
        }

    };
}

#--------------------------------------------------------------------------#
# test_perl_to_yaml
#
# two blocks: perl, yaml
#
# Tests that perl references serialize correctly to a specific YAML output
#
# The perl must be an array reference of data to serialize:
#
# [ $thing1, $thing2, ... ]
#--------------------------------------------------------------------------#

sub test_perl_to_yaml {
    my ($block) = @_;

    my ($perl, $yaml, $label) =
      _testml_has_points($block, qw(perl yaml)) or return;

    my $input = eval "no strict; $perl"; die $@ if $@;

    subtest $label, sub {
        my $result = eval { CPAN::Meta::YAML->new( @$input )->write_string };
        is( $@, '', "write_string lives" );
        is( $result, $yaml, "dumped YAML correct" );
    };
}

#--------------------------------------------------------------------------#
# test_dump_error
#
# two blocks: perl, error
#
# Tests that perl references result in an error when dumped
#
# The perl must be an array reference of data to serialize:
#
# [ $thing1, $thing2, ... ]
#
# The error must be a key in the %ERROR hash in this file
#--------------------------------------------------------------------------#

sub test_dump_error {
    my ($block) = @_;

    my ($perl, $error, $label) =
      _testml_has_points($block, qw(perl error)) or return;

    my $input = eval "no strict; $perl"; die $@ if $@;
    chomp $error;
    my $expected = $ERROR{$error};

    subtest $label, sub {
        my $result = eval { CPAN::Meta::YAML->new( @$input )->write_string };
        ok( !$result, "returned false" );
        error_like( $expected, "Got expected error" );
    };
}

#--------------------------------------------------------------------------#
# test_load_error
#
# two blocks: yaml, error
#
# Tests that a YAML string results in an error when loaded
#
# The error must be a key in the %ERROR hash in this file
#--------------------------------------------------------------------------#

sub test_load_error {
    my ($block) = @_;

    my ($yaml, $error, $label) =
      _testml_has_points($block, qw(yaml error)) or return;

    chomp $error;
    my $expected = $ERROR{$error};

    subtest $label, sub {
        my $result = eval { CPAN::Meta::YAML->read_string( $yaml ) };
        is( $result, undef, 'read_string returns undef' );
        error_like( $expected, "Got expected error" )
            or diag "YAML:\n$yaml";
    };
}

#--------------------------------------------------------------------------#
# test_load_warning
#
# two blocks: yaml, warning
#
# Tests that a YAML string results in warning when loaded
#
# The warning must be a key in the %WARN hash in this file
#--------------------------------------------------------------------------#
sub test_load_warning {
    my ($block) = @_;

    my ($yaml, $warning, $label) =
      _testml_has_points($block, qw(yaml warning)) or return;

    chomp $warning;
    my $expected = $WARN{$warning};

    subtest $label, sub {
        # this is not in a sub like warning_like because of the danger of
        # matching the regex parameter against something earlier in the stack
        my @warnings;
        local $SIG{__WARN__} = sub { push @warnings, shift; };

        my $result = eval { CPAN::Meta::YAML->read_string( $yaml ) };

        is(scalar(@warnings), 1, 'got exactly one warning');
        like(
            $warnings[0],
            $expected,
            'Got expected warning',
        ) or diag "YAML:\n$yaml\n", 'warning: ', explain(\@warnings);
    };
}

#--------------------------------------------------------------------------#
# test_yaml_json
#
# two blocks: yaml, json
#
# Tests that a YAML string can be loaded to Perl and dumped to JSON and
# match an expected JSON output.  The expected JSON is loaded and dumped
# to ensure similar JSON dump options.
#--------------------------------------------------------------------------#

sub test_yaml_json {
    my ($block, $json_lib) = @_;
    $json_lib ||= do { require JSON::PP; 'JSON::PP' };

    my ($yaml, $json, $label) =
      _testml_has_points($block, qw(yaml json)) or return;

    subtest "$label", sub {
        # test YAML Load
        my $object = eval {
            CPAN::Meta::YAML::Load($yaml);
        };
        my $err = $@;
        ok !$err, "YAML loads";
        return if $err;

        # test YAML->Perl->JSON
        # N.B. round-trip JSON to decode any \uNNNN escapes and get to
        # characters
        my $want = $json_lib->new->encode(
            $json_lib->new->decode($json)
        );
        my $got = $json_lib->new->encode($object);
        is $got, $want, "Load is accurate";
    };
}

#--------------------------------------------------------------------------#
# test_code_point
#
# two blocks: code, yaml
#
# Tests that a Unicode codepoint is correctly dumped to YAML as both
# key and value.
#
# The code test point must be a non-negative integer
#
# The yaml code point is the expected output of { $key => $value } where
# both key and value are the character represented by the codepoint.
#--------------------------------------------------------------------------#

sub test_code_point {
    my ($block) = @_;

    my ($code, $yaml, $label) =
        _testml_has_points($block, qw(code yaml)) or return;

    subtest "$label - Unicode map key/value test" => sub {
        my $data = { chr($code) => chr($code) };
        my $dump = CPAN::Meta::YAML::Dump($data);
        $dump =~ s/^---\n//;
        is $dump, $yaml, "Dump key and value of code point char $code";

        my $yny = CPAN::Meta::YAML::Dump(CPAN::Meta::YAML::Load($yaml));
        $yny =~ s/^---\n//;
        is $yny, $yaml, "YAML for code point $code YNY roundtrips";

        my $nyn = CPAN::Meta::YAML::Load(CPAN::Meta::YAML::Dump($data));
        cmp_deeply( $nyn, $data, "YAML for code point $code NYN roundtrips" );
    }
}

#--------------------------------------------------------------------------#
# error_like
#
# Test CPAN::Meta::YAML->errstr against a regular expression and clear the
# errstr afterwards
#--------------------------------------------------------------------------#

sub error_like {
    my ($regex, $label) = @_;
    $label = "Got expected error" unless defined $label;
    local $Test::Builder::Level = $Test::Builder::Level + 1;
    my $ok = like( $@, $regex, $label );
    return $ok;
}

#--------------------------------------------------------------------------#
# cmp_deeply
#
# is_deeply with some better diagnostics
#--------------------------------------------------------------------------#
sub cmp_deeply {
    my ($got, $want, $label) = @_;
    local $Test::Builder::Level = $Test::Builder::Level + 1;
    is_deeply( $got, $want, $label )
        or diag "GOT:\n", explain($got), "\nWANTED:\n", explain($want);
}

1;
