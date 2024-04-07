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

# Convert POD data to formatted *roff input.
#
# The driver script for Pod::Man.
#
# SPDX-License-Identifier: GPL-1.0-or-later OR Artistic-1.0-Perl

use 5.006;
use strict;
use warnings;

use Getopt::Long qw(GetOptions);
use Pod::Man ();
use Pod::Usage qw(pod2usage);

use strict;

# Clean up $0 for error reporting.
$0 =~ s%.*/%%;

# Insert -- into @ARGV before any single dash argument to hide it from
# Getopt::Long; we want to interpret it as meaning stdin.
my $stdin;
@ARGV = map { $_ eq '-' && !$stdin++ ? ('--', $_) : $_ } @ARGV;

# Parse our options, trying to retain backward compatibility with pod2man but
# allowing short forms as well.  --lax is currently ignored.
my %options;
Getopt::Long::config ('bundling_override');
GetOptions (\%options, 'center|c=s', 'date|d=s', 'encoding|e=s', 'errors=s',
            'fixed=s', 'fixedbold=s', 'fixeditalic=s', 'fixedbolditalic=s',
            'guesswork=s', 'help|h', 'lax|l', 'language=s', 'lquote=s',
            'name|n=s', 'nourls', 'official|o', 'quotes|q=s', 'release|r=s',
            'rquote=s', 'section|s=s', 'stderr', 'verbose|v', 'utf8|u')
    or exit 1;
pod2usage (0) if $options{help};

# Official sets --center, but don't override things explicitly set.
if ($options{official} && !defined $options{center}) {
    $options{center} = 'Perl Programmers Reference Guide';
}

# Verbose is only our flag, not a Pod::Man flag.
my $verbose = $options{verbose};
delete $options{verbose};

# This isn't a valid Pod::Man option and is only accepted for backward
# compatibility.
delete $options{lax};

# If neither stderr nor errors is set, default to errors = die.
if (!defined $options{stderr} && !defined $options{errors}) {
    $options{errors} = 'die';
}

# Initialize and run the formatter, pulling a pair of input and output off at
# a time.  For each file, we check whether the document was completely empty
# and, if so, will remove the created file and exit with a non-zero exit
# status.
my $parser = Pod::Man->new (%options);
my $status = 0;
my @files;
do {
    @files = splice (@ARGV, 0, 2);
    print "  $files[1]\n" if $verbose;
    $parser->parse_from_file (@files);
    if ($parser->{CONTENTLESS}) {
        $status = 1;
        if (defined $files[0]) {
            warn "$0: unable to format $files[0]\n";
        } else {
            warn "$0: unable to format standard input\n";
        }
        if (defined ($files[1]) and $files[1] ne '-') {
            unlink $files[1] unless (-s $files[1]);
        }
    }
} while (@ARGV);
exit $status;

__END__

=for stopwords
en em --stderr stderr --utf8 UTF-8 overdo markup MT-LEVEL Allbery Solaris URL
troff troff-specific formatters uppercased Christiansen --nourls UTC prepend
lquote rquote unrepresentable mandoc manref EBCDIC

=head1 NAME

pod2man - Convert POD data to formatted *roff input

=head1 SYNOPSIS

pod2man [B<--center>=I<string>] [B<--date>=I<string>]
    [B<--encoding>=I<encoding>] [B<--errors>=I<style>] [B<--fixed>=I<font>]
    [B<--fixedbold>=I<font>] [B<--fixeditalic>=I<font>]
    [B<--fixedbolditalic>=I<font>] [B<--guesswork>=I<rule>[,I<rule>...]]
    [B<--name>=I<name>] [B<--nourls>] [B<--official>]
    [B<--release>=I<version>] [B<--section>=I<manext>]
    [B<--quotes>=I<quotes>] [B<--lquote>=I<quote>] [B<--rquote>=I<quote>]
    [B<--stderr>] [B<--utf8>] [B<--verbose>] [I<input> [I<output>] ...]

pod2man B<--help>

=head1 DESCRIPTION

B<pod2man> is a wrapper script around the L<Pod::Man> module, using it to
generate *roff input from POD source.  The resulting *roff code is suitable
for display on a terminal using L<nroff(1)>, normally via L<man(1)>, or
printing using L<troff(1)>.

By default (on non-EBCDIC systems), B<pod2man> outputs UTF-8 manual pages.
Its output should work with the B<man> program on systems that use B<groff>
(most Linux distributions) or B<mandoc> (most BSD variants), but may result in
mangled output on older UNIX systems.  To choose a different, possibly more
backward-compatible output mangling on such systems, use C<--encoding=roff>
(the default in earlier Pod::Man versions).  See the B<--encoding> option and
L<Pod::Man/ENCODING> for more details.

I<input> is the file to read for POD source (the POD can be embedded in code).
If I<input> isn't given, it defaults to C<STDIN>.  I<output>, if given, is the
file to which to write the formatted output.  If I<output> isn't given, the
formatted output is written to C<STDOUT>.  Several POD files can be processed
in the same B<pod2man> invocation (saving module load and compile times) by
providing multiple pairs of I<input> and I<output> files on the command line.

B<--section>, B<--release>, B<--center>, B<--date>, and B<--official> can be
used to set the headers and footers to use.  If not given, Pod::Man will
assume various defaults.  See below for details.

=head1 OPTIONS

Each option is annotated with the version of podlators in which that option
was added with its current meaning.

=over 4

=item B<-c> I<string>, B<--center>=I<string>

[1.00] Sets the centered page header for the C<.TH> macro to I<string>.  The
default is C<User Contributed Perl Documentation>, but also see B<--official>
below.

=item B<-d> I<string>, B<--date>=I<string>

[4.00] Set the left-hand footer string for the C<.TH> macro to I<string>.  By
default, the first of POD_MAN_DATE, SOURCE_DATE_EPOCH, the modification date
of the input file, or the current date (if input comes from C<STDIN>) will be
used, and the date will be in UTC.  See L<Pod::Man/CLASS METHODS> for more
details.

=item B<-e> I<encoding>, B<--encoding>=I<encoding>

[5.00] Specifies the encoding of the output.  I<encoding> must be an encoding
recognized by the L<Encode> module (see L<Encode::Supported>).  The default on
non-EBCDIC systems is UTF-8.

If the output contains characters that cannot be represented in this encoding,
that is an error that will be reported as configured by the B<--errors>
option.  If error handling is other than C<die>, the unrepresentable character
will be replaced with the Encode substitution character (normally C<?>).

If the C<encoding> option is set to the special value C<groff> (the default on
EBCDIC systems), or if the Encode module is not available and the encoding is
set to anything other than C<roff> (see below), Pod::Man will translate all
non-ASCII characters to C<\[uNNNN]> Unicode escapes.  These are not
traditionally part of the *roff language, but are supported by B<groff> and
B<mandoc> and thus by the majority of manual page processors in use today.

If I<encoding> is set to the special value C<roff>, B<pod2man> will do its
historic transformation of (some) ISO 8859-1 characters into *roff escapes
that may be adequate in troff and may be readable (if ugly) in nroff.  This
was the default behavior of versions of B<pod2man> before 5.00.  With this
encoding, all other non-ASCII characters will be replaced with C<X>.  It may
be required for very old troff and nroff implementations that do not support
UTF-8, but its representation of any non-ASCII character is very poor and
often specific to European languages.  Its use is discouraged.

WARNING: The input encoding of the POD source is independent from the output
encoding, and setting this option does not affect the interpretation of the
POD input.  Unless your POD source is US-ASCII, its encoding should be
declared with the C<=encoding> command in the source.  If this is not done,
Pod::Simple will will attempt to guess the encoding and may be successful if
it's Latin-1 or UTF-8, but it will produce warnings.  See L<perlpod(1)> for
more information.

=item B<--errors>=I<style>

[2.5.0] Set the error handling style.  C<die> says to throw an exception on
any POD formatting error.  C<stderr> says to report errors on standard error,
but not to throw an exception.  C<pod> says to include a POD ERRORS section in
the resulting documentation summarizing the errors.  C<none> ignores POD
errors entirely, as much as possible.

The default is C<die>.

=item B<--fixed>=I<font>

[1.0] The fixed-width font to use for verbatim text and code.  Defaults to
C<CW>.  Some systems may want C<CR> instead.  Only matters for B<troff>
output.

=item B<--fixedbold>=I<font>

[1.0] Bold version of the fixed-width font.  Defaults to C<CB>.  Only matters
for B<troff> output.

=item B<--fixeditalic>=I<font>

[1.0] Italic version of the fixed-width font (something of a misnomer, since
most fixed-width fonts only have an oblique version, not an italic version).
Defaults to C<CI>.  Only matters for B<troff> output.

=item B<--fixedbolditalic>=I<font>

[1.0] Bold italic (in theory, probably oblique in practice) version of the
fixed-width font.  Pod::Man doesn't assume you have this, and defaults to
C<CB>.  Some systems (such as Solaris) have this font available as C<CX>.
Only matters for B<troff> output.

=item B<--guesswork>=I<rule>[,I<rule>...]

[5.00] By default, B<pod2man> applies some default formatting rules based on
guesswork and regular expressions that are intended to make writing Perl
documentation easier and require less explicit markup.  These rules may not
always be appropriate, particularly for documentation that isn't about Perl.
This option allows turning all or some of it off.

The special rule C<all> enables all guesswork.  This is also the default for
backward compatibility reasons.  The special rule C<none> disables all
guesswork.  Otherwise, the value of this option should be a comma-separated
list of one or more of the following keywords:

=over 4

=item functions

Convert function references like C<foo()> to bold even if they have no markup.
The function name accepts valid Perl characters for function names (including
C<:>), and the trailing parentheses must be present and empty.

=item manref

Make the first part (before the parentheses) of man page references like
C<foo(1)> bold even if they have no markup.  The section must be a single
number optionally followed by lowercase letters.

=item quoting

If no guesswork is enabled, any text enclosed in CZ<><> is surrounded by
double quotes in nroff (terminal) output unless the contents are already
quoted.  When this guesswork is enabled, quote marks will also be suppressed
for Perl variables, function names, function calls, numbers, and hex
constants.

=item variables

Convert Perl variable names to a fixed-width font even if they have no markup.
This transformation will only be apparent in troff output, or some other
output format (unlike nroff terminal output) that supports fixed-width fonts.

=back

Any unknown guesswork name is silently ignored (for potential future
compatibility), so be careful about spelling.

=item B<-h>, B<--help>

[1.00] Print out usage information.

=item B<-l>, B<--lax>

[1.00] No longer used.  B<pod2man> used to check its input for validity as a
manual page, but this should now be done by L<podchecker(1)> instead.
Accepted for backward compatibility; this option no longer does anything.

=item B<--language>=I<language>

[5.00] Add commands telling B<groff> that the input file is in the given
language.  The value of this setting must be a language abbreviation for which
B<groff> provides supplemental configuration, such as C<ja> (for Japanese) or
C<zh> (for Chinese).

This adds:

    .mso <language>.tmac
    .hla <language>

to the start of the file, which configure correct line breaking for the
specified language.  Without these commands, groff may not know how to add
proper line breaks for Chinese and Japanese text if the man page is installed
into the normal man page directory, such as F</usr/share/man>.

On many systems, this will be done automatically if the man page is installed
into a language-specific man page directory, such as F</usr/share/man/zh_CN>.
In that case, this option is not required.

Unfortunately, the commands added with this option are specific to B<groff>
and will not work with other B<troff> and B<nroff> implementations.

=item B<--lquote>=I<quote>

=item B<--rquote>=I<quote>

[4.08] Sets the quote marks used to surround CE<lt>> text.  B<--lquote> sets
the left quote mark and B<--rquote> sets the right quote mark.  Either may
also be set to the special value C<none>, in which case no quote mark is added
on that side of CE<lt>> text (but the font is still changed for troff output).

Also see the B<--quotes> option, which can be used to set both quotes at once.
If both B<--quotes> and one of the other options is set, B<--lquote> or
B<--rquote> overrides B<--quotes>.

=item B<-n> I<name>, B<--name>=I<name>

[4.08] Set the name of the manual page for the C<.TH> macro to I<name>.
Without this option, the manual name is set to the uppercased base name of the
file being converted unless the manual section is 3, in which case the path is
parsed to see if it is a Perl module path.  If it is, a path like
C<.../lib/Pod/Man.pm> is converted into a name like C<Pod::Man>.  This option,
if given, overrides any automatic determination of the name.

Although one does not have to follow this convention, be aware that the
convention for UNIX manual pages is for the title to be in all-uppercase, even
if the command isn't.  (Perl modules traditionally use mixed case for the
manual page title, however.)

This option is probably not useful when converting multiple POD files at once.

When converting POD source from standard input, the name will be set to
C<STDIN> if this option is not provided.  Providing this option is strongly
recommended to set a meaningful manual page name.

=item B<--nourls>

[2.5.0] Normally, LZ<><> formatting codes with a URL but anchor text are
formatted to show both the anchor text and the URL.  In other words:

    L<foo|http://example.com/>

is formatted as:

    foo <http://example.com/>

This flag, if given, suppresses the URL when anchor text is given, so this
example would be formatted as just C<foo>.  This can produce less
cluttered output in cases where the URLs are not particularly important.

=item B<-o>, B<--official>

[1.00] Set the default header to indicate that this page is part of the
standard Perl release, if B<--center> is not also given.

=item B<-q> I<quotes>, B<--quotes>=I<quotes>

[4.00] Sets the quote marks used to surround CE<lt>> text to I<quotes>.  If
I<quotes> is a single character, it is used as both the left and right quote.
Otherwise, it is split in half, and the first half of the string is used as
the left quote and the second is used as the right quote.

I<quotes> may also be set to the special value C<none>, in which case no quote
marks are added around CE<lt>> text (but the font is still changed for troff
output).

Also see the B<--lquote> and B<--rquote> options, which can be used to set the
left and right quotes independently.  If both B<--quotes> and one of the other
options is set, B<--lquote> or B<--rquote> overrides B<--quotes>.

=item B<-r> I<version>, B<--release>=I<version>

[1.00] Set the centered footer for the C<.TH> macro to I<version>.  By
default, this is set to the version of Perl you run B<pod2man> under.  Setting
this to the empty string will cause some *roff implementations to use the
system default value.

Note that some system C<an> macro sets assume that the centered footer will be
a modification date and will prepend something like C<Last modified: >.  If
this is the case for your target system, you may want to set B<--release> to
the last modified date and B<--date> to the version number.

=item B<-s> I<string>, B<--section>=I<string>

[1.00] Set the section for the C<.TH> macro.  The standard section numbering
convention is to use 1 for user commands, 2 for system calls, 3 for functions,
4 for devices, 5 for file formats, 6 for games, 7 for miscellaneous
information, and 8 for administrator commands.  There is a lot of variation
here, however; some systems (like Solaris) use 4 for file formats, 5 for
miscellaneous information, and 7 for devices.  Still others use 1m instead of
8, or some mix of both.  About the only section numbers that are reliably
consistent are 1, 2, and 3.

By default, section 1 will be used unless the file ends in C<.pm>, in which
case section 3 will be selected.

=item B<--stderr>

[2.1.3] By default, B<pod2man> dies if any errors are detected in the POD
input.  If B<--stderr> is given and no B<--errors> flag is present, errors are
sent to standard error, but B<pod2man> does not abort.  This is equivalent to
C<--errors=stderr> and is supported for backward compatibility.

=item B<-u>, B<--utf8>

[2.1.0] This option used to tell B<pod2man> to produce UTF-8 output.  Since
this is now the default as of version 5.00, it is ignored and does nothing.

=item B<-v>, B<--verbose>

[1.11] Print out the name of each output file as it is being generated.

=back

=head1 EXIT STATUS

As long as all documents processed result in some output, even if that output
includes errata (a C<POD ERRORS> section generated with C<--errors=pod>),
B<pod2man> will exit with status 0.  If any of the documents being processed
do not result in an output document, B<pod2man> will exit with status 1.  If
there are syntax errors in a POD document being processed and the error
handling style is set to the default of C<die>, B<pod2man> will abort
immediately with exit status 255.

=head1 DIAGNOSTICS

If B<pod2man> fails with errors, see L<Pod::Man> and L<Pod::Simple> for
information about what those errors might mean.

=head1 EXAMPLES

    pod2man program > program.1
    pod2man SomeModule.pm /usr/perl/man/man3/SomeModule.3
    pod2man --section=7 note.pod > note.7

If you would like to print out a lot of man page continuously, you probably
want to set the C and D registers to set contiguous page numbering and
even/odd paging, at least on some versions of man(7).

    troff -man -rC1 -rD1 perl.1 perldata.1 perlsyn.1 ...

To get index entries on C<STDERR>, turn on the F register, as in:

    troff -man -rF1 perl.1

The indexing merely outputs messages via C<.tm> for each major page, section,
subsection, item, and any C<XE<lt>E<gt>> directives.

=head1 AUTHOR

Russ Allbery <rra@cpan.org>, based on the original B<pod2man> by Larry Wall
and Tom Christiansen.

=head1 COPYRIGHT AND LICENSE

Copyright 1999-2001, 2004, 2006, 2008, 2010, 2012-2019, 2022 Russ Allbery
<rra@cpan.org>

This program is free software; you may redistribute it and/or modify it
under the same terms as Perl itself.

=head1 SEE ALSO

L<Pod::Man>, L<Pod::Simple>, L<man(1)>, L<nroff(1)>, L<perlpod(1)>,
L<podchecker(1)>, L<perlpodstyle(1)>, L<troff(1)>, L<man(7)>

The man page documenting the an macro set may be L<man(5)> instead of
L<man(7)> on your system.

The current version of this script is always available from its web site at
L<https://www.eyrie.org/~eagle/software/podlators/>.  It is also part of the
Perl core distribution as of 5.6.0.

=cut
SCRIPT_BODY

# Finish the generation of the script.
close($out) or die "Cannot close $file: $!\n";
chmod(0755, $file) or die "Cannot reset permissions for $file: $!\n";
if ($Config{'eunicefix'} ne q{:}) {
    exec("$Config{'eunicefix'} $file");
}

# Local Variables:
# copyright-at-end-flag: t
# End:
