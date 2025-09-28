# Helper functions to test the podlators distribution.
#
# This module is an internal implementation detail of the podlators test
# suite.  It provides some supporting functions to make it easier to write
# tests.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

package Test::Podlators;

use 5.010;
use base qw(Exporter);
use strict;
use warnings;

use Encode qw(decode encode);
use Exporter;
use File::Spec;
use Test::More;

our $VERSION = '2.01';

# Export the test helper functions.
our @EXPORT_OK = qw(
    read_snippet read_test_data slurp test_snippet test_snippet_with_io
);

# The file handle used to capture STDERR while we mess with file descriptors.
my $OLD_STDERR;

# The file name used to capture standard error output.
my $SAVED_STDERR;

# Internal function to clean up the standard error output file.  Leave the
# temporary directory in place, since otherwise we race with other test
# scripts trying to create the temporary directory when running tests in
# parallel.
sub _stderr_cleanup {
    if ($SAVED_STDERR && -e $SAVED_STDERR) {
        unlink($SAVED_STDERR);
    }
    return;
}

# Remove saved standard error on exit, even if we have an abnormal exit.
END {
    _stderr_cleanup();
}

# Internal function to redirect stderr to a file.  Stores the name in
# $SAVED_STDERR.
sub _stderr_save {
    my $tmpdir = File::Spec->catdir('t', 'tmp');
    if (!-d $tmpdir) {
        mkdir($tmpdir, 0777) or BAIL_OUT("cannot create $tmpdir: $!");
    }
    my $path = File::Spec->catfile($tmpdir, "out$$.err");
    open($OLD_STDERR, '>&', STDERR) or BAIL_OUT("cannot dup STDERR: $!");
    open(STDERR, '>', $path) or BAIL_OUT("cannot redirect STDERR: $!");
    $SAVED_STDERR = $path;
    return;
}

# Internal function to restore stderr.
#
# Returns: The contents of the stderr file.
sub _stderr_restore {
    return if !$SAVED_STDERR;
    close(STDERR) or BAIL_OUT("cannot close STDERR: $!");
    open(STDERR, '>&', $OLD_STDERR) or BAIL_OUT("cannot dup STDERR: $!");
    close($OLD_STDERR) or BAIL_OUT("cannot close redirected STDERR: $!");
    my $stderr = slurp($SAVED_STDERR);
    _stderr_cleanup();
    return $stderr;
}

# Read one test snippet from the provided relative file name and return it.
# For the format, see t/data/snippets/README.md.
#
# $path     - Relative path to read test data from
#
# Returns: Reference to hash of test data with the following keys:
#            name      - Name of the test for status reporting
#            options   - Hash of options
#            input     - The input block of the test data
#            output    - The output block of the test data
#            errors    - Expected errors
#            exception - Text of exception (with file and line stripped)
sub read_snippet {
    my ($path) = @_;
    $path = File::Spec->catfile('t', 'data', 'snippets', $path);
    my %data;

    # Read the sections and store them in the %data hash.
    my ($line, $section);
    open(my $fh, '<', $path) or BAIL_OUT("cannot open $path: $!");
    while (defined($line = <$fh>)) {
        if ($line =~ m{ \A \s* \[ (\S+) \] \s* \z }xms) {
            $section = $1;
            $data{$section} = q{};
        } elsif ($section) {
            $data{$section} .= $line;
        }
    }
    close($fh) or BAIL_OUT("cannot close $path: $!");

    # Strip trailing blank lines from all sections.
    for my $section (keys %data) {
        $data{$section} =~ s{ \n\s+ \z }{\n}xms;
    }

    # Clean up the name section by removing newlines and extra space.
    if ($data{name}) {
        $data{name} =~ s{ \A \s+ }{}xms;
        $data{name} =~ s{ \s+ \z }{}xms;
        $data{name} =~ s{ \s+ }{ }xmsg;
    }

    # Turn the options section into a hash.
    if ($data{options}) {
        my @lines = split(m{ \n }xms, $data{options});
        delete $data{options};
        for my $optline (@lines) {
            next if $optline !~ m{ \S }xms;
            my ($option, $value) = split(q{ }, $optline, 2);
            if (defined($value)) {
                chomp($value);
            } else {
                $value = q{};
            }
            $data{options}{$option} = $value;
        }
    }

    # Return the results.
    return \%data;
}

# Read one set of test data from the provided file handle and return it.
# There are several different possible formats, which are specified by the
# format option.
#
# The data read from the file handle will be ignored until a line consisting
# solely of "###" is found.  Then, two or more blocks separated by "###" are
# read, ending with another line of "###".  There will always be at least an
# input and an output block, and may be more blocks based on the format
# configuration.
#
# $fh         - File handle to read the data from
# $format_ref - Reference to a hash of options describing the data
#   errors  - Set to true to read expected errors after the output section
#   options - Set to true to read a hash of options as the first data block
#
# Returns: Reference to hash of test data with the following keys:
#            input   - The input block of the test data
#            output  - The output block of the test data
#            errors  - Expected errors if errors was set in $format_ref
#            options - Hash of options if options was set in $format_ref
#          or returns undef if no more test data is found.
sub read_test_data {
    my ($fh, $format_ref) = @_;
    $format_ref ||= {};
    my %data;

    # Find the first block of test data.
    my $line;
    while (defined($line = <$fh>)) {
        last if $line eq "###\n";
    }
    if (!defined($line)) {
        return;
    }

    # If the format contains the options key, read the options into a hash.
    if ($format_ref->{options}) {
        while (defined($line = <$fh>)) {
            last if $line eq "###\n";
            my ($option, $value) = split(q{ }, $line, 2);
            if (defined($value)) {
                chomp($value);
            } else {
                $value = q{};
            }
            $data{options}{$option} = $value;
        }
    }

    # Read the input and output sections.
    my @sections = qw(input output);
    if ($format_ref->{errors}) {
        push(@sections, 'errors');
    }
    for my $key (@sections) {
        $data{$key} = q{};
        while (defined($line = <$fh>)) {
            last if $line eq "###\n";
            $data{$key} .= $line;
        }
    }
    return \%data;
}

# Slurp output data back from a file handle.  It would be nice to use
# Perl6::Slurp, but this is a core module, so we have to implement our own
# wheels.  BAIL_OUT is called on any failure to read the file.
#
# $file  - File to read
# $strip - If set to "man", strip out the Pod::Man header
#
# Returns: Contents of the file, possibly stripped
sub slurp {
    my ($file, $strip) = @_;
    open(my $fh, '<', $file) or BAIL_OUT("cannot open $file: $!");

    # If told to strip the man header, do so.
    if (defined($strip) && $strip eq 'man') {
        while (defined(my $line = <$fh>)) {
            last if $line eq ".nh\n";
        }
    }

    # Read the rest of the file and return it.
    my $data = do { local $/ = undef; <$fh> };
    close($fh) or BAIL_OUT("cannot read from $file: $!");
    return $data;
}

# Test a formatter on a particular POD snippet.  This does all the work of
# loading the snippet, creating the formatter, running it, and checking the
# results, and reports those results with Test::More.
#
# $class       - Class name of the formatter, as a string
# $snippet     - Path to the snippet file defining the test
# $options_ref - Hash of options with the following keys:
#   encoding - Expect the output to be in this non-standard encoding
sub test_snippet {
    my ($class, $snippet, $options_ref) = @_;
    my $data_ref = read_snippet($snippet);
    $options_ref //= {};

    # Determine the encoding to expect for the output portion of the snippet.
    my $encoding = $options_ref->{encoding} // 'UTF-8';

    # Create the formatter object.
    my $parser = $class->new(%{ $data_ref->{options} }, name => 'TEST');
    isa_ok($parser, $class, 'Parser object');

    # Save stderr to a temporary file and then run the parser, storing the
    # output into a Perl variable.
    my $errors = _stderr_save();
    my $got;
    $parser->output_string(\$got);
    eval { $parser->parse_string_document($data_ref->{input}) };
    my $exception = $@;
    my $stderr = _stderr_restore();

    # If we were testing Pod::Man, strip off everything prior to .nh from the
    # output so that we aren't testing the generated header.
    if ($class eq 'Pod::Man') {
        $got =~ s{ \A .* \n [.]nh \n }{}xms;
    }

    # Strip any trailing blank lines (Pod::Text likes to add them).
    $got =~ s{ \n\s+ \z }{\n}xms;

    # Check the output, errors, and any exception.
    is($got, $data_ref->{output}, "$data_ref->{name}: output");
    if ($data_ref->{errors} || $stderr) {
        is($stderr, $data_ref->{errors} || q{}, "$data_ref->{name}: errors");
    }
    if ($data_ref->{exception} || $exception) {
        if ($exception) {
            $exception =~ s{ [ ] at [ ] .* }{\n}xms;
        }
        is($exception, $data_ref->{exception}, "$data_ref->{name}: exception");
    }
    return;
}

# Helper function to check the preamble of Pod::Man output.
#
# $name     - Name of the test
# $fh       - File handle with results
# $encoding - Expected encoding
#
# Returns: True if the preamble contains accent definitions
sub _check_man_preamble {
    my ($name, $fh, $encoding) = @_;
    $encoding = lc($encoding);

    # Check the encoding line.
    my $line = <$fh>;
    if ($encoding eq 'ascii') {
        unlike(
            $line, qr{ mode: [ ] troff }xms,
            "$name: no preconv coding line",
        );
    } else {
        is(
            $line,
            ".\\\" -*- mode: troff; coding: $encoding -*-\n",
            "$name: preconv coding line",
        );
    }

    # Consume the rest of the preamble and check for accent definitions.
    my $saw_accents;
    while (defined($line = <$fh>)) {
        $line = decode($encoding, $line);
        if ($line =~ m{ Accent [ ] mark [ ] definitions }xms) {
            $saw_accents = 1;
        }
        last if $line =~ m{ \A [.]nh }xms;
    }

    return $saw_accents;
}

# Test a formatter with I/O streams on a particular POD snippet.  This does
# all the work of loading the snippet, creating the formatter, running it, and
# checking the results, and reports those results with Test::More.  It's
# similar to test_snippet, but uses input and output temporary files instead
# to test encoding layers and also checks the Pod::Man accent output.
#
# $class       - Class name of the formatter, as a string
# $snippet     - Path to the snippet file defining the test
# $options_ref - Hash of options with the following keys:
#   encoding    - Expect the snippet to be in this non-standard encoding
#   perlio_utf8 - Set to 1 to set PerlIO UTF-8 encoding on the output file
#   perlio_iso  - Set to 1 to set PerlIO ISO 8859-1 encoding on the output file
#   output      - Expect the output to be in this non-standard encoding
sub test_snippet_with_io {
    my ($class, $snippet, $options_ref) = @_;
    $options_ref //= {};
    my $data_ref = read_snippet($snippet);

    # Determine the encoding to expect for the output portion of the snippet.
    my $encoding = $options_ref->{encoding} // 'UTF-8';

    # Determine the encoding to expect for the actual output.
    my $outencoding = $options_ref->{output} // 'UTF-8';

    # Additional test output based on whether we're using PerlIO.
    my $perlio = q{};
    if ($options_ref->{perlio_utf8} || $options_ref->{perlio_iso}) {
        $perlio = ' (PerlIO)';
    }

    # Create the formatter object.
    my $parser = $class->new(%{ $data_ref->{options} }, name => 'TEST');
    isa_ok($parser, $class, 'Parser object');

    # Write the input POD to a temporary file prefaced by the encoding
    # directive.
    my $tmpdir = File::Spec->catdir('t', 'tmp');
    if (!-d $tmpdir) {
        mkdir($tmpdir, 0777) or BAIL_OUT("cannot create $tmpdir: $!");
    }
    my $input_file = File::Spec->catfile('t', 'tmp', "tmp$$.pod");
    open(my $input, '>', $input_file)
      or BAIL_OUT("cannot create $input_file: $!");
    print {$input} $data_ref->{input}
      or BAIL_OUT("cannot write to $input_file: $!");
    close($input) or BAIL_OUT("cannot flush output to $input_file: $!");

    # Create an output file and parse from the input file to the output file.
    my $output_file = File::Spec->catfile('t', 'tmp', "out$$.tmp");
    open(my $output, '>', $output_file)
      or BAIL_OUT("cannot create $output_file: $!");
    if ($options_ref->{perlio_utf8}) {
        ## no critic (BuiltinFunctions::ProhibitStringyEval)
        ## no critic (ValuesAndExpressions::RequireInterpolationOfMetachars)
        eval 'binmode($output, ":encoding(utf-8)")';
        ## use critic
    } elsif ($options_ref->{perlio_iso}) {
        ## no critic (BuiltinFunctions::ProhibitStringyEval)
        ## no critic (ValuesAndExpressions::RequireInterpolationOfMetachars)
        eval 'binmode($output, ":encoding(iso-8859-1)")';
        ## use critic
    }

    # Parse the input file into the output file.
    $parser->parse_from_file($input_file, $output);
    close($output) or BAIL_OUT("cannot flush output to $output_file: $!");

    # Read back in the results.  For Pod::Man, also check the coding line, and
    # ensure that we didn't output the accent definitions if we wrote UTF-8
    # output.
    open(my $results, '<', $output_file)
      or BAIL_OUT("cannot open $output_file: $!");
    my $saw_accents;
    if ($class eq 'Pod::Man') {
        my $name = $data_ref->{name};
        $saw_accents = _check_man_preamble($name, $results, $outencoding);
    }
    my $saw = do { local $/ = undef; <$results> };
    $saw = decode($outencoding, $saw);
    $saw =~ s{ \n\s+ \z }{\n}xms;
    close($results) or BAIL_OUT("cannot close output file: $!");

    # Clean up.
    unlink($input_file, $output_file);

    # Check the accent definitions and the output.
    if ($class eq 'Pod::Man') {
        is(
            $saw_accents,
            ($data_ref->{options}{encoding} || q{}) eq 'roff' ? 1 : undef,
            "$data_ref->{name}: accent definitions$perlio",
        );
    }
    is(
        $saw,
        decode($encoding, $data_ref->{output}),
        "$data_ref->{name}: output$perlio",
    );
    return;
}

1;
__END__

=for stopwords
Allbery podlators PerlIO UTF-8 formatter FH whitespace

=head1 NAME

Test::Podlators - Helper functions for podlators tests

=head1 SYNOPSIS

    use Test::Podlators qw(read_test_data);

    # Read the next block of test data, including options.
    my $data = read_test_data(\*DATA, { options => 1 });

=head1 DESCRIPTION

This module collects various utility functions that are useful for writing
test cases for the podlators distribution.  It is not intended to be, and
probably isn't, useful outside of the test suite for that module.

=head1 FUNCTIONS

None of these functions are imported by default.  The ones used by a script
should be explicitly imported.

=over 4

=item read_snippet(PATH)

Read one test snippet from the provided relative file name and return it.  The
path should be relative to F<t/data/snippets>.  For the format, see
F<t/data/snippets/README>.

The result will be a hash with the following keys:

=over 4

=item name

The name of the test, for reporting purposes.

=item options

A hash of any options to values, if any options were specified.

=item input

Input POD to try formatting.

=item output

The expected output.

=item errors

Expected errors from the POD formatter.

=item exception

An expected exception from the POD formatter, with the file and line
information stripped from the end of the exception.

=back

=item read_test_data(FH, FORMAT)

Reads a block of test data from FH, looking for test information according to
the description provided in FORMAT.  All data prior to the first line
consisting of only C<###> will be ignored.  Then, the test data must consist
of two or more blocks separated by C<###> and ending in a final C<###> line.

FORMAT is optional, in which case the block of test data should be just input
text and output text.  If provided, it should be a reference to a hash with
one or more of the following keys:

=over 4

=item options

If set, the first block of data in the test description is a set of options in
the form of a key, whitespace, and a value, one per line.  The value may be
missing, in which case the value associated with the key is the empty string.

=back

The return value is a hash with at least some of the following keys:

=over 4

=item input

The input data for the test.  This is always present.

=item options

If C<options> is set in the FORMAT argument, this is the hash of keys and
values in the options section of the test data.

=item output

The output data for the test.  This is always present.

=back

=item slurp(FILE[, STRIP])

Read the contents of FILE and return it as a string.  If STRIP is set to
C<man>, strip off any Pod::Man header from the file before returning it.

=item test_snippet(CLASS, SNIPPET[, OPTIONS])

Test a formatter on a particular POD snippet.  This does all the work of
loading the snippet, creating the formatter by instantiating CLASS, running
it, and checking the results.  Results are reported with Test::More.

OPTIONS, if present, is a reference to a hash of options.  Currently, only
one key is supported: C<encoding>, which, if set, specifies the encoding of
the output portion of the snippet.

=item test_snippet_with_io(CLASS, SNIPPET[, OPTIONS])

The same as test_snippet(), except, rather than parsing the input into a
string buffer, this function uses real, temporary input and output files.
This can be used to test I/O layer handling and proper encoding.  It also
does additional tests for the preamble to the *roff output.

OPTIONS, if present, is a reference to a hash of options chosen from the
following:

=over 4

=item encoding

The encoding to expect from the snippet file.  Default if not specified is
UTF-8.

=item output

The encoding to expect from the output.  Default if not specified is UTF-8.

=item perlio_iso

If set to true, set a PerlIO ISO-8859-1 encoding layer on the output file
before writing to it.

=item perlio_utf8

If set to true, set a PerlIO UTF-8 encoding layer on the output file before
writing to it.

=back

=back

=head1 AUTHOR

Russ Allbery <rra@cpan.org>

=head1 COPYRIGHT AND LICENSE

Copyright 2015-2016, 2018-2020, 2022 Russ Allbery <rra@cpan.org>

This program is free software; you may redistribute it and/or modify it
under the same terms as Perl itself.

=cut

# Local Variables:
# copyright-at-end-flag: t
# End:
