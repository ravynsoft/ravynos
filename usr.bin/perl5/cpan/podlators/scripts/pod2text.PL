#!/usr/bin/perl
#
# Special wrapper script to generate the actual pod2man script.  This is
# required for proper start-up code on non-UNIX platforms, and is used inside
# Perl core.

use 5.008;
use strict;
use warnings;

use Config qw(%Config);
use Cwd qw(cwd);
use File::Basename qw(basename dirname);

# List explicitly here the variables you want Configure to generate.
# Metaconfig only looks for shell variables, so you have to mention them as if
# they were shell variables, not %Config entries.  Thus you write
#  $startperl
# to ensure Configure will look for $Config{startperl}.

# This forces PL files to create target in same directory as PL file.
# This is so that make depend always knows where to find PL derivatives.
chdir(dirname($0)) or die "Cannot change directories: $!\n";
my $file = basename($0, '.PL');
if ($^O eq 'VMS') {
    $file .= '.com';
}

# Create the generated script.
## no critic (InputOutput::RequireBriefOpen)
## no critic (InputOutput::RequireCheckedSyscalls)
open(my $out, '>', $file) or die "Cannot create $file: $!\n";
print "Extracting $file (with variable substitutions)\n";
## use critic

# In this section, Perl variables will be expanded during extraction.  You can
# use $Config{...} to use Configure variables.
print {$out} <<"PREAMBLE" or die "Cannot write to $file: $!\n";
$Config{startperl}
    eval 'exec $Config{perlpath} -S \$0 \${1+"\$@"}'
        if 0; # ^ Run only under a shell
PREAMBLE

# In the following, Perl variables are not expanded during extraction.
print {$out} <<'SCRIPT_BODY' or die "Cannot write to $file: $!\n";

# Convert POD data to formatted ASCII text.
#
# The driver script for Pod::Text, Pod::Text::Termcap, and Pod::Text::Color,
# invoked by perldoc -t among other things.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.006;
use strict;
use warnings;

use Getopt::Long qw(GetOptions);
use Pod::Text ();
use Pod::Usage qw(pod2usage);

# Clean up $0 for error reporting.
$0 =~ s%.*/%%;

# Take an initial pass through our options, looking for one of the form
# -<number>.  We turn that into -w <number> for compatibility with the
# original pod2text script.
for (my $i = 0; $i < @ARGV; $i++) {
    last if $ARGV[$i] =~ /^--$/;
    if ($ARGV[$i] =~ /^-(\d+)$/) {
        splice (@ARGV, $i++, 1, '-w', $1);
    }
}

# Insert -- into @ARGV before any single dash argument to hide it from
# Getopt::Long; we want to interpret it as meaning stdin (which Pod::Simple
# does correctly).
my $stdin;
@ARGV = map { $_ eq '-' && !$stdin++ ? ('--', $_) : $_ } @ARGV;

# Parse our options.  Use the same names as Pod::Text for simplicity.
my %options;
Getopt::Long::config ('bundling');
GetOptions (\%options, 'alt|a', 'code', 'color|c', 'encoding|e=s', 'errors=s',
            'guesswork=s', 'help|h', 'indent|i=i', 'loose|l',
            'margin|left-margin|m=i', 'nourls', 'overstrike|o', 'quotes|q=s',
            'sentence|s', 'stderr', 'termcap|t', 'utf8|u', 'width|w=i')
    or exit 1;
pod2usage (1) if $options{help};

# Figure out what formatter we're going to use.  -c overrides -t.
my $formatter = 'Pod::Text';
if ($options{color}) {
    $formatter = 'Pod::Text::Color';
    eval { require Term::ANSIColor };
    if ($@) { die "-c (--color) requires Term::ANSIColor be installed\n" }
    require Pod::Text::Color;
} elsif ($options{termcap}) {
    $formatter = 'Pod::Text::Termcap';
    require Pod::Text::Termcap;
} elsif ($options{overstrike}) {
    $formatter = 'Pod::Text::Overstrike';
    require Pod::Text::Overstrike;
}
delete @options{'color', 'termcap', 'overstrike'};

# If neither stderr nor errors is set, default to errors = die.
if (!defined $options{stderr} && !defined $options{errors}) {
    $options{errors} = 'die';
}

# Initialize and run the formatter.
my $parser = $formatter->new (%options);
my $status = 0;
do {
    my ($input, $output) = splice (@ARGV, 0, 2);
    $parser->parse_from_file ($input, $output);
    if ($parser->{CONTENTLESS}) {
        $status = 1;
        if (defined $input) {
            warn "$0: unable to format $input\n";
        } else {
            warn "$0: unable to format standard input\n";
        }
        if (defined ($output) and $output ne '-') {
            unlink $output unless (-s $output);
        }
    }
} while (@ARGV);
exit $status;

__END__

=for stopwords
-aclostu --alt --stderr Allbery --overstrike overstrike --termcap --utf8
UTF-8 subclasses --nourls EBCDIC unrepresentable

=head1 NAME

pod2text - Convert POD data to formatted ASCII text

=head1 SYNOPSIS

pod2text [B<-aclostu>] [B<--code>] S<[B<-e> I<encoding>]>
    [B<--errors>=I<style>] [B<--guesswork>=I<rule>[,I<rule>...]]
    S<[B<-i> I<indent>]> S<[B<-q> I<quotes>]>
    [B<--nourls>] [B<--stderr>] S<[B<-w> I<width>]> [I<input> [I<output> ...]]

pod2text B<-h>

=head1 DESCRIPTION

B<pod2text> is a wrapper script around the L<Pod::Text> and its subclasses.
It uses them to generate formatted text from POD source.  It can optionally
use either termcap sequences or ANSI color escape sequences to format the
text.

I<input> is the file to read for POD source (the POD can be embedded in code).
If I<input> isn't given, it defaults to C<STDIN>.  I<output>, if given, is the
file to which to write the formatted output.  If I<output> isn't given, the
formatted output is written to C<STDOUT>.  Several POD files can be processed
in the same B<pod2text> invocation (saving module load and compile times) by
providing multiple pairs of I<input> and I<output> files on the command line.

By default, the output encoding is the same as the encoding of the input file,
or UTF-8 if that encoding is not set (except on EBCDIC systems).  See the
B<-e> option to explicitly set the output encoding and L<Pod::Text/Encoding>
for more discussion.

=head1 OPTIONS

Each option is annotated with the version of podlators in which that option
was added with its current meaning.

=over 4

=item B<-a>, B<--alt>

[1.00] Use an alternate output format that, among other things, uses a
different heading style and marks C<=item> entries with a colon in the left
margin.

=item B<--code>

[1.11] Include any non-POD text from the input file in the output as well.
Useful for viewing code documented with POD blocks with the POD rendered and
the code left intact.

=item B<-c>, B<--color>

[1.00] Format the output with ANSI color escape sequences.  Using this option
requires that Term::ANSIColor be installed on your system.

=item B<-e> I<encoding>, B<--encoding>=I<encoding>

[5.00] Specifies the encoding of the output.  I<encoding> must be an encoding
recognized by the L<Encode> module (see L<Encode::Supported>).  If the output
contains characters that cannot be represented in this encoding, that is an
error that will be reported as configured by the C<errors> option.  If error
handling is other than C<die>, the unrepresentable character will be replaced
with the Encode substitution character (normally C<?>).

WARNING: The input encoding of the POD source is independent from the output
encoding, and setting this option does not affect the interpretation of the
POD input.  Unless your POD source is US-ASCII, its encoding should be
declared with the C<=encoding> command in the source, as near to the top of
the file as possible.  If this is not done, Pod::Simple will will attempt to
guess the encoding and may be successful if it's Latin-1 or UTF-8, but it will
produce warnings.  See L<perlpod(1)> for more information.

=item B<--errors>=I<style>

[2.5.0] Set the error handling style.  C<die> says to throw an exception on
any POD formatting error.  C<stderr> says to report errors on standard error,
but not to throw an exception.  C<pod> says to include a POD ERRORS section in
the resulting documentation summarizing the errors.  C<none> ignores POD
errors entirely, as much as possible.

The default is C<die>.

=item B<--guesswork>=I<rule>[,I<rule>...]

[5.01] By default, B<pod2text> applies some default formatting rules based on
guesswork and regular expressions that are intended to make writing Perl
documentation easier and require less explicit markup.  These rules may not
always be appropriate, particularly for documentation that isn't about Perl.
This option allows turning all or some of it off.

The special rule C<all> enables all guesswork.  This is also the default for
backward compatibility reasons.  The special rule C<none> disables all
guesswork.  Otherwise, the value of this option should be a comma-separated
list of one or more of the following keywords:

=over 4

=item quoting

If no guesswork is enabled, any text enclosed in CZ<><> is surrounded by
double quotes in nroff (terminal) output unless the contents are already
quoted.  When this guesswork is enabled, quote marks will also be suppressed
for Perl variables, function names, function calls, numbers, and hex
constants.

=back

Any unknown guesswork name is silently ignored (for potential future
compatibility), so be careful about spelling.

=item B<-i> I<indent>, B<--indent=>I<indent>

[1.00] Set the number of spaces to indent regular text, and the default
indentation for C<=over> blocks.  Defaults to 4 spaces if this option isn't
given.

=item B<-h>, B<--help>

[1.00] Print out usage information and exit.

=item B<-l>, B<--loose>

[1.00] Print a blank line after a C<=head1> heading.  Normally, no blank line
is printed after C<=head1>, although one is still printed after C<=head2>,
because this is the expected formatting for manual pages; if you're formatting
arbitrary text documents, using this option is recommended.

=item B<-m> I<width>, B<--left-margin>=I<width>, B<--margin>=I<width>

[1.24] The width of the left margin in spaces.  Defaults to 0.  This is the
margin for all text, including headings, not the amount by which regular text
is indented; for the latter, see B<-i> option.

=item B<--nourls>

[2.5.0] Normally, LZ<><> formatting codes with a URL but anchor text are
formatted to show both the anchor text and the URL.  In other words:

    L<foo|http://example.com/>

is formatted as:

    foo <http://example.com/>

This flag, if given, suppresses the URL when anchor text is given, so this
example would be formatted as just C<foo>.  This can produce less cluttered
output in cases where the URLs are not particularly important.

=item B<-o>, B<--overstrike>

[1.06] Format the output with overstrike printing.  Bold text is rendered as
character, backspace, character.  Italics and file names are rendered as
underscore, backspace, character.  Many pagers, such as B<less>, know how to
convert this to bold or underlined text.

=item B<-q> I<quotes>, B<--quotes>=I<quotes>

[4.00] Sets the quote marks used to surround CE<lt>> text to I<quotes>.  If
I<quotes> is a single character, it is used as both the left and right quote.
Otherwise, it is split in half, and the first half of the string is used as
the left quote and the second is used as the right quote.

I<quotes> may also be set to the special value C<none>, in which case no quote
marks are added around CE<lt>> text.

=item B<-s>, B<--sentence>

[1.00] Assume each sentence ends with two spaces and try to preserve that
spacing.  Without this option, all consecutive whitespace in non-verbatim
paragraphs is compressed into a single space.

=item B<--stderr>

[2.1.3] By default, B<pod2text> dies if any errors are detected in the POD
input.  If B<--stderr> is given and no B<--errors> flag is present, errors are
sent to standard error, but B<pod2text> does not abort.  This is equivalent to
C<--errors=stderr> and is supported for backward compatibility.

=item B<-t>, B<--termcap>

[1.00] Try to determine the width of the screen and the bold and underline
sequences for the terminal from termcap, and use that information in
formatting the output.  Output will be wrapped at two columns less than the
width of your terminal device.  Using this option requires that your system
have a termcap file somewhere where Term::Cap can find it and requires that
your system support termios.  With this option, the output of B<pod2text> will
contain terminal control sequences for your current terminal type.

=item B<-u>, B<--utf8>

[2.2.0] Set the output encoding to UTF-8.  This is equivalent to
C<--encoding=UTF-8> and is supported for backward compatibility.

=item B<-w>, B<--width=>I<width>, B<->I<width>

[1.00] The column at which to wrap text on the right-hand side.  Defaults to
76, unless B<-t> is given, in which case it's two columns less than the width
of your terminal device.

=back

=head1 EXIT STATUS

As long as all documents processed result in some output, even if that output
includes errata (a C<POD ERRORS> section generated with C<--errors=pod>),
B<pod2text> will exit with status 0.  If any of the documents being processed
do not result in an output document, B<pod2text> will exit with status 1.  If
there are syntax errors in a POD document being processed and the error
handling style is set to the default of C<die>, B<pod2text> will abort
immediately with exit status 255.

=head1 DIAGNOSTICS

If B<pod2text> fails with errors, see L<Pod::Text> and L<Pod::Simple> for
information about what those errors might mean.  Internally, it can also
produce the following diagnostics:

=over 4

=item -c (--color) requires Term::ANSIColor be installed

(F) B<-c> or B<--color> were given, but Term::ANSIColor could not be loaded.

=item Unknown option: %s

(F) An unknown command line option was given.

=back

In addition, other L<Getopt::Long> error messages may result from invalid
command-line options.

=head1 ENVIRONMENT

=over 4

=item COLUMNS

If B<-t> is given, B<pod2text> will take the current width of your screen from
this environment variable, if available.  It overrides terminal width
information in TERMCAP.

=item TERMCAP

If B<-t> is given, B<pod2text> will use the contents of this environment
variable if available to determine the correct formatting sequences for your
current terminal device.

=back

=head1 AUTHOR

Russ Allbery <rra@cpan.org>.

=head1 COPYRIGHT AND LICENSE

Copyright 1999-2001, 2004, 2006, 2008, 2010, 2012-2019, 2022 Russ Allbery
<rra@cpan.org>

This program is free software; you may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Encode::Supported>, L<Pod::Text>, L<Pod::Text::Color>,
L<Pod::Text::Overstrike>, L<Pod::Text::Termcap>, L<Pod::Simple>, L<perlpod(1)>

The current version of this script is always available from its web site at
L<https://www.eyrie.org/~eagle/software/podlators/>.  It is also part of the
Perl core distribution as of 5.6.0.

=cut
SCRIPT_BODY

# Finish the generation of the script.
close($out) or die "Cannot close $file: $!\n";
chmod(0755, $file) or die "Cannot reset permissions for $file: $!\n";
if ($Config{'eunicefix'} ne ':') {
    exec("$Config{'eunicefix'} $file");
}

# Local Variables:
# copyright-at-end-flag: t
# End:
