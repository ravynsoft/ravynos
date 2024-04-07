#!/usr/bin/perl -w

package main;

BEGIN {
    chdir 't' if -d 't';
    @INC = "../lib";
    # Do not require test.pl, this file has its own framework.
}

use strict;
use warnings;
use feature 'unicode_strings';
no warnings 'experimental::builtin';
use builtin 'refaddr';

use Carp;
use Config;
use Digest;
use File::Find;
use File::Spec;
use Text::Tabs;

$| = 1;

BEGIN {
    if ( $Config{usecrosscompile} ) {
        print "1..0 # Not all files are available during cross-compilation\n";
        exit 0;
    }
    if ($^O eq 'dec_osf') {
        print "1..0 # $^O cannot handle this test\n";
        exit(0);
    }
    if ( $ENV{'PERL_BUILD_PACKAGING'} ) {
        print "1..0 # This distro may have modified some files in cpan/. Skipping validation. \n";
        exit 0;
    }
    require '../regen/regen_lib.pl';
}

sub DEBUG { 0 };

=pod

=head1 NAME

podcheck.t - Look for possible problems in the Perl pods

=head1 SYNOPSIS

 cd t
 ./perl -I../lib porting/podcheck.t [--show-all] [--cpan] [--deltas]
                                    [--counts] [--pedantic] [FILE ...]

 ./perl -I../lib porting/podcheck.t --add-link MODULE ...

 ./perl -I../lib porting/podcheck.t --regen

=head1 DESCRIPTION

podcheck.t is an extension of Pod::Checker.  It looks for pod errors and
potential errors in the files given as arguments, or if none specified, in all
pods in the distribution workspace, except certain known special ones
(specified below).  It does additional checking beyond that done by
Pod::Checker, and keeps a database of known potential problems, and will
fail a pod only if the number of such problems differs from that given in the
database.

The additional checks it always makes are:

=over

=item Cross-pod link checking

Pod::Checker verifies that links to an internal target in a pod are not
broken.  podcheck.t extends that (when called without FILE arguments) to
external links.  It does this by gathering up all the possible targets in the
workspace, and cross-checking them.  It also checks that a non-broken link
points to just one target.  (The destination pod could have two targets with
the same name.)

The way that the C<LE<lt>E<gt>> pod command works (for links outside the pod)
is to actually create a link to C<metacpan.org> with an embedded query for
the desired pod or man page.  That means that links outside the distribution
are valid.  podcheck.t doesn't verify the validity of such links, but instead
keeps a database of those known to be valid.  This means that if a link to a
target not on the list is created, the target needs to be added to the data
base.  This is accomplished via the L<--add-link|/--add-link MODULE ...>
option to podcheck.t, described below.

=item An internal link that isn't so specified

If a link is broken, but there is an existing internal target of the same
name, it is likely that the internal target was meant, and the C<"/"> is
missing from the C<LE<lt>E<gt>> pod command.

=item Missing or duplicate NAME or missing NAME short description

A pod can't be linked to unless it has a unique name.
And a NAME should have a dash and short description after it.

=item Occurrences of the Unicode replacement character

L<Pod::Simple> replaces bytes that aren't valid according to the document's
encoding (declared or auto-detected) with C<\N{REPLACEMENT CHARACTER}>.

=back

If the C<PERL_POD_PEDANTIC> environment variable is set or the C<--pedantic>
command line argument is provided, then a few more checks are made.
The pedantic checks are:

=over

=item Verbatim paragraphs that wrap in an 80 (including 2 spare) column window

Pod that inappropriately wraps is less legible.  Pod formatters generally wrap
correctly, except for too long verbatim lines.  We assume that any display
window has at least the traditional 80 columns, and check for verbatim lines
that won't fit in that space, including when using a pager that reserves 2
columns for its own use.  (Thus the check is for a net of 78 columns.)
For those lines that don't fit, it tells you how much needs to be cut in
order to fit.

Often, the easiest thing to do to gain space for these is to lower the indent
to just one space.

=item Items that perhaps should be links

There are mentions of apparent files in the pods that perhaps should be links
instead, using C<LE<lt>...E<gt>>

=item Items that perhaps should be C<FE<lt>...E<gt>>

What look like path names enclosed in C<CE<lt>...E<gt>> should perhaps have
C<FE<lt>...E<gt>> mark-up instead.

=back

A number of issues raised by podcheck.t and by the base Pod::Checker are not
really problems, but merely potential problems, that is, false positives.
After inspecting them and
deciding that they aren't real problems, it is possible to shut up this program
about them, unlike base Pod::Checker.  For a valid link to an outside module
or man page, call podcheck.t with the C<--add-link> option to add it to the
database of known links; for other causes, call podcheck.t with the C<--regen>
option to regenerate the entire database.  This tells it that all existing
issues are to not be mentioned again.

C<--regen> isn't fool-proof.  The database merely keeps track of the number of these
potential problems of each type for each pod.  If a new problem of a given
type is introduced into the pod, podcheck.t will spit out all of them.  You
then have to figure out which is the new one, and should it be changed or not.
But doing it this way insulates the database from having to keep track of line
numbers of problems, which may change, or the exact wording of each problem
which might also change without affecting whether it is a problem or not.

Also, if the count of potential problems of a given type for a pod decreases,
the database must be regenerated so that it knows the new number.  The program
gives instructions when this happens.

Some pods will have varying numbers of problems of a given type.  This can
be handled by manually editing the database file (see L</FILES>), and setting
the number of those problems for that pod to a negative number.  This will
cause the corresponding error to always be suppressed no matter how many there
actually are.

Another problem is that there is currently no check that modules listed as
valid in the database
actually are.  Thus any errors introduced there will remain there.

=head2 Specially handled pods

=over

=item perltoc

This pod is generated by pasting bits from other pods.  Errors in those bits
will show up as errors here, as well as for those other pods.  Therefore
errors here are suppressed, and the pod is checked only to verify that nodes
within it actually exist that are externally linked to.

=item perldelta

The current perldelta pod is initialized from a template that contains
placeholder text.  Some of this text is in the form of links that don't really
exist.  Any such links that are listed in C<@perldelta_ignore_links> will not
generate messages.  It is presumed that these links will be cleaned up when
the perldelta is cleaned up for release since they should be marked with
C<XXX>.

=item Porting/perldelta_template.pod

This is not a pod, but a template for C<perldelta>.  Any errors introduced
by it will show up when C<perldelta> is created from it.

=item cpan-upstream pods

See the L</--cpan> option documentation

=item old perldeltas

See the L</--deltas> option documentation

=back

=head1 OPTIONS

=over

=item --add-link MODULE ...

Use this option to teach podcheck.t that the C<MODULE>s or man pages actually
exist, and to silence any messages that links to them are broken.

podcheck.t checks that links within the Perl core distribution are valid, but
it doesn't check links to man pages or external modules.  When it finds
a broken link, it checks its database of external modules and man pages,
and only if not found there does it raise a message.  This option just adds
the list of modules and man page references that follow it on the command line
to that database.

For example,

    cd t
    ./perl -I../lib porting/podcheck.t --add-link Unicode::Casing

causes the external module "Unicode::Casing" to be added to the database, so
C<LE<lt>Unicode::CasingE<gt>> will be considered valid.

=item --regen

Regenerate the database used by podcheck.t to include all the existing
potential problems.  Future runs of the program will not then flag any of
these.  Setting this option also sets C<--pedantic>.

=item --cpan

Normally, all pods in the cpan directory are skipped, except to make sure that
any blead-upstream links to such pods are valid.
This option will cause cpan upstream pods to be fully checked.

=item --deltas

Normally, all old perldelta pods are skipped, except to make sure that
any links to such pods are valid.  This is because they are considered
stable, and perhaps trying to fix them will cause changes that will
misrepresent Perl's history.  But, this option will cause them to be fully
checked.

=item --show-all

Normally, if the number of potential problems of a given type found for a
pod matches the expected value in the database, they will not be displayed.
This option forces the database to be generally ignored during the run, so all
potential problems are displayed and will fail their respective pod test.
If, however, the database indicates that a particular problem type for a
particular file is to be skipped, this option doesn't override that unless
that particular file is passed specifically as one of the FILE parameters on
the command line.  And, passing particular FILEs selects this option in
general.

=item --counts

Instead of testing, this just dumps the counts of the occurrences of the
various types of potential problems in the database.

=item --pedantic

There are three potential problems that are not checked for by default.
This options enables them. The environment variable C<PERL_POD_PEDANTIC>
can be set to 1 to enable this option also.
This option is set when C<--regen> is used.

=back

=head1 FILES

The database is stored in F<t/porting/known_pod_issues.dat>

=head1 SEE ALSO

L<Pod::Checker>

=cut

# VMS builds have a '.com' appended to utility and script names, and it adds a
# trailing dot for any other file name that doesn't have a dot in it.  The db
# is stored without those things.  This regex allows for these special file
# names to be dealt with.  It needs to be interpolated into a larger regex
# that furnishes the closing boundary.
my $vms_re = qr/ \. (?: com )? /x;

# Some filenames in the MANIFEST match $vms_re, and so must not be handled the
# same way that the special vms ones are.  This hash lists those.
my %special_vms_files;

# This is to get this to work across multiple file systems, including those
# that are not case sensitive.  The db is stored in lower case, Un*x style,
# and all file name comparisons are done that way.
sub canonicalize($) {
    my $input = shift;
    my ($volume, $directories, $file)
                    = File::Spec->splitpath(File::Spec->canonpath($input));
    # Assumes $volume is constant for everything in this directory structure
    $directories = "" if ! $directories;
    $file = "" if ! $file;
    $file = lc join '/', File::Spec->splitdir($directories), $file;
    $file =~ s! / /+ !/!gx;       # Multiple slashes => single slash

    # The db is stored without the special suffixes that are there in VMS, so
    # strip them off to get the comparable name.  But some files on all
    # platforms have these suffixes, so this shouldn't happen for them, as any
    # of their db entries will have the suffixes in them.  The hash has been
    # populated with these files.
    if ($^O eq 'VMS'
        && $file =~ / ( $vms_re ) $ /x
        && ! exists $special_vms_files{$file})
    {
        $file =~ s/ $1 $ //x;
    }
    return $file;
}

#####################################################
# HOW IT WORKS (in general)
#
# If not called with specific files to check, the directory structure is
# examined for files that have pods in them.  Files that might not have to be
# fully parsed (e.g. in cpan) are parsed enough at this time to find their
# pod's NAME, and to get a checksum.
#
# Those kinds of files are sorted last, but otherwise the pods are parsed with
# the package coded here, My::Pod::Checker, which is an extension to
# Pod::Checker that adds some tests and suppresses others that aren't
# appropriate.  The latter module has no provision for capturing diagnostics,
# so a package, Tie_Array_to_FH, is used to force them to be placed into an
# array instead of printed.
#
# Parsing the files builds up a list of links.  The files are gone through
# again, doing cross-link checking and outputting all saved-up problems with
# each pod.
#
# Sorting the files last that potentially don't need to be fully parsed allows
# us to not parse them unless there is a link to an internal anchor in them
# from something that we have already parsed.  Keeping checksums allows us to
# not parse copies of other pods.
#
#####################################################

# 1 => Exclude low priority messages that aren't likely to be problems, and
# has many false positives; higher numbers give more messages.
my $Warnings_Level = 200;

# perldelta during construction may have place holder links.  N.B.  This
# variable is referred to by name in release_managers_guide.pod
our @perldelta_ignore_links = ( "XXX", "perl5YYYdelta", "perldiag/message" );

# To see if two pods with the same NAME are actually copies of the same pod,
# which is not an error, it uses a checksum to save work.
my $digest_type = "SHA-1";

my $original_t_dir = File::Spec->rel2abs(File::Spec->curdir);
my $data_dir = File::Spec->catdir($original_t_dir, 'porting');
my $known_issues = File::Spec->catfile($data_dir, 'known_pod_issues.dat');
my $MANIFEST = File::Spec->catfile(File::Spec->updir($original_t_dir), 'MANIFEST');
my $copy_fh;

my $MAX_LINE_LENGTH = 78;   # 78 columns
my $INDENT = 4;             # Lines other than '=head' lines are indented at
                            # least this much

# Our warning messages.  Better not have [('"] in them, as those are used as
# delimiters for variable parts of the messages by poderror.
my $broken_link = "Apparent broken link";
my $broken_internal_link = "Apparent internal link is missing its forward slash";
my $multiple_targets = "There is more than one target";
my $duplicate_name = "Pod NAME already used";
my $no_name = "There is no NAME";
my $missing_name_description = "The NAME should have a dash and short description after it";
my $replacement_character = "Unicode replacement character found";
# the pedantic warnings messages
my $line_length = "Verbatim line length including indents exceeds $MAX_LINE_LENGTH by";
my $C_not_linked = "? Should you be using L<...> instead of";
my $C_with_slash = "? Should you be using F<...> or maybe L<...> instead of";

# objects, tests, etc can't be pods, so don't look for them. Also skip
# files output by the patch program.  Could also ignore most of .gitignore
# files, but not all, so don't.

my $obj_ext = $Config{'obj_ext'}; $obj_ext =~ tr/.//d; # dot will be added back
my $lib_ext = $Config{'lib_ext'}; $lib_ext =~ tr/.//d;
my $lib_so  = $Config{'so'};      $lib_so  =~ tr/.//d;
my $dl_ext  = $Config{'dlext'};   $dl_ext  =~ tr/.//d;

# Not really pods, but can look like them.
my %excluded_files = (
                        canonicalize("lib/unicore/mktables") => 1,
                        canonicalize("Porting/make-rmg-checklist") => 1,
                        canonicalize("Porting/perldelta_template.pod") => 1,
                        canonicalize("regen/feature.pl") => 1,
                        canonicalize("regen/warnings.pl") => 1,
                        canonicalize("autodoc.pl") => 1,
                        canonicalize("configpm") => 1,
                        canonicalize("miniperl") => 1,
                        canonicalize("perl") => 1,
                        canonicalize('cpan/Pod-Perldoc/corpus/no-head.pod') => 1,
                        canonicalize('cpan/Pod-Perldoc/corpus/perlfunc.pod') => 1,
                        canonicalize('cpan/Pod-Perldoc/corpus/utf8.pod') => 1,
                        canonicalize("lib/unicore/mktables") => 1,
                        canonicalize("dist/devel-ppport/parts/inc/ppphdoc") => 1,
                    );

# This list should not include anything for which case sensitivity is
# important, as it won't work on VMS, and won't show up until tested on VMS.
# All or almost all such files should be listed in the MANIFEST, so that can
# be examined for them, and each such file explicitly excluded, as is done for
# .PL files in the loop just below this.  For files not catchable this way,
# is_pod_file() can be used to exclude these at a finer grained level.
my $non_pods = qr/
                (?: \. (?: [achot]  | zip | gz | bz2 | jar | tar | tgz
                           | orig | rej | patch   # Patch program output
                           | sw[op] | \#.*  # Editor droppings
                           | old      # buildtoc output
                           | xs       # pod should be in the .pm file
                           | al       # autosplit files
                           | bs       # bootstrap files
                           | (?i:sh)  # shell scripts, hints, templates
                           | lst      # assorted listing files
                           | bat      # Windows,OS2 batch files
                           | cmd      # Windows,OS2 command files
                           | lis      # VMS compiler listings
                           | map      # VMS linker maps
                           | opt      # VMS linker options files
                           | mms      # MM(K|S) description files
                           | ts       # timestamp files generated during build
                           | txt      # plain text
                           | $obj_ext # object files
                           | exe      # $Config{'exe_ext'} might be empty string
                           | $lib_ext # object libraries
                           | $lib_so  # shared libraries
                           | $dl_ext  # dynamic libraries
                           | gif      # GIF images (example files from CGI.pm)
                           | eg       # examples from libnet
                           | U        # metaconfig unit
                           | core .*
                       )
                 $
               ) | ~$                    # Vim droppings
                 | \.bak$                # Other editor droppings
                 | \ \(Autosaved\)\.txt$ # Other editor droppings
                 | ^cxx\$demangler_db\.$ # VMS name mangler database
                 | ^typemap\.?$          # typemap files
                 | ^(?i:Makefile\.PL)$
                 | ^core (?: $ | \. .* )
                 | ^vgcore\.[1-9][0-9]*$
                 | \b Changes \b

                   # This is a pod, but is part of a corpus to test agains; we
                   # don't care about any issues in it.
                 | ext\/Pod-Html\/corpus\/perlvar-copy.pod
             /x;

# Matches something that looks like a file name, but is enclosed in C<...>
my $C_path_re = qr{ ^
                        # exclude various things that have slashes
                        # in them but aren't paths
                        (?!
                            (?: (?: s | qr | m | tr | y ) / ) # regexes
                            | \d+/\d+ \b       # probable fractions
                            | (?: [LF] < )+
                            | OS/2 \b
                            | Perl/perl.git \b
                            | Perl/perl5.git \b
                            | Perl/Tk \b
                            | origin/blead \b
                            | origin/maint \b
                        )
                        /?  # Optional initial slash
                        \w+ # First component of path, doesn't begin with
                            # a minus
                        (?: / [-\w]+ )+ # Subsequent path components
                        (?: \. \w+ )?   # Optional trailing dot and suffix
                        >*  # Any enclosed L< F< have matching closing >
                        $
                    }x;

# '.PL' files should be excluded, as they aren't final pods, but often contain
# material used in generating pods, and so can look like a pod.  We can't use
# the regexp above because case sensitivity is important for these, as some
# '.pl' files should be examined for pods.  Instead look through the MANIFEST
# for .PL files and get their full path names, so we can exclude each such
# file explicitly.  This works because other porting tests prohibit having two
# files with the same names except for case.
open my $manifest_fh, '<:bytes', $MANIFEST or die "Can't open $MANIFEST";
while (<$manifest_fh>) {

    # While we have MANIFEST open, on VMS platforms, look for files that match
    # the magic VMS file names that have to be handled specially.  Add these
    # to the list of them.
    if ($^O eq 'VMS' && / ^ ( [^\t]* $vms_re ) \t /x) {
        $special_vms_files{$1} = 1;
    }
    if (/ ^ ( [^\t]* \. PL ) \t /x) {
        $excluded_files{canonicalize($1)} = 1;
    }
}
close $manifest_fh, or die "Can't close $MANIFEST";


# Pod::Checker messages to suppress
my @suppressed_messages = (
    # We catch independently the ones that are real problems.
    qr/multiple occurrences \(\d+\) of link target/,

    "unescaped <>",                 # Not every '<' or '>' need be escaped
    qr/No items in =over/,          # i.e., a blockquote, which we consider legal
);

sub suppressed {
    # Returns bool as to if input message is one that is to be suppressed

    my $message = shift;

    return grep { $message =~ /^$_/i } @suppressed_messages;
}

{   # Closure to contain a simple subset of test.pl.  This is to get rid of the
    # unnecessary 'failed at' messages that would otherwise be output pointing
    # to a particular line in this file.

    my $current_test = 0;
    my $planned;

    sub plan {
        my %plan = @_;
        $planned = $plan{tests} + 1;    # +1 for final test that files haven't
                                        # been removed
        print "1..$planned\n";
        return;
    }

    sub ok {
        my $success = shift;
        my $message = shift;

        chomp $message;

        $current_test++;
        print "not " unless $success;
        print "ok $current_test - $message\n";
        return $success;
    }

    sub skip {
        my $why = shift;
        my $n    = @_ ? shift : 1;
        for (1..$n) {
            $current_test++;
            print "ok $current_test # skip $why\n";
        }
        no warnings 'exiting';
        last SKIP;
    }

    sub _note {
        my ($andle, $message) = @_;

        chomp $message;

        print $andle $message =~ s/^/# /mgr;
        print $andle "\n";
        return;
    }

    sub note { unshift @_, \*STDOUT; goto &_note }

    sub diag { unshift @_, \*STDERR; goto &_note }

    END {
        if ($planned && $planned != $current_test) {
            print STDERR
            "# Looks like you planned $planned tests but ran $current_test.\n";
        }
    }
}

# List of known potential problems by pod and type.
my %known_problems;

# Pods given by the keys contain an interior node that is referred to from
# outside it.
my %has_referred_to_node;

my $show_counts = 0;
my $regen = 0;
my $add_link = 0;
my $show_all = 0;
my $pedantic = 0;

my $do_upstream_cpan = 0; # Assume that are to skip anything in /cpan
my $do_deltas = 0;        # And stable perldeltas

while (@ARGV && substr($ARGV[0], 0, 1) eq '-') {
    my $arg = shift @ARGV;

    $arg =~ s/^--/-/; # Treat '--' the same as a single '-'
    if ($arg eq '-regen') {
        $regen = 1;
        $pedantic = 1;
    }
    elsif ($arg =~ /^-add[-_]link$/) {
        $add_link = 1;
    }
    elsif ($arg eq '-cpan') {
        $do_upstream_cpan = 1;
    }
    elsif ($arg eq '-deltas') {
        $do_deltas = 1;
    }
    elsif ($arg =~ /^-show[-_]all$/) {
        $show_all = 1;
    }
    elsif ($arg eq '-counts') {
        $show_counts = 1;
    }
    elsif ($arg eq '-pedantic') {
        $pedantic = 1;
    }
    else {
        die <<EOF;
Unknown option '$arg'

Usage: $0 [ --regen | --cpan | --show-all | FILE ... | --add-link MODULE ... ]\n"
    --add-link -> Add the MODULE and man page references to the database
    --regen    -> Regenerate the data file for $0
    --cpan     -> Include files in the cpan subdirectory.
    --deltas   -> Include stable perldeltas
    --show-all -> Show all known potential problems
    --counts   -> Don't test, but give summary counts of the currently
                  existing database
    --pedantic -> Check for overly long lines in verbatim blocks
EOF
    }
}

$pedantic = 1 if exists $ENV{PERL_POD_PEDANTIC} and $ENV{PERL_POD_PEDANTIC};
my @files = @ARGV;

my $cpan_or_deltas = $do_upstream_cpan || $do_deltas;
if (($regen + $show_all + $show_counts + $add_link + $cpan_or_deltas ) > 1) {
    croak "--regen, --show-all, --counts, and --add-link are mutually"
        . " exclusive\n and none can be run with --cpan nor --deltas";
}

my $has_input_files = @files;


if ($add_link) {
    if (! $has_input_files) {
        croak "--add-link requires at least one module or man page reference";
    }
}
elsif ($has_input_files) {
    if ($regen || $show_counts || $do_upstream_cpan || $do_deltas) {
        croak "--regen, --counts, --deltas, and --cpan can't be used since using specific files";
    }
    foreach my $file (@files) {
        croak "Can't read file '$file'" if ! -r $file;
    }
}

our %problems;  # potential problems found in this run

package My::Pod::Checker {      # Extend Pod::Checker
    use parent 'Pod::Checker';

    # Uses inside out hash to protect from typos
    # For new fields, remember to add to destructor DESTROY()
    my %CFL_text;           # The text comprising the current C<>, F<>, or L<>
    my %C_text;             # If defined, are in a C<> section, and includes
                            # the accumulated text from that
    my %current_indent;     # Current line's indent
    my %filename;           # The pod is stored in this file
    my %in_CFL;             # count of stacked C<>, F<>, L<> directives
    my %indents;            # Stack of indents from =over's in effect for
                            # current line
    my %in_for;             # true if in a =for or =begin
    my %in_NAME;            # true if within NAME section
    my %in_begin;           # true if within =begin section
    my %in_X;               # true if in a X<>
    my %linkable_item;      # Bool: if the latest =item is linkable.  It isn't
                            # for bullet and number lists
    my %linkable_nodes;     # Pod::Checker adds all =items to its node list,
                            # but not all =items are linkable-to
    my %running_CFL_text;   # The current text that is being accumulated until
                            # an end_FOO is found, and this includes any C<>,
                            # F<>, or L<> directives.
    my %running_simple_text; # The currentt text that is being accumulated
                            # until an end_FOO is found, and all directives
                            # have been expanded into plain text
    my %command_count;      # Number of commands seen
    my %seen_pod_cmd;       # true if have =pod earlier
    my %skip;               # is SKIP set for this pod
    my %start_line;         # the first input line number in the thing
                            # currently being worked on

    sub DESTROY {
        my $addr = refaddr $_[0];
        delete $CFL_text{$addr};
        delete $C_text{$addr};
        delete $command_count{$addr};
        delete $current_indent{$addr};
        delete $filename{$addr};
        delete $in_begin{$addr};
        delete $in_CFL{$addr};
        delete $indents{$addr};
        delete $in_for{$addr};
        delete $in_NAME{$addr};
        delete $in_X{$addr};
        delete $linkable_item{$addr};
        delete $linkable_nodes{$addr};
        delete $running_CFL_text{$addr};
        delete $running_simple_text{$addr};
        delete $seen_pod_cmd{$addr};
        delete $skip{$addr};
        delete $start_line{$addr};
        return;
    }

    sub new {
        my $class = shift;
        my $filename = shift;

        my $self = $class->SUPER::new(-quiet => 1,
                                     -warnings => $Warnings_Level);
        my $addr = refaddr $self;
        $command_count{$addr} = 0;
        $current_indent{$addr} = 0;
        $filename{$addr} = $filename;
        $in_begin{$addr} = 0;
        $in_X{$addr} = 0;
        $in_CFL{$addr} = 0;
        $in_NAME{$addr} = 0;
        $linkable_item{$addr} = 0;
        $seen_pod_cmd{$addr} = 0;
        return $self;
    }

    # re's for messages that Pod::Checker outputs
    my $location = qr/ \b (?:in|at|on|near) \s+ /xi;
    my $optional_location = qr/ (?: $location )? /xi;
    my $line_reference = qr/ [('"]? $optional_location \b line \s+
                             (?: \d+ | EOF | \Q???\E | - )
                             [)'"]? /xi;

    sub poderror {  # Called to register a potential problem

        # This adds an extra field to the parent hash, 'parameter'.  It is
        # used to extract the variable parts of a message leaving just the
        # constant skeleton.  This in turn allows the message to be
        # categorized better, so that it shows up as a single type in our
        # database, with the specifics of each occurrence not being stored with
        # it.

        my $self = shift;
        my $opts = shift;

        my $addr = refaddr $self;
        return if $skip{$addr};

        # Input can be a string or hash.  If a string, parse it to separate
        # out the line number and convert to a hash for easier further
        # processing
        my $message;
        if (ref $opts ne 'HASH') {
            $message = join "", $opts, @_;
            my $line_number;
            if ($message =~ s/\s*($line_reference)//) {
                ($line_number = $1) =~ s/\s*$optional_location//;
            }
            else {
                $line_number = '???';
            }
            $opts = { -msg => $message, -line => $line_number };
        } else {
            $message = $opts->{'-msg'};

        }

        $message =~ s/^\d+\s+//;
        return if main::suppressed($message);

        $self->SUPER::poderror($opts, @_);

        $opts->{parameter} = "" unless $opts->{parameter};

        # The variable parts of the message tend to be enclosed in '...',
        # "....", or (...).  Extract them and put them in an extra field,
        # 'parameter'.  This is trickier because the matching delimiter to a
        # '(' is its mirror, and not itself.  Text::Balanced could be used
        # instead.
        while ($message =~ m/ \s* $optional_location ( [('"] )/xg) {
            my $delimiter = $1;
            my $start = $-[0];
            $delimiter = ')' if $delimiter eq '(';

            # If there is no ending delimiter, don't consider it to be a
            # variable part.  Most likely it is a contraction like "Don't"
            last unless $message =~ m/\G .+? \Q$delimiter/xg;

            my $length = $+[0] - $start;

            # Get the part up through the closing delimiter
            my $special = substr($message, $start, $length);
            $special =~ s/^\s+//;   # No leading whitespace

            # And add that variable part to the parameter, while removing it
            # from the message.  This isn't a foolproof way of finding the
            # variable part.  For example '(s)' can occur in e.g.,
            # 'paragraph(s)'
            if ($special ne '(s)') {
                substr($message, $start, $length) = "";
                pos $message = $start;
                $opts->{-msg} = $message;
                $opts->{parameter} .= " " if $opts->{parameter};
                $opts->{parameter} .= $special;
            }
        }

        # Extract any additional line number given.  This is often the
        # beginning location of something whereas the main line number gives
        # the ending one.
        if ($message =~ /( $line_reference )/xi) {
            my $line_ref = $1;
            while ($message =~ s/\s*\Q$line_ref//) {
                $opts->{-msg} = $message;
                $opts->{parameter} .= " " if $opts->{parameter};
                $opts->{parameter} .= $line_ref;
            }
        }

        Carp::carp("Couldn't extract line number from '$message'") if $message =~ /line \d+/;
        push @{$problems{$filename{$addr}}{$message}}, $opts;
        #push @{$problems{$self->get_filename}{$message}}, $opts;
    }

    # In the next subroutines, we keep track of the text of the current
    # innermost thing, like F<fooC<bar>baz>.  The things we care about raising
    # messages about in this program all come from a single sequence of
    # characters uninterrupted by other pod commands.  Therefore we don't have
    # to worry about recursion, and we can just set the string we care about
    # to empty on entrance to each command.

    sub handle_text {
        # This is called by the parent class to deal with any straight text.
        # We mostly just append this to the running current value which will
        # be dealt with upon the end of the current construct, like a
        # paragraph.  But certain things don't contribute to checking the pod
        # and are ignored.  We also have set flags to indicate this text is
        # going towards constructing certain constructs, and handle those
        # specially.

        my $self = shift;
        my $addr = refaddr $self;

        my $return = $self->SUPER::handle_text(@_);

        if ($in_X{$addr} || $in_for{$addr}) { # ignore
            return $return;
        }

        my $text = join "\n", @_;
        $running_simple_text{$addr} .= $text;

        # Keep separate tabs on C<>, F<>, and L<> directives, and one
        # especially for C<> ones.
        if ($in_CFL{$addr}) {
            $CFL_text{$addr} .= $text;
            $C_text{$addr} .= $text if defined $C_text{$addr};
        }
        else {
            # This variable is updated instead in the corresponding C, F, or L
            # handler.
            $running_CFL_text{$addr} .= $text;
        }

        # do this line-by-line so we can get the right line number
        my @lines = split /^/, $running_simple_text{$addr};
        for my $i (0..$#lines) {
            if ($lines[$i] =~ m/\N{REPLACEMENT CHARACTER}/) {
                $self->poderror({ -line => $start_line{$addr} + $i,
                    -msg => $replacement_character,
                    parameter => "possibly invalid ". $self->encoding . " input at character " . pos $lines[$i],
                });
            }
        }
        return $return;
    }

    # The start_FOO routines check that somehow a C<> construct hasn't escaped
    # without being checked, and initialize things, and call the parent
    # class's equivalent routine.

    # The end_FOO routines close things off, and check the text that has been
    # accumulated for FOO, then call the parent's corresponding routine.

    sub start_Para {
        my $self = shift;
        check_see_but_not_link($self);

        my $addr = refaddr $self;
        $start_line{$addr} = $_[0]->{start_line};
        $running_CFL_text{$addr} = "";
        $running_simple_text{$addr} = "";
        return $self->SUPER::start_Para(@_);
    }

    sub start_item {
        my $self = shift;
        check_see_but_not_link($self);

        my $addr = refaddr $self;
        $start_line{$addr} = $_[0]->{start_line};
        $running_CFL_text{$addr} = "";
        $running_simple_text{$addr} = "";

    }

    sub start_item_text {
        my $self = shift;
        start_item($self);
        my $addr = refaddr $self;

        # This is the only =item that is linkable
        $linkable_item{$addr} = 1;

        return $self->SUPER::start_item_text(@_);
    }

    sub start_item_number {
        my $self = shift;
        start_item($self);

        return $self->SUPER::start_item_number(@_);
    }

    sub start_item_bullet {
        my $self = shift;
        start_item($self);

        return $self->SUPER::start_item_bullet(@_);
    }

    sub end_item {  # No difference in =item types endings
        my $self = shift;
        check_see_but_not_link($self);
        return $self->SUPER::end_item(@_);
    }

    sub start_over {
        my $self = shift;
        check_see_but_not_link($self);

        my $addr = refaddr $self;
        $start_line{$addr} = $_[0]->{start_line};
        $running_CFL_text{$addr} = "";
        $running_simple_text{$addr} = "";

        # Save this indent on a stack, and keep track of total indent
        my $indent =  $_[0]{'indent'};
        push @{$indents{$addr}}, $indent;
        $current_indent{$addr} += $indent;

        return $self->SUPER::start_over(@_);
    }

    sub end_over_bullet { shift->end_over(@_) }
    sub end_over_number { shift->end_over(@_) }
    sub end_over_text   { shift->end_over(@_) }
    sub end_over_block  { shift->end_over(@_) }
    sub end_over_empty  { shift->end_over(@_) }
    sub end_over {
        my $self = shift;
        check_see_but_not_link($self);

        my $addr = refaddr $self;

        # Pop current indent
        if (@{$indents{$addr}}) {
            $current_indent{$addr} -= pop @{$indents{$addr}};
        }
        else {
            # =back without corresponding =over, but should have
            # warned already
            $current_indent{$addr} = 0;
        }
    }

    sub check_see_but_not_link {

        # Looks through accumulated text for current element to see if it
        # refers to something that should be linked to, but isn't.

        my $self = shift;
        my $addr = refaddr $self;

        return unless defined $running_CFL_text{$addr};

        while ($running_CFL_text{$addr} =~ m{
                                ( (?: \w+ \s+ )* )  # The phrase before, if any
                                \b [Ss]ee \s+
                                ( ( [^L] )
                                  <
                                  ( [^<]*? )  # The not < excludes nested C<L<...
                                  >
                                )
                                ( \s+ (?: under | in ) \s+ L< )?
                            }xg)
        {
            my $prefix = $1 // "";
            my $construct = $2;     # The whole thing, like C<...>
            my $type = $3;
            my $interior = $4;
            my $trailing = $5;      # After the whole thing ending in "L<"

            # If the full phrase is something like, "you might see C<", or
            # similar, it really isn't a reference to a link.  The ones I saw
            # all had the word "you" in them; and the "you" wasn't the
            # beginning of a sentence.
            if ($prefix !~ / \b you \b /x) {

                # Now, find what the module or man page name within the
                # construct would be if it actually has L<> syntax.  If it
                # doesn't have that syntax, will set the module to the entire
                # interior.
                if (! defined $trailing # not referring to something in another
                                        # section
                    && $interior !~ /$non_pods/

                    # There can't be spaces (I think) in module names or man
                    # pages
                    && $interior !~ / \s /x

                    # F<> that end in eg \.pl are almost certainly ok, as are
                    # those that look like a path with multiple "/" chars
                    && ($type ne "F"
                        || (! -e $interior
                            && $interior !~ /\.\w+$/
                            && $interior !~ /\/.+\//)
                    )
                ) {
                    # TODO: move the checking of $pedantic higher up
                    $self->poderror({ -line => $start_line{$addr},
                        -msg => $C_not_linked,
                        parameter => $construct
                    });
                }
            }
        }

        undef $running_CFL_text{$addr};
    }

    sub end_Para {
        my $self = shift;
        check_see_but_not_link($self);

        my $addr = refaddr $self;
        if ($in_NAME{$addr}) {
            if ($running_simple_text{$addr} =~ /^\s*(\S+?)\s*$/) {
                $self->poderror({ -line => $start_line{$addr},
                    -msg => $missing_name_description,
                    parameter => $1});
            }
            $in_NAME{$addr} = 0;
        }
        $self->SUPER::end_Para(@_);
    }

    sub start_head1 {
        my $self = shift;
        check_see_but_not_link($self);

        my $addr = refaddr $self;
        $start_line{$addr} = $_[0]->{start_line};
        $running_CFL_text{$addr} = "";
        $running_simple_text{$addr} = "";

        return $self->SUPER::start_head1(@_);
    }

    sub end_head1 {  # This is called at the end of the =head line.
        my $self = shift;
        check_see_but_not_link($self);

        my $addr = refaddr $self;

        $in_NAME{$addr} = 1 if $running_simple_text{$addr} eq 'NAME';
        return $self->SUPER::end_head(@_);
    }

    sub start_Verbatim {
        my $self = shift;
        check_see_but_not_link($self);

        my $addr = refaddr $self;
        $running_simple_text{$addr} = "";
        $start_line{$addr} = $_[0]->{start_line};
        return $self->SUPER::start_Verbatim(@_);
    }

    sub end_Verbatim {
        my $self = shift;
        my $addr = refaddr $self;

        # Pick up the name if it looks like one, since the parent class
        # doesn't handle verbatim NAMEs
        if ($in_NAME{$addr}
            && $running_simple_text{$addr} =~ /^\s*(\S+?)\s*[,-]/)
        {
            $self->name($1);
        }

        my $indent = $self->get_current_indent;

        # Look at each line to verify it is short enough
        my @lines = split /^/, $running_simple_text{$addr};
        for my $i (0 .. @lines - 1) {
            $lines[$i] =~ s/\s+$//;
            my $exceeds = length(Text::Tabs::expand($lines[$i]))
                        + $indent - $MAX_LINE_LENGTH;
            next unless $exceeds > 0;

            $self->poderror({ -line => $start_line{$addr} + $i,
                -msg => $line_length,
                parameter => "+$exceeds (including " . ($indent - $INDENT) .
                             " from =over's and $INDENT as base indent)",
            });
        }

        undef $running_simple_text{$addr};

        # Parent class didn't bother to define this
        #return $self->SUPER::SUPER::end_Verbatim(@_);
    }

    sub start_C {
        my $self = shift;
        my $addr = refaddr $self;

        $C_text{$addr} = "";

        # If not in a stacked set of C<>, F<> and L<>, initialize the text for
        # them.
        $CFL_text{$addr} = "" if ! $in_CFL{$addr};
        $in_CFL{$addr}++;

        return $self->SUPER::start_C(@_);
    }

    sub start_F {
        my $self = shift;
        my $addr = refaddr $self;

        $CFL_text{$addr} = "" if ! $in_CFL{$addr};
        $in_CFL{$addr}++;
        return $self->SUPER::start_F(@_);
    }

    sub start_L {
        my $self = shift;
        my $addr = refaddr $self;

        $CFL_text{$addr} = "" if ! $in_CFL{$addr};
        $in_CFL{$addr}++;
        return $self->SUPER::start_L(@_);
    }

    sub end_C {
        my $self = shift;
        my $addr = refaddr $self;

        # Warn if looks like a file or link enclosed instead by this C<>
        if ($C_text{$addr} =~ qr/^ $C_path_re $/x) {
            # Here it does look like it could be a file path or a link.
            # But some varieties of regex patterns could also fit with what we
            # have so far.  Weed those out as best we can.  '/foo/' is almost
            # certainly meant to be a pattern, as is '/foo/g'.
            my $is_pattern;
            if ($C_text{$addr} !~ qr| ^ / [^/]* / ( [msixpodualngcr]* ) $ |x) {
                $is_pattern = 0;
            }
            else {

                # Here, it looks like a pattern potentially followed by some
                # modifiers.  To make doubly sure, don't count as patterns
                # those constructs which have more occurrences (generally 1)
                # of a modifier than is legal.
                my %counts;
                map { $counts{$_}++ } split "", $1;
                foreach my $modifier (keys %counts) {
                    if ($counts{$modifier} > (($modifier eq 'a')
                                              ? 2
                                              : 1))
                    {
                        $is_pattern = 0;
                        last;
                    }
                }
                $is_pattern = 1 unless defined $is_pattern;
            }

            unless ($is_pattern) {
                $self->poderror({ -line => $start_line{$addr},
                    -msg => $C_with_slash,
                    parameter => "C<$C_text{$addr}>"
                });
            }
        }
        undef $C_text{$addr};

        # Add the current text to the running total.  This was not done in
        # handle_text(), because it just sees the plain text of the innermost
        # stacked directive.  We want to keep all the directive names
        # enclosing the text.  Otherwise the fact that C<L<foobar>> is to a
        # link would be lost, as the L<> would be gone.
        $CFL_text{$addr} = "C<$CFL_text{$addr}>";

        # Add this text to the whole running total only if popping this
        # directive off the stack leaves it empty.  As long as something is on
        # the stack, it gets added to $CFL_text (just above).  It is only
        # entirely constructed when the stack is empty.
        $in_CFL{$addr}--;
        $running_CFL_text{$addr} .= $CFL_text{$addr} if ! $in_CFL{$addr};

        return $self->SUPER::end_C(@_);
    }

    sub end_F {
        my $self = shift;
        my $addr = refaddr $self;

        $CFL_text{$addr} = "F<$CFL_text{$addr}>";
        $in_CFL{$addr}--;
        $running_CFL_text{$addr} .= $CFL_text{$addr} if ! $in_CFL{$addr};
        return $self->SUPER::end_F(@_);
    }

    sub end_L {
        my $self = shift;
        my $addr = refaddr $self;

        $CFL_text{$addr} = "L<$CFL_text{$addr}>";
        $in_CFL{$addr}--;
        $running_CFL_text{$addr} .= $CFL_text{$addr} if ! $in_CFL{$addr};
        return $self->SUPER::end_L(@_);
    }

    sub start_X {
        my $self = shift;
        my $addr = refaddr $self;

        $in_X{$addr} = 1;
        return $self->SUPER::start_X(@_);
    }

    sub end_X {
        my $self = shift;
        my $addr = refaddr $self;

        $in_X{$addr} = 0;
        return $self->SUPER::end_X(@_);
    }

    sub start_for {
        my $self = shift;
        my $addr = refaddr $self;

        $in_for{$addr} = 1;
        return $self->SUPER::start_for(@_);
    }

    sub end_for {
        my $self = shift;
        my $addr = refaddr $self;

        $in_for{$addr} = 0;
        return $self->SUPER::end_for(@_);
    }

    sub hyperlink {
        my ($self, $link) = @_;

        if ($link && $link->type eq 'pod') {
            my $page = $link->page;
            my $node = $link->node;

            # If the hyperlink is to an interior node of another page, save it
            # so that we can see if we need to parse normally skipped files.
            $has_referred_to_node{$page} = 1 if $node;

            # Ignore certain placeholder links in perldelta.  Check if the
            # link is page-level, and also check if to a node within the page
            if (   $self->name && $self->name eq "perldelta"
                && ((  grep { $page eq $_ } @perldelta_ignore_links)
                    || (   $node
                        && (grep { "$page/$node" eq $_ } @perldelta_ignore_links)
            ))) {
                return;
            }
        }

        return $self->SUPER::hyperlink($link);
    }

    sub node {
        my $self = shift;
        my $text = $_[0];
        if($text) {
            $text =~ s/\s+$//s; # strip trailing whitespace
            $text =~ s/\s+/ /gs; # collapse whitespace
            my $addr = refaddr $self;
            push(@{$linkable_nodes{$addr}}, $text) if
                                    ! $current_indent{$addr}
                                    || $linkable_item{$addr};
        }
        return $self->SUPER::node($_[0]);
    }

    sub get_current_indent {
        return $INDENT + $current_indent{refaddr $_[0]};
    }

    sub get_filename {
        return $filename{refaddr $_[0]};
    }

    sub linkable_nodes {
        my $linkables = $linkable_nodes{refaddr $_[0]};
        return undef unless $linkables;
        return @$linkables;
    }

    sub get_skip {
        return $skip{refaddr $_[0]} // 0;
    }

    sub set_skip {
        my $self = shift;
        $skip{refaddr $self} = shift;

        # If skipping, no need to keep the problems for it
        delete $problems{$self->get_filename};
        return;
    }

    sub parse_from_file {
        # This overrides the super class method so that if an open fails on a
        # transitory file, it doesn't croak.  It returns 1 if it did find the
        # file, 0 if it didn't

        my $self = shift;
        my $filename = shift;
        # ignores 2nd param, which is output file.  Always uses undef

        if (open my $in_fh, '<:bytes', $filename) {
            $self->SUPER::parse_from_file($in_fh, undef);
            close $in_fh;
            return 1;
        }

        # If couldn't open file, perhaps it was transitory, and hence not an error
        return 0 unless -e $filename;

        die "Can't open '$filename': $!\n";
    }
}

my %filename_to_checker; # Map a filename to its pod checker object
my %id_to_checker;       # Map a checksum to its pod checker object
my %nodes;               # key is filename, values are nodes in that file.
my %nodes_first_word;    # same, but value is first word of each node
my %valid_modules;       # List of modules known to exist outside us.
my %digests;             # checksums of files, whose names are the keys
my %filename_to_pod;     # Map a filename to its pod NAME
my %files_with_unknown_issues;
my %files_with_fixes;

my $data_fh;
open $data_fh, '<:bytes', $known_issues or die "Can't open $known_issues";

my %counts; # For --counts param, count of each issue type
my %suppressed_files;   # Files with at least one issue type to suppress
my $HEADER = <<END;
# This file is the data file for $0.
# There are three types of lines.
# Comment lines are white-space only or begin with a '#', like this one.  Any
#   changes you make to the comment lines will be lost when the file is
#   regen'd.
# Lines without tab characters are simply NAMES of pods that the program knows
#   will have links to them and the program does not check if those links are
#   valid.
# All other lines should have three fields, each separated by a tab.  The
#   first field is the name of a pod; the second field is an error message
#   generated by this program; and the third field is a count of how many
#   known instances of that message there are in the pod.  -1 means that the
#   program can expect any number of this type of message.
END

my @existing_issues;


while (<$data_fh>) {    # Read the database
    chomp;
    next if /^\s*(?:#|$)/;          # Skip comment and empty lines
    next if /^ [ < = > ]{7} /xx;    # Skip version control conflict markers
    if (/\t/) {
        if ($add_link) {    # The issues are saved and later output unchanged
            push @existing_issues, $_;
            next;
        }

        # Keep track of counts of each issue type for each file
        my ($filename, $message, $count) = split /\t/;

        # The way things aren't shown is to see if the count of the number of
        # warnings of a given type has changed.  To show all, we pretend there
        # weren't any already stored.  If the stored value is negative, it
        # means counting for this warning in this file is disabled, and hence
        # won't change.  To skip showing those files under --show-all, we
        # retain the negatvie value.  To show all occurrences of other
        # warnings, we skip setting their count, making them appear to have
        # had zero occurrences.
        next if $show_all && $count > 0;

        $known_problems{$filename}{$message} = $count;

        if ($show_counts) {
            if ($count < 0) {   # -1 means to suppress this issue type
                $suppressed_files{$filename} = $filename;
            }
            else {
                $counts{$message} += $count;
            }
        }
    }
    else {  # Lines without a tab are modules known to be valid
        $valid_modules{$_} = 1
    }
}
close $data_fh;

if ($add_link) {
    $copy_fh = open_new($known_issues);

    # Check for basic sanity, and add each command line argument
    foreach my $module (@files) {
        die "\"$module\" does not look like a module or man page"
            # Must look like (A or A::B or A::B::C ..., or foo(3C)
            if $module !~ /^ (?: \w+ (?: :: \w+ )* | \w+ \( \d \w* \) ) $/x;
        $valid_modules{$module} = 1
    }
    my_safer_print($copy_fh, $HEADER);
    foreach (sort { lc $a cmp lc $b } keys %valid_modules) {
        my_safer_print($copy_fh, $_, "\n");
    }

    # The rest of the db file is output unchanged.
    my_safer_print($copy_fh, join "\n", @existing_issues, "");

    close_and_rename($copy_fh);
    exit;
}

if ($show_counts) {
    my $total = 0;
    foreach my $message (sort keys %counts) {
        $total += $counts{$message};
        note(Text::Tabs::expand("$counts{$message}\t$message"));
    }
    note("-----\n" . Text::Tabs::expand("$total\tknown potential issues"));
    if (%suppressed_files) {
        note("\nFiles that have all messages of at least one type suppressed:");
        note(join ", ", sort keys %suppressed_files);
    }
    exit 0;
}

# re to match files that are to be parsed only if there is an internal link
# to them.  It does not include cpan, as whether those are parsed depends
# on a switch.  Currently, only perltoc and the stable perldelta.pod's
# are included.  The latter all have characters between 'perl' and
# 'delta'.  (Actually the currently developed one matches as well, but
# is a duplicate of perldelta.pod, so can be skipped, so fine for it to
# match this.
my $only_for_interior_links_re = qr/ ^ pod\/perltoc.pod $
                                   /x;
unless ($do_deltas) {
    $only_for_interior_links_re = qr/$only_for_interior_links_re |
                                    \b perl \d+ delta \. pod \b
                                /x;
}

{ # Closure
    my $first_time = 1;

    sub output_thanks ($$$$) {  # Called when an issue has been fixed
        my $filename = shift;
        my $original_count = shift;
        my $current_count = shift;
        my $message = shift;

        $files_with_fixes{$filename} = 1;
        my $return;
        my $fixed_count = $original_count - $current_count;
        my $a_problem = ($fixed_count == 1) ? "a problem" : "multiple problems";
        my $another_problem = ($fixed_count == 1) ? "another problem" : "another set of problems";
        my $diff;
        if ($message) {
            $diff = <<EOF;
There were $original_count occurrences (now $current_count) in this pod of type
"$message",
EOF
        } else {
            $diff = <<EOF;
There are no longer any problems found in this pod!
EOF
        }

        if ($first_time) {
            $first_time = 0;
            $return = <<EOF;
Thanks for fixing $a_problem!
$diff
Now you must teach $0 that this was fixed.
EOF
        }
        else {
            $return = <<EOF
Thanks for fixing $another_problem.
$diff
EOF
        }

        return $return;
    }
}

sub my_safer_print {    # print, with error checking for outputting to db
    my ($fh, @lines) = @_;

    if (! print $fh @lines) {
        my $save_error = $!;
        close($fh);
        die "Write failure: $save_error";
    }
}

sub extract_pod {   # Extracts just the pod from a file; returns undef if file
                    # doesn't exist
    my $filename = shift;

    if (open my $in_fh, '<:bytes', $filename) {
        use Pod::Simple::JustPod;
        my $parser = Pod::Simple::JustPod->new();
        $parser->no_errata_section(1);
        $parser->no_whining(1);
        $parser->source_filename($filename);
        my $output;
        $parser->output_string( \$output );
        $parser->parse_lines( <$in_fh>, undef );
        close $in_fh;

        return $output;
    }

    # The file should already have been opened once to get here, so if that
    # fails, something is wrong.  It's possible that a transitory file
    # containing a pod would get here, so if the file no longer exists just
    # return undef.
    return unless -e $filename;
    die "Can't open '$filename': $!\n";
}

my $digest = Digest->new($digest_type);

# This is used as a callback from File::Find::find(), which always constructs
# pathnames using Unix separators
sub is_pod_file {
    # If $_ is a pod file, add it to the lists and do other prep work.

    if (-d) {
        # Don't look at files in directories that are for tests, nor those
        # beginning with a dot, nor those in the directory where Windows
        # builds generate HTML from other POD sources.
        if (m!/t\z! || m!/\.! || m!^./win32/html\z!) {
            $File::Find::prune = 1;
        }
        return;
    }

    return unless -r && -s;    # Can't check it if can't read it; no need to
                               # check if 0 length
    return unless -f || -l;    # Weird file types won't be pods

    my ($leaf) = m!([^/]+)\z!;
    if (m!/\.!                 # No hidden Unix files
        || $leaf =~ $non_pods) {
        note("Not considering $_") if DEBUG;
        return;
    }

    my $filename = $File::Find::name;

    # $filename is relative, like './path'.  Strip that initial part away.
    $filename =~ s!^\./!! or die 'Unexpected pathname "$filename"';

    return if $excluded_files{canonicalize($filename)};

    my $contents = do {
        local $/;
        my $candidate;
        if (! open $candidate, '<:bytes', $_) {

            # If a transitory file was found earlier, the open could fail
            # legitimately and we just skip the file; also skip it if it is a
            # broken symbolic link, as it is probably just a build problem;
            # certainly not a file that we would want to check the pod of.
            # Otherwise fail it here and no reason to process it further.
            # (But the test count will be off too)
            ok(0, "Can't open '$filename': $!")
                                            if -r $filename && ! -l $filename;
            return;
        }
        <$candidate>;
    };

    # If the file is a .pm or .pod, having any initial '=' on a line is
    # grounds for testing it.  Otherwise, require a head1 NAME line to
    # consider it as a potential pod
    if ($filename =~ /\.(?:pm|pod)/) {
        return unless $contents =~ /^=/m;
    } else {
        return unless $contents =~ /^=head1 +NAME/m;
    }

    # Here, we know that the file is a pod.  Add it to the list of files
    # to check and create a checker object for it.

    push @files, $filename;
    my $checker = My::Pod::Checker->new($filename);
    $filename_to_checker{$filename} = $checker;

    # In order to detect duplicate pods and only analyze them once, we
    # compute checksums for the file, so don't have to do an exact
    # compare.  Note that if the pod is just part of the file, the
    # checksums can differ for the same pod.  That special case is handled
    # later, since if the checksums of the whole file are the same, that
    # case won't even come up.  We don't need the checksums for files that
    # we parse only if there is a link to its interior, but we do need its
    # NAME, which is also retrieved in the code below.

    if ($filename =~ / (?: ^(cpan|lib|ext|dist)\/ )
                        | $only_for_interior_links_re
                    /x)
    {
        my $byte_contents = $contents;
        utf8::encode($byte_contents);
        $digest->add($byte_contents);   # Doesn't handle Unicode
        $digests{$filename} = $digest->digest;

        # lib files aren't analyzed if they are duplicates of files copied
        # there from some other directory.  But to determine this, we need
        # to know their NAMEs.  We might as well find the NAME now while
        # the file is open.  Similarly, cpan files aren't analyzed unless
        # we're analyzing all of them, or this particular file is linked
        # to by a file we are analyzing, and thus we will want to verify
        # that the target exists in it.  We need to know at least the NAME
        # to see if it's worth analyzing, or so we can determine if a lib
        # file is a copy of a cpan one.
        if ($filename =~ m{ (?: ^ (?: cpan | lib ) / )
                            | $only_for_interior_links_re
                            }x) {
            if ($contents =~ /^=head1 +NAME.*/mg) {
                # The NAME is the first non-spaces on the line up to a
                # comma, dash or end of line.  Otherwise, it's invalid and
                # this pod doesn't have a legal name that we're smart
                # enough to find currently.  But the  parser will later
                # find it if it thinks there is a legal name, and set the
                # name
                if ($contents =~ /\G    # continue from the line after =head1
                                  \s*   # ignore any empty lines

                                  # ignore =for paragraphs followed by empty
                                  # lines
                                  (?: ^ =for .*? \n (?: [^\s]*? \n )* \s* )*

                                  ^ \s* ( \S+?) \s* (?: [,-] | $ )/mx) {
                    my $name = $1;
                    $checker->name($name);
                    $id_to_checker{$name} = $checker
                        if $filename =~ m{^cpan/};
                }
            }
            elsif ($filename =~ m{^cpan/}) {
                $id_to_checker{$digests{$filename}} = $checker;
            }
        }
    }

    return;
} # End of is_pod_file()

# Start of real code that isn't processing the command line (except the
# db is read in above, as is processing of the --add-link option).
# Here, @files contains list of files on the command line.  If have any of
# these, unconditionally test them, and show all the errors, even the known
# ones, and, since not testing other pods, don't do cross-pod link tests.
# (Could add extra code to do cross-pod tests for the ones in the list.)

if ($has_input_files) {
    undef %known_problems;
    $do_upstream_cpan = $do_deltas = 1;  # In case one of the inputs is one
                                         # of these types
}
else { # No input files -- go find all the possibilities.
    if ($regen) {
        $copy_fh = open_new($known_issues);
        note("Regenerating $known_issues, please be patient...");
        print $copy_fh $HEADER;
    }

    # Move to the directory above us, but have to adjust @INC to account for
    # that.
    s{^\.\./lib$}{lib} for @INC;
    chdir File::Spec->updir;

    # And look in this directory and all its subdirectories
    find( {wanted => \&is_pod_file, no_chdir => 1}, '.');

    # Add ourselves to the test
    push @files, "t/porting/podcheck.t";
}

# Now we know how many tests there will be.
plan (tests => scalar @files) if ! $regen;


# Sort file names so we get consistent results, and to put cpan last,
# preceded by the ones that we don't generally parse.  This is because both
# these classes are generally parsed only if there is a link to the interior
# of them, and we have to parse all others first to guarantee that they don't
# have such a link. 'lib' files come just before these, as some of these are
# duplicates of others.  We already have figured this out when gathering the
# data as a special case for all such files, but this, while unnecessary,
# puts the derived file last in the output.  'readme' files come before those,
# as those also could be duplicates of others, which are considered the
# primary ones.  These currently aren't figured out when gathering data, so
# are done here.
@files = sort { if ($a =~ /^cpan/) {
                   return 1 if $b !~ /^cpan/;
                   return lc $a cmp lc $b;
               }
               elsif ($b =~ /^cpan/) {
                   return -1;
               }
               elsif ($a =~ /$only_for_interior_links_re/) {
                   return 1 if $b !~ /$only_for_interior_links_re/;
                   return lc $a cmp lc $b;
               }
               elsif ($b =~ /$only_for_interior_links_re/) {
                   return -1;
               }
               elsif ($a =~ /^lib/) {
                   return 1 if $b !~ /^lib/;
                   return lc $a cmp lc $b;
               }
               elsif ($b =~ /^lib/) {
                   return -1;
               } elsif ($a =~ /\breadme\b/i) {
                   return 1 if $b !~ /\breadme\b/i;
                   return lc $a cmp lc $b;
               }
               elsif ($b =~ /\breadme\b/i) {
                   return -1;
               }
               else {
                   return lc $a cmp lc $b;
               }
           }
           @files;

# Now go through all the files and parse them
FILE:
foreach my $filename (@files) {
    my $parsed = 0;
    note("parsing $filename") if DEBUG;

    # We may have already figured out some things in the process of generating
    # the file list.  If so, we have a $checker object already.  But if not,
    # generate one now.
    my $checker = $filename_to_checker{$filename};
    if (! $checker) {
        $checker = My::Pod::Checker->new($filename);
        $filename_to_checker{$filename} = $checker;
    }

    # We have set the name in the checker object if there is a possibility
    # that no further parsing is necessary, but otherwise do the parsing now.
    if (! $checker->name) {
        if (! $checker->parse_from_file($filename, undef)) {
            $checker->set_skip("$filename is transitory");
            next FILE;
        }
        $parsed = 1;
    }

    if ($checker->num_errors() < 0) {   # Returns negative if not a pod
        $checker->set_skip("$filename is not a pod");
    }
    else {

        # Here, is a pod.  See if it is one that has already been tested,
        # or should be tested under another directory.  Use either its NAME
        # if it has one, or a checksum if not.
        my $name = $checker->name;
        my $id;

        if ($name) {
            $id = $name;
        }
        else {
            my $digest = Digest->new($digest_type);
            my $contents = extract_pod($filename);

            # If the return is undef, it means that $filename was a transitory
            # file; skip it.
            next FILE unless defined $contents;
            my $byte_contents = $contents;
            utf8::encode($byte_contents);
            $digest->add($byte_contents);   # Doesn't handle Unicode
            $id = $digest->digest;
        }

        # If there is a match for this pod with something that we've already
        # processed, don't process it, and output why.
        my $prior_checker;
        if (defined ($prior_checker = $id_to_checker{$id})
            && $prior_checker != $checker)  # Could have defined the checker
                                            # earlier without pursuing it
        {

            # If the pods are identical, then it's just a copy, and isn't an
            # error.  First use the checksums we have already computed to see
            # if the entire files are identical, which means that the pods are
            # identical too.
            my $prior_filename = $prior_checker->get_filename;
            my $same = (! $name
                        || ($digests{$prior_filename}
                            && $digests{$filename}
                            && $digests{$prior_filename} eq $digests{$filename}));

            # If they differ, it could be that the files differ for some
            # reason, but the pods they contain are identical.  Extract the
            # pods and do the comparisons on just those.
            if (! $same && $name) {
                my $contents = extract_pod($filename);

                # If return is <undef>, it means that $filename no longer
                # exists.  This means it was a transitory file, and should not
                # be tested.
                next FILE unless defined $contents;

                my $prior_contents = extract_pod($prior_filename);

                # If return is <undef>, it means that $prior_filename no
                # longer exists.  This means it was a transitory file, and
                # should not have been tested, but we already did process it.
                # What we should do now is to back-out its records, and
                # process $filename in its stead.  But backing out is not so
                # simple, and so I'm (khw) skipping that unless and until
                # experience shows that it is needed.  We do go process
                # $filename, and there are potential false positive conflicts
                # with the transitory $prior_contents, and rerunning the test
                # should cause it to succeed.
                goto process_this_pod unless defined $prior_contents;

                $same = $prior_contents eq $contents;
            }

            use File::Basename 'basename';
            if ($same) {
                $checker->set_skip("The pod of $filename is a duplicate of "
                                    . "the pod for $prior_filename");
            } elsif ($prior_filename =~ /\breadme\b/i) {
                $checker->set_skip("$prior_filename is a README apparently for $filename");
            } elsif ($filename =~ /\breadme\b/i) {
                $checker->set_skip("$filename is a README apparently for $prior_filename");
            } elsif (! $do_upstream_cpan
                     && $filename =~ /^cpan/
                     && $prior_filename =~ /^cpan/)
            {
                $checker->set_skip("CPAN is upstream for $filename");
            } elsif ( $filename =~ /^utils/ or $prior_filename =~ /^utils/ ) {
                $checker->set_skip("$filename copy is in utils/");
            } elsif ($prior_filename =~ /^(?:cpan|ext|dist)/
                     && $filename !~ /^(?:cpan|ext|dist)/
                     && basename($prior_filename) eq basename($filename))
            {
                $checker->set_skip("$filename: Need to run make?");
            } else { # Here have two pods with identical names that differ
                $prior_checker->poderror(
                        { -msg => $duplicate_name,
                            -line => "???",
                            parameter => "'$filename' also has NAME '$name'"
                        });
                $checker->poderror(
                    { -msg => $duplicate_name,
                        -line => "???",
                        parameter => "'$prior_filename' also has NAME '$name'"
                    });

                # Changing the names helps later.
                $prior_checker->name("$name version arbitrarily numbered 1");
                $checker->name("$name version arbitrarily numbered 2");
            }

            # In any event, don't process this pod that has the same name as
            # another.
            next FILE;
        }

    process_this_pod:

        # A unique pod.
        $id_to_checker{$id} = $checker;

        my $parsed_for_links = ", but parsed for its interior links";
        if ((! $do_upstream_cpan && $filename =~ /^cpan/)
             || $filename =~ $only_for_interior_links_re)
        {
            if ($filename =~ /^cpan/) {
                $checker->set_skip("CPAN is upstream for $filename");
            }
            elsif ($filename =~ /perl\d+delta/) {
                if (! $do_deltas) {
                    $checker->set_skip("$filename is a stable perldelta");
                }
            }
            elsif ($filename =~ /perltoc/) {
                $checker->set_skip("$filename dependent on component pods");
            }
            else {
                croak("Unexpected file '$filename' encountered that has parsing for interior-linking only");
            }

            if ($name && $has_referred_to_node{$name}) {
                $checker->set_skip($checker->get_skip() . $parsed_for_links);
            }
        }

        # Need a name in order to process it, because not meaningful
        # otherwise, and also can't test links to this without a name.
        if (!defined $name) {
            $checker->poderror( { -msg => $no_name,
                                  -line => '???'
                                });
            next FILE;
        }

        # For skipped files, just get its NAME
        my $skip;
        if (($skip = $checker->get_skip()) && $skip !~ /$parsed_for_links/)
        {
            $checker->node($name) if $name;
        }
        elsif (! $parsed) {
            if (! $checker->parse_from_file($filename, undef)) {
                $checker->set_skip("$filename is transitory");
                next FILE;
            }
        }

        # Go through everything in the file that could be an anchor that
        # could be a link target.  Count how many there are of the same name.
        foreach my $node ($checker->linkable_nodes) {
            next FILE if ! $node;        # Can be empty is like '=item *'
            $nodes{$name}{$node}++;

            # Experiments have shown that cpan search can figure out the
            # target of a link even if the exact wording is incorrect, as long
            # as the first word is.  This happens frequently in perlfunc.pod,
            # where the link will be just to the function, but the target
            # entry also includes parameters to the function.
            my $first_word = $node;
            if ($first_word =~ s/^(\S+)\s+\S.*/$1/) {
                $nodes_first_word{$name}{$first_word} = $node;
            }
        }
        $filename_to_pod{$filename} = $name;
    }
}

# Here, all files have been parsed, and all links and link targets are stored.
# Now go through the files again and see which don't have matches.
if (! $has_input_files) {
    foreach my $filename (@files) {
        next if $filename_to_checker{$filename}->get_skip;

        my $checker = $filename_to_checker{$filename};
        foreach my $link ($checker->hyperlinks()) {
            my $linked_to_page = $link->page;
            next unless $linked_to_page;   # intra-file checks are handled by std
                                           # Pod::Checker
            # Currently, we assume all external links are valid
            next if $link->type eq 'url';

            # Initialize the potential message.
            my %problem = ( -msg => $broken_link,
                            -line => $link->line,
                            parameter => "to \"$linked_to_page\"",
                        );

            # See if we have found the linked-to_file in our parse
            if (exists $nodes{$linked_to_page}) {
                my $node = $link->node;

                # If link is only to the page-level, already have it
                next if ! $node;

                # If link is to a node that exists in the file, is ok
                if ($nodes{$linked_to_page}{$node}) {

                    # But if the page has multiple targets with the same name,
                    # it's ambiguous which one this should be to.
                    if ($nodes{$linked_to_page}{$node} > 1) {
                        $problem{-msg} = $multiple_targets;
                        $problem{parameter} = "in $linked_to_page that $node could be pointing to";
                        $checker->poderror(\%problem);
                    }
                } elsif (! $nodes_first_word{$linked_to_page}{$node}) {

                    # Here the link target was not found, either exactly or to
                    # the first word.  Is an error.
                    $problem{parameter} =~ s,"$,/$node",;
                    $checker->poderror(\%problem);
                }

            } # Linked-to-file not in parse; maybe is in exception list
            elsif (! exists $valid_modules{$link->page}) {

                # Here, is a link to a target that we can't find.  Check if
                # there is an internal link on the page with the target name.
                # If so, it could be that they just forgot the initial '/'
                # But perldelta is handled specially: only do this if the
                # broken link isn't one of the known bad ones (that are
                # placemarkers and should be removed for the final)
                my $NAME = $filename_to_pod{$filename};
                if (! defined $NAME) {
                    $checker->poderror(\%problem);
                }
                else {
                    if ($nodes{$NAME}{$linked_to_page}) {
                        $problem{-msg} =  $broken_internal_link;
                    }
                    $checker->poderror(\%problem);
                }
            }
        }
    }
}

# If regenerating the data file, start with the modules for which we don't
# check targets.  If you change the sort order, you need to run --regen before
# committing so that future commits that do run regen don't show irrelevant
# changes.
if ($regen) {
    foreach (sort { lc $a cmp lc $b } keys %valid_modules) {
        my_safer_print($copy_fh, $_, "\n");
    }
}

# Now ready to output the messages.
foreach my $filename (@files) {
    my $canonical = canonicalize($filename);
    SKIP: {
        my $skip = $filename_to_checker{$filename}->get_skip // "";

        if ($regen) {
            foreach my $message ( sort keys %{$problems{$filename}}) {
                my $count;

                # Preserve a negative setting.
                if ($known_problems{$canonical}{$message}
                    && $known_problems{$canonical}{$message} < 0)
                {
                    $count = $known_problems{$canonical}{$message};
                }
                else {
                    $count = @{$problems{$filename}{$message}};
                }
                my_safer_print($copy_fh, $canonical . "\t$message\t$count\n");
            }
            next;
        }

        skip($skip, 1) if $skip;
        my @diagnostics;
        my $thankful_diagnostics = 0;
        my $indent = '  ';

        my $total_known = 0;
        foreach my $message ( sort keys %{$problems{$filename}}) {
            $known_problems{$canonical}{$message} = 0
                                    if ! $known_problems{$canonical}{$message};
            my $diagnostic = "";
            my $problem_count = scalar @{$problems{$filename}{$message}};
            $total_known += $problem_count;
            next if $known_problems{$canonical}{$message} < 0;

            # If we have new problems not previously known, we output all of
            # such problems, as we can't know which are really new and which
            # not
            if ($problem_count > $known_problems{$canonical}{$message}) {

                # Here we are about to output all the messages for this type,
                # subtract back this number we previously added in.
                $total_known -= $problem_count;

                $diagnostic .= $indent . qq{"$message"};
                if ($problem_count > 2) {
                    $diagnostic .= "  ($problem_count occurrences,"
			. " expected $known_problems{$canonical}{$message})";
                }
                foreach my $problem (@{$problems{$filename}{$message}}) {
                    $diagnostic .= " " if $problem_count == 1;
                    $diagnostic .= "\n$indent$indent";
                    $diagnostic .= "$problem->{parameter}" if $problem->{parameter};
                    $diagnostic .= " near line $problem->{-line} of "
                                   . $filename;
                    $diagnostic .= " $problem->{comment}" if $problem->{comment};
                }
                $diagnostic .= "\n";
                $files_with_unknown_issues{$filename} = 1;
            } elsif ($problem_count < $known_problems{$canonical}{$message}) {
               $diagnostic = output_thanks($filename, $known_problems{$canonical}{$message}, $problem_count, $message);
               $thankful_diagnostics++;
            }
            push @diagnostics, $diagnostic if $diagnostic;
        }

        # The above loop has output messages where there are current potential
        # issues.  But it misses where there were some that have been entirely
        # fixed.  For those, we need to look through the old issues
        foreach my $message ( sort keys %{$known_problems{$canonical}}) {
            next if $problems{$filename}{$message};
            next if ! $known_problems{$canonical}{$message};
            next if $known_problems{$canonical}{$message} < 0; # Preserve negs

            next if !$pedantic and $message =~
                /^(?:\Q$line_length\E|\Q$C_not_linked\E|\Q$C_with_slash\E)/;

            my $diagnostic = output_thanks($filename, $known_problems{$canonical}{$message}, 0, $message);
            push @diagnostics, $diagnostic if $diagnostic;
            $thankful_diagnostics++ if $diagnostic;
        }

        my $output = "POD of $filename";
        $output .= ", excluding $total_known not shown known potential problems"
                                                                if $total_known;
        if (@diagnostics && @diagnostics == $thankful_diagnostics) {
            # Output fixed issues as passing to-do tests, so they do not
            # cause failures, but t/harness still flags them.
            $output .= " # TODO"
        }
        ok(@diagnostics == $thankful_diagnostics, $output);
        if (@diagnostics) {
            diag(join "", @diagnostics,
            "See end of this test output for your options on silencing this");
        }

        delete $known_problems{$canonical};
    }
}

if (! $regen
    && ! ok (keys %known_problems == 0, "The known problems database ($data_dir/known_pod_issues.dat) includes no references to non-existent files"))
{
    note("The following files were not found: "
         . join ", ", sort keys %known_problems);
    note("They will automatically be removed from the db the next time");
    note("  cd t; ./perl -I../lib porting/podcheck.t --regen");
    note("is run");
}

my $how_to = <<EOF;
   run this test script by hand, using the following formula (on
   Un*x-like machines):
        cd t
        ./perl -I../lib porting/podcheck.t --regen
EOF

if (%files_with_unknown_issues) {
    my $were_count_files = scalar keys %files_with_unknown_issues;
    $were_count_files = ($were_count_files == 1)
                        ? "was $were_count_files file"
                        : "were $were_count_files files";
    my $message = <<EOF;

HOW TO GET ${\__FILE__} TO PASS

There $were_count_files that had new potential problems identified.
Some of them may be real, and some of them may be false positives because
this program isn't as smart as it likes to think it is.  You can teach this
program to ignore the issues it has identified, and hence pass, by doing the
following:

1) If a problem is about a link to an unknown module or man page that
   you know exists, re-run the command something like:
      ./perl -I../lib porting/podcheck.t --add-link { MODULE | man_page ... }
   (MODULEs should look like Foo::Bar, and man_pages should look like
   bar(3c); don't do this for a module or man page that you aren't sure
   about; instead treat as another type of issue and follow the
   instructions below.)

2) For other issues, decide if each should be fixed now or not.  Fix the
   ones you decided to, and rerun this test to verify that the fixes
   worked.

3) If there remain false positive or problems that you don't plan to fix right
   now,
$how_to
   That should cause all current potential problems to be accepted by
   the program, so that the next time it runs, they won't be flagged.
EOF
    if (%files_with_fixes) {
        $message .= "   This step will also take care of the files that have fixes in them\n";
    }

    $message .= <<EOF;
   For a few files, such as perltoc, certain issues will always be
   expected, and more of the same will be added over time.  For those,
   before you do the regen, you can edit
   $known_issues
   and find the entry for the module's file and specific error message,
   and change the count of known potential problems to -1.
EOF

    diag($message);
} elsif (%files_with_fixes) {
    diag(<<EOF
To teach this test script that the potential problems have been fixed,
$how_to
EOF
    );
}

if ($regen) {
    chdir $original_t_dir || die "Can't change directories to $original_t_dir";
    close_and_rename($copy_fh);
}

1;
