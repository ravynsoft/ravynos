#!/usr/bin/perl
#
# A tool for analysing the performance of the code snippets found in
# t/perf/benchmarks or similar


=head1 NAME

bench.pl - Compare the performance of perl code snippets across multiple
perls.

=head1 SYNOPSIS

    # Basic: run the tests in t/perf/benchmarks against two or
    # more perls

    bench.pl [options] perlA[=labelA] perlB[=labelB] ...

    # run the tests against the same perl twice, with varying options

    bench.pl [options] perlA=bigint --args='-Mbigint' perlA=plain

    # Run bench on blead, saving results to file; then modify the blead
    # binary, and benchmark again, comparing against the saved results

    bench.pl [options] --write=blead.time ./perl=blead
    # ... hack hack hack, updating ./perl ...
    bench.pl --read=blead.time ./perl=hacked

    # You can also combine --read with --write and new benchmark runs

    bench.pl --read=blead.time --write=last.time -- ./perl=hacked

=head1 DESCRIPTION

By default, F<bench.pl> will run code snippets found in
F<t/perf/benchmarks> (or similar) under cachegrind, in order to calculate
how many instruction reads, data writes, branches, cache misses, etc. that
one execution of the snippet uses. Usually it will run them against two or
more perl executables and show how much each test has gotten better or
worse.

It is modelled on the F<perlbench> tool, but since it measures instruction
reads etc., rather than timings, it is much more precise and reproducible.
It is also considerably faster, and is capable of running tests in
parallel (with C<-j>). Rather than  displaying a single relative
percentage per test/perl combination, it displays values for 13 different
measurements, such as instruction reads, conditional branch misses etc.

There are options to write the raw data to a file, and to read it back.
This means that you can view the same run data in different views with
different selection and sort options. You can also use this mechanism
to save the results of timing one perl, and then read it back while timing
a modification, so that you don't have rerun the same tests on the same
perl over and over, or have two perl executables built at the same time.

The optional C<=label> after each perl executable is used in the display
output. If you are doing a two step benchmark then you should provide
a label for at least the "base" perl. If a label isn't specified, it
defaults to the name of the perl executable. Labels must be unique across
all current executables, plus any previous ones obtained via --read.

In its most general form, the specification of a perl executable is:

    path/perl=+mylabel --args='-foo -bar' --args='-baz' \
                       --env='A=a' --env='B=b'

This defines how to run the executable F<path/perl>. It has a label,
which due to the C<+>, is appended to the binary name to give a label of
C<path/perl=+mylabel> (without the C<+>, the label would be just
C<mylabel>).

It can be optionally followed by one or more C<--args> or C<--env>
switches, which specify extra command line arguments or environment
variables to use when invoking that executable. Each C<--env> switch
should be of the form C<--env=VARIABLE=value>. Any C<--arg> values are
concatenated to the eventual command line, along with the global
C<--perlargs> value if any. The above would cause a system() call looking
something like:

    PERL_HASH_SEED=0 A=a B=b valgrind --tool=cachegrind \
        path/perl -foo -bar -baz ....

=head1 OPTIONS

=head2 General options

=over 4

=item *

--action=I<foo>

What action to perform. The default is  I<grind>, which runs the benchmarks
using I<cachegrind> as the back end. The only other action at the moment is
I<selftest>, which runs some basic sanity checks and produces TAP output.

=item *

--debug

Enable debugging output.

=item *

---help

Display basic usage information.

=item *

-v
--verbose

Display progress information.

=back

=head2 Test selection options

=over 4

=item *

--tests=I<FOO>

Specify a subset of tests to run (or in the case of C<--read>, to read).
It may be either a comma-separated list of test names, or a regular
expression. For example

    --tests=expr::assign::scalar_lex,expr::assign::2list_lex
    --tests=/^expr::/


=back

=head2 Input options

=over 4


=item *

-r I<file>
--read=I<file>

Read in saved data from a previous C<--write> run from the specified file.
If C<--tests> is present too, then only tests matching those conditions
are read from the file.

C<--read> may be specified multiple times, in which case the results
across all files are aggregated. The list of test names from each file
(after filtering by C<--tests>) must be identical across all files.

This list of tests is used instead of that obtained from the normal
benchmark file (or C<--benchfile>) for any benchmarks that are run.

The perl labels must be unique across all read in test results.

Requires C<JSON::PP> to be available.

=back

=head2 Benchmarking options

Benchmarks will be run for all perls specified on the command line.
These options can be used to modify the benchmarking behavior:

=over 4

=item *

--autolabel

Generate a unique label for every executable which doesn't have an
explicit C<=label>. Works by stripping out common prefixes and suffixes
from the executable names, then for any non-unique names, appending
C<-0>, C<-1>, etc. text directly surrounding the unique part which look
like version numbers (i.e. which match C</[0-9\.]+/>) aren't stripped.
For example,

    perl-5.20.0-threaded  perl-5.22.0-threaded  perl-5.24.0-threaded

stripped to unique parts would be:

    20  22  24

but is actually only stripped down to:

    5.20.0  5.22.0  5.24.0

If the final results are plain integers, they are prefixed with "p"
to avoid looking like column numbers to switches like C<--norm=2>.


=item *

--benchfile=I<foo>

The path of the file which contains the benchmarks (F<t/perf/benchmarks>
by default).

=item *

--grindargs=I<foo>

Optional command-line arguments to pass to all cachegrind invocations.

=item *

-j I<N>
--jobs=I<N>

Run I<N> jobs in parallel (default 1). This determines how many cachegrind
process will run at a time, and should generally be set to the number
of CPUs available.

=item *

--perlargs=I<foo>

Optional command-line arguments to pass to every perl executable.  This
may optionaly be combined with C<--args> switches following individual
perls. For example:

    bench.pl --perlargs='-Ilib -It/lib' .... \
        perlA --args='-Mstrict' \
        perlB --args='-Mwarnings'

would cause the invocations

    perlA -Ilib -It/lib -Mstrict
    perlB -Ilib -It/lib -Mwarnings

=back

=head2 Output options

Any results accumulated via --read or by running benchmarks can be output
in any or all of these three ways:

=over 4

=item *

-w I<file>
--write=I<file>

Save the raw data to the specified file. It can be read back later with
C<--read>. If combined with C<--read> then the output file will be
the merge of the file read and any additional perls added on the command
line.

Requires C<JSON::PP> to be available.

=item *

--bisect=I<field,minval,maxval>

Exit with a zero status if the named field is in the specified range;
exit with 1 otherwise. It will complain if more than one test or perl has
been specified. It is intended to be called as part of a bisect run, to
determine when something changed.  For example,

    bench.pl -j 8 --tests=foo --bisect=Ir,100,105 --perlargs=-Ilib \
        ./miniperl

might be called from bisect to find when the number of instruction reads
for test I<foo> falls outside the range 100..105.

=item *

--show

Display the results to stdout in human-readable form.  This is enabled by
default, except with --write and --bisect. The following sub-options alter
how --show behaves.

=over 4

=item *

--average

Only display the overall average, rather than the results for each
individual test.

=item *

--compact=I<perl>

Display the results for a single perl executable in a compact form.
Which perl to display is specified in the same manner as C<--norm>.

=item *

--fields=I<a,b,c>

Display only the specified fields; for example,

    --fields=Ir,Ir_m,Ir_mm

If only one field is selected, the output is in more compact form.

=item *

--norm=I<foo>

Specify which perl column in the output to treat as the 100% norm.
It may be:

=over

* a column number (0..N-1),

* a negative column number (-1..-N) which counts from the right (so -1 is
the right-most column),

* or a perl executable name,

* or a perl executable label.

=back

It defaults to the leftmost column.

=item *

--raw

Display raw data counts rather than percentages in the outputs. This
allows you to see the exact number of intruction reads, branch misses etc.
for each test/perl combination. It also causes the C<AVERAGE> display
per field to be calculated based on the average of each tests's count
rather than average of each percentage. This means that tests with very
high counts will dominate.

=item *

--sort=I<field:perl>

Order the tests in the output based on the value of I<field> in the
column I<perl>. The I<perl> value is as per C<--norm>. For example

    bench.pl --sort=Dw:perl-5.20.0 \
        perl-5.16.0 perl-5.18.0 perl-5.20.0

=back

=back

=cut



use 5.010000;
use warnings;
use strict;
use Getopt::Long qw(:config no_auto_abbrev require_order);
use IPC::Open2 ();
use IO::Select;
use IO::File;
use POSIX ":sys_wait_h";

# The version of the file format used to save data. We refuse to process
# the file if the integer component differs.

my $FORMAT_VERSION = 1.0;

# The fields we know about

my %VALID_FIELDS = map { $_ => 1 }
    qw(Ir Ir_m1 Ir_mm Dr Dr_m1 Dr_mm Dw Dw_m1 Dw_mm COND COND_m IND IND_m);

sub usage {
    die <<EOF;
Usage: $0 [options] -- perl[=label] ...

General options:

  --action=foo       What action to perform [default: grind]:
                        grind      run the code under cachegrind
                        selftest   perform a selftest; produce TAP output
  --debug            Enable verbose debugging output.
  --help             Display this help.
  -v|--verbose       Display progress information.


Selection:

  --tests=FOO        Select only the specified tests for reading, benchmarking
                       and display.  FOO may be either a list of tests or
                       a pattern: 'foo,bar,baz' or '/regex/';
                       [default: all tests].

Input:

  -r|--read=file     Read in previously saved data from the specified file.
                        May be repeated, and be used together with new
                        benchmarking to create combined results.

Benchmarking:
  Benchmarks will be run for any perl specified on the command line.
  These options can be used to modify the benchmarking behavior:

  --autolabel        generate labels for any executables without one
  --benchfile=foo    File containing the benchmarks.
                         [default: t/perf/benchmarks].
  --grindargs=foo    Optional command-line args to pass to cachegrind.
  -j|--jobs=N        Run N jobs in parallel [default 1].
  --perlargs=foo     Optional command-line args to pass to each perl to run.

Output:
  Any results accumulated via --read or running benchmarks can be output
  in any or all of these three ways:

  -w|--write=file    Save the raw data to the specified file (may be read
                       back later with --read).

  --bisect=f,min,max Exit with a zero status if the named field f is in
                       the specified min..max range; exit 1 otherwise.
                       Produces no other output. Only legal if a single
                       benchmark test has been specified.

  --show             Display the results to stdout in human-readable form.
                       This is enabled by default, except with --write and
                       --bisect. The following sub-options alter how
                       --show behaves.

    --average          Only display average, not individual test results.
    --compact=perl     Display the results of a single perl in compact form.
                       Which perl specified like --norm
    --fields=a,b,c     Display only the specified fields (e.g. Ir,Ir_m,Ir_mm).
    --norm=perl        Which perl column to treat as 100%; may be a column
                         number (0..N-1) or a perl executable name or label;
                         [default: 0].
    --raw              Display raw data counts rather than percentages.
    --sort=field:perl  Sort the tests based on the value of 'field' in the
                       column 'perl'. The perl value is as per --norm.


The command line ends with one or more specified perl executables,
which will be searched for in the current \$PATH. Each binary name may
have an optional =LABEL appended, which will be used rather than the
executable name in output. The labels must be unique across all current
executables and previous runs obtained via --read. Each executable may
optionally be succeeded by --args= and --env= to specify per-executable
arguments and environmenbt variables:

    perl-5.24.0=strict --args='-Mwarnings -Mstrict' --env='FOO=foo' \
    perl-5.24.0=plain
EOF
}

my %OPTS = (
    action    => 'grind',
    average   => 0,
    benchfile => undef,
    bisect    => undef,
    compact   => undef,
    debug     => 0,
    grindargs => '',
    fields    => undef,
    jobs      => 1,
    norm      => 0,
    perlargs  => '',
    raw       => 0,
    read      => undef,
    show      => undef,
    sort      => undef,
    tests     => undef,
    verbose   => 0,
    write     => undef,
);


# process command-line args and call top-level action

{
    GetOptions(
        'action=s'    => \$OPTS{action},
        'average'     => \$OPTS{average},
        'autolabel'   => \$OPTS{autolabel},
        'benchfile=s' => \$OPTS{benchfile},
        'bisect=s'    => \$OPTS{bisect},
        'compact=s'   => \$OPTS{compact},
        'debug'       => \$OPTS{debug},
        'grindargs=s' => \$OPTS{grindargs},
        'help|h'      => \$OPTS{help},
        'fields=s'    => \$OPTS{fields},
        'jobs|j=i'    => \$OPTS{jobs},
        'norm=s'      => \$OPTS{norm},
        'perlargs=s'  => \$OPTS{perlargs},
        'raw'         => \$OPTS{raw},
        'read|r=s@'   => \$OPTS{read},
        'show'        => \$OPTS{show},
        'sort=s'      => \$OPTS{sort},
        'tests=s'     => \$OPTS{tests},
        'v|verbose'   => \$OPTS{verbose},
        'write|w=s'   => \$OPTS{write},
    ) or die "Use the -h option for usage information.\n";

    usage if $OPTS{help};


    if (defined $OPTS{read} or defined $OPTS{write}) {
        # fail early if it's not present
        require JSON::PP;
    }

    if (defined $OPTS{fields}) {
        my @f = split /,/, $OPTS{fields};
        for (@f) {
            die "Error: --fields: unknown field '$_'\n"
                unless $VALID_FIELDS{$_};
        }
        my %f = map { $_ => 1 } @f;
        $OPTS{fields} = \%f;
    }

    my %valid_actions = qw(grind 1 selftest 1);
    unless ($valid_actions{$OPTS{action}}) {
        die "Error: unrecognised action '$OPTS{action}'\n"
          . "must be one of: " . join(', ', sort keys %valid_actions)."\n";
    }

    if (defined $OPTS{sort}) {
        my @s = split /:/, $OPTS{sort};
        if (@s != 2) {
            die "Error: --sort argument should be of the form field:perl: "
              . "'$OPTS{sort}'\n";
        }
        my ($field, $perl) = @s;
        die "Error: --sort: unknown field '$field'\n"
            unless $VALID_FIELDS{$field};
        # the 'perl' value will be validated later, after we have processed
        # the perls
        $OPTS{'sort-field'} = $field;
        $OPTS{'sort-perl'}  = $perl;
    }

    # show is the default output action
    $OPTS{show} = 1 unless $OPTS{write} || $OPTS{bisect};

    if ($OPTS{action} eq 'grind') {
        do_grind(\@ARGV);
    }
    elsif ($OPTS{action} eq 'selftest') {
        if (@ARGV) {
            die "Error: no perl executables may be specified with selftest\n"
        }
        do_selftest();
    }
}
exit 0;


# Given a hash ref keyed by test names, filter it by deleting unwanted
# tests, based on $OPTS{tests}.

sub filter_tests {
    my ($tests) = @_;

    my $opt = $OPTS{tests};
    return unless defined $opt;

    my @tests;

    if ($opt =~ m{^/}) {
        $opt =~ s{^/(.+)/$}{$1}
            or die "Error: --tests regex must be of the form /.../\n";
        for (keys %$tests) {
            delete $tests->{$_} unless /$opt/;
        }
    }
    else {
        my %t;
        for (split /,/, $opt) {
            $t{$_} = 1;
            next if exists $tests->{$_};

            my $e = "Error: no such test found: '$_'\n";
            if ($OPTS{verbose}) {
                $e .= "Valid test names are:\n";
                $e .= "  $_\n" for sort keys %$tests;
            }
            else {
                $e .= "Re-run with --verbose for a list of valid tests.\n";
            }
            die $e;
        }
        for (keys %$tests) {
            delete $tests->{$_} unless exists $t{$_};
        }
    }
    die "Error: no tests to run\n" unless %$tests;
}


# Read in the test file, and filter out any tests excluded by $OPTS{tests}
# return a hash ref { testname => { test }, ... }
# and an array ref of the original test names order,

sub read_tests_file {
    my ($file) = @_;

    my $ta;
    {
        local @INC = ('.');
        $ta = do $file;
    }
    unless ($ta) {
        die "Error: can't load '$file': code didn't return a true value\n"
                if defined $ta;
        die "Error: can't parse '$file':\n$@\n" if $@;
        die "Error: can't read '$file': $!\n";
    }

    # validate and process each test

    {
        my %valid = map { $_ => 1 } qw(desc setup code pre post compile);
        my @tests = @$ta;
        if (!@tests || @tests % 2 != 0) {
            die "Error: '$file' does not contain evenly paired test names and hashes\n";
        }
        while (@tests) {
            my $name = shift @tests;
            my $hash = shift @tests;

            unless ($name =~ /^[a-zA-Z]\w*(::\w+)*$/) {
                die "Error: '$file': invalid test name: '$name'\n";
            }

            for (sort keys %$hash) {
                die "Error: '$file': invalid key '$_' for test '$name'\n"
                    unless exists $valid{$_};
            }

            # make description default to the code
            $hash->{desc} = $hash->{code} unless exists $hash->{desc};
        }
    }

    my @orig_order;
    for (my $i=0; $i < @$ta; $i += 2) {
        push @orig_order, $ta->[$i];
    }

    my $t = { @$ta };
    filter_tests($t);
    return $t, \@orig_order;
}


# Process the perl name/label/column argument of options like --norm and
# --sort.  Return the index of the matching perl.

sub select_a_perl {
    my ($perl, $perls, $who) = @_;
    $perls ||= [];
    my $n = @$perls;

    if ($perl =~ /^-([0-9]+)$/) {
        my $p = $1;
        die "Error: $who value $perl outside range -1..-$n\n"
                                        if $p < 1 || $p > $n;
        return $n - $p;
    }

    if ($perl =~ /^[0-9]+$/) {
        die "Error: $who value $perl outside range 0.." . $#$perls . "\n"
                                        unless $perl < $n;
        return $perl;
    }
    else {
        my @perl = grep    $perls->[$_][0] eq $perl
                        || $perls->[$_][1] eq $perl,
                        0..$#$perls;
        unless (@perl) {
            my $valid = '';
            for (@$perls) {
                $valid .= "    $_->[1]";
                $valid .= "  $_->[0]" if $_->[0] ne  $_->[1];
                $valid .= "\n";
            }
            die "Error: $who: unrecognised perl '$perl'\n"
              . "Valid perl names are:\n$valid";
        }
        die "Error: $who: ambiguous perl '$perl'\n"
                                        if @perl > 1;
        return $perl[0];
    }
}


# Validate the list of perl executables on the command line.
# The general form is
#
#      a_perl_exe[=label] [ --args='perl args'] [ --env='FOO=foo' ]
#
# Return a list of [ exe, label, {env}, 'args' ] tuples

sub process_executables_list {
    my ($read_perls, @cmd_line_args) = @_;

    my @results; # returned, each item is [ perlexe, label, {env}, 'args' ]
    my %seen_from_reads = map { $_->[1] => 1 } @$read_perls;
    my %seen;
    my @labels;

    while (@cmd_line_args) {
        my $item = shift @cmd_line_args;

        if ($item =~ /^--(.*)$/) {
            my ($switch, $val) = split /=/, $1, 2;
            die "Error: unrecognised executable switch '--$switch'\n"
                unless $switch =~  /^(args|env)$/;

            die "Error: --$switch without a preceding executable name\n"
                unless @results;

            unless (defined $val) {
                $val = shift @cmd_line_args;
                die "Error: --$switch is missing value\n"
                    unless defined $val;
            }

            if ($switch eq 'args') {
                $results[-1][3] .= " $val";
            }
            else {
                # --env
                $val =~ /^(\w+)=(.*)$/
                    or die "Error: --env is missing =value\n";
                $results[-1][2]{$1} = $2;
            }

            next;
        }

        # whatever is left must be the name of an executable

        my ($perl, $label) = split /=/, $item, 2;
        push @labels, $label;
        unless ($OPTS{autolabel}) {
            $label //= $perl;
            $label = $perl.$label if $label =~ /^\+/;
        }

        die "Error: duplicate label '$label': "
                        . "each executable must have a unique label\n"
            if defined $label && $seen{$label}++;

        die "Error: duplicate label '$label': "
                        . "seen both in --read file and on command line\n"
            if defined $label && $seen_from_reads{$label};

        my $r = qx($perl -e 'print qq(ok\n)' 2>&1);
        die "Error: unable to execute '$perl': $r\n" if $r ne "ok\n";

        push @results, [ $perl, $label,  { }, '' ];
    }

    # make args '' by default
    for (@results) {
        push @$_, '' unless @$_ > 3;
    }

    if ($OPTS{autolabel}) {

        # create a list of [ 'perl-path', $i ] pairs for all
        # $results[$i] which don't have a label
        my @labels;
        for (0..$#results)  {
            push @labels, [ $results[$_][0], $_ ]
                        unless defined $results[$_][1];
        }

        if (@labels) {
            # strip off common prefixes
            my $pre = '';
          STRIP_PREFIX:
            while (length $labels[0][0]) {
                my $c = substr($labels[0][0], 0, 1);
                for my $i (1..$#labels) {
                    last STRIP_PREFIX if substr($labels[$i][0], 0, 1) ne $c;
                }
                substr($labels[$_][0], 0, 1)  = '' for 0..$#labels;
                $pre .= $c;
            }
            # add back any final "version-ish" prefix
            $pre =~ s/^.*?([0-9\.]*)$/$1/;
            substr($labels[$_][0], 0, 0) = $pre for 0..$#labels;

            # strip off common suffixes
            my $post = '';
          STRIP_SUFFFIX:
            while (length $labels[0][0]) {
                my $c = substr($labels[0][0], -1, 1);
                for my $i (1..$#labels) {
                    last STRIP_SUFFFIX if substr($labels[$i][0], -1, 1) ne $c;
                }
                chop $labels[$_][0] for 0..$#labels;
                $post = "$c$post";
            }
            # add back any initial "version-ish" suffix
            $post =~ s/^([0-9\.]*).*$/$1/;
            $labels[$_][0] .= $post for 0..$#labels;

            # avoid degenerate empty string for single executable name
            $labels[0][0] = '0' if @labels == 1 && !length $labels[0][0];

            # if the auto-generated labels are plain integers, prefix
            # them with 'p' (for perl) to distinguish them from column
            # indices (otherwise e.g. --norm=2 is ambiguous)

            if ($labels[0][0] =~ /^\d*$/) {
                $labels[$_][0] = "p$labels[$_][0]" for 0..$#labels;
            }

            # now de-duplicate labels

            my (%seen, %index);
            $seen{$read_perls->[$_][1]}++ for 0..$#$read_perls;
            $seen{$labels[$_][0]}++ for 0..$#labels;

            for my $i (0..$#labels)  {
                my $label = $labels[$i][0];
                next unless $seen{$label} > 1;
                my $d = length($label) ? '-' : '';
                my $n = $index{$label} // 0;
                $n++ while exists $seen{"$label$d$n"};
                $labels[$i][0] .= "$d$n";
                $index{$label} = $n + 1;
            }

            # finally, store them
            $results[$_->[1]][1]= $_->[0] for @labels;
        }
    }


    return @results;
}



# Return a string containing a perl program which runs the benchmark code
# $ARGV[0] times. If $body is true, include the main body (setup) in
# the loop; otherwise create an empty loop with just pre and post.
# Note that an empty body is handled with '1;' so that a completely empty
# loop has a single nextstate rather than a stub op, so more closely
# matches the active loop; e.g.:
#   {1;}    => nextstate;                       unstack
#   {$x=1;} => nextstate; const; gvsv; sassign; unstack
# Note also that each statement is prefixed with a label; this avoids
# adjacent nextstate ops being optimised away.
#
# A final 1; statement is added so that the code is always in void
# context.
#
# It the compile flag is set for a test, the body of the loop is wrapped in
# eval 'sub { .... }' to measure compile time rather than execution time

sub make_perl_prog {
    my ($name, $test, $body) = @_;
    my ($desc, $setup, $code, $pre, $post, $compile) =
                                @$test{qw(desc setup code pre post compile)};

    $setup //= '';
    $pre  = defined $pre  ? "_PRE_: $pre; " : "";
    $post = defined $post ? "_POST_: $post; " : "";
    $code = $body ? $code : "1";
    $code = "_CODE_: $code; ";
    my $full = "$pre$code$post _CXT_: 1; ";
    $full = "eval q{sub { $full }};" if $compile;

    return <<EOF;
# $desc
package $name;
BEGIN { srand(0) }
$setup;
for my \$__loop__ (1..\$ARGV[0]) {
    $full
}
EOF
}


# Parse the output from cachegrind. Return a hash ref.
# See do_selftest() for examples of the output format.

sub parse_cachegrind {
    my ($output, $id, $perl) = @_;

    my %res;

    my @lines = split /\n/, $output;
    for (@lines) {
        unless (s/(==\d+==)|(--\d+--) //) {
            die "Error: while executing $id:\n"
              . "unexpected code or cachegrind output:\n$_\n";
        }
        if (/I\s+refs:\s+([\d,]+)/) {
            $res{Ir} = $1;
        }
        elsif (/I1\s+misses:\s+([\d,]+)/) {
            $res{Ir_m1} = $1;
        }
        elsif (/LLi\s+misses:\s+([\d,]+)/) {
            $res{Ir_mm} = $1;
        }
        elsif (/D\s+refs:\s+.*?([\d,]+) rd .*?([\d,]+) wr/) {
            @res{qw(Dr Dw)} = ($1,$2);
        }
        elsif (/D1\s+misses:\s+.*?([\d,]+) rd .*?([\d,]+) wr/) {
            @res{qw(Dr_m1 Dw_m1)} = ($1,$2);
        }
        elsif (/LLd\s+misses:\s+.*?([\d,]+) rd .*?([\d,]+) wr/) {
            @res{qw(Dr_mm Dw_mm)} = ($1,$2);
        }
        elsif (/Branches:\s+.*?([\d,]+) cond .*?([\d,]+) ind/) {
            @res{qw(COND IND)} = ($1,$2);
        }
        elsif (/Mispredicts:\s+.*?([\d,]+) cond .*?([\d,]+) ind/) {
            @res{qw(COND_m IND_m)} = ($1,$2);
        }
    }

    for my $field (keys %VALID_FIELDS) {
        die "Error: can't parse '$field' field from cachegrind output:\n$output"
            unless exists $res{$field};
        $res{$field} =~ s/,//g;
    }

    return \%res;
}


# Handle the 'grind' action

sub do_grind {
    my ($cmd_line_args) = @_; # the residue of @ARGV after option processing

    my ($loop_counts, $perls, $results, $tests, $order, @run_perls);
    my ($bisect_field, $bisect_min, $bisect_max);
    my ($done_read, $processed, $averages, %seen_labels);

    if (defined $OPTS{bisect}) {
        ($bisect_field, $bisect_min, $bisect_max) = split /,/, $OPTS{bisect}, 3;
        die "Error: --bisect option must be of form 'field,integer,integer'\n"
            unless
                    defined $bisect_max
                and $bisect_min =~ /^[0-9]+$/
                and $bisect_max =~ /^[0-9]+$/;

        die "Error: unrecognised field '$bisect_field' in --bisect option\n"
            unless $VALID_FIELDS{$bisect_field};

        die "Error: --bisect min ($bisect_min) must be <= max ($bisect_max)\n"
            if $bisect_min > $bisect_max;
    }

    # Read in previous benchmark results

    foreach my $file (@{$OPTS{read}}) {
        open my $in, '<:encoding(UTF-8)', $file
            or die "Error: can't open '$file' for reading: $!\n";
        my $data = do { local $/; <$in> };
        close $in;

        my $hash = JSON::PP::decode_json($data);
        if (int($FORMAT_VERSION) < int($hash->{version})) {
            die "Error: unsupported version $hash->{version} in file"
              . " '$file' (too new)\n";
        }
        my ($read_loop_counts, $read_perls, $read_results, $read_tests, $read_order) =
            @$hash{qw(loop_counts perls results tests order)};

        # check file contents for consistency
        my $k_o = join ';', sort @$read_order;
        my $k_r = join ';', sort keys %$read_results;
        my $k_t = join ';', sort keys %$read_tests;
        die "File '$file' contains no results\n" unless length $k_r;
        die "File '$file' contains differing test and results names\n"
            unless $k_r eq $k_t;
        die "File '$file' contains differing test and sort order names\n"
            unless $k_o eq $k_t;

        # delete tests not matching --tests= criteria, if any
        filter_tests($read_results);
        filter_tests($read_tests);

        for my $perl (@$read_perls) {
            my $label = $perl->[1];
            die "Error: duplicate label '$label': seen in file '$file'\n"
                if exists $seen_labels{$label};
            $seen_labels{$label}++;
        }

        if (!$done_read) {
            ($loop_counts, $perls, $results, $tests, $order) =
                ($read_loop_counts, $read_perls, $read_results, $read_tests, $read_order);
            $done_read = 1;
        }
        else {
            # merge results across multiple files

            if (   join(';', sort keys %$tests)
                ne join(';', sort keys %$read_tests))
            {
                my $err = "Can't merge multiple read files: "
                        . "they contain differing test sets.\n";
                if ($OPTS{verbose}) {
                    $err .= "Previous tests:\n";
                    $err .= "  $_\n" for sort keys %$tests;
                    $err .= "tests from '$file':\n";
                    $err .= "  $_\n" for sort keys %$read_tests;
                }
                else {
                    $err .= "Re-run with --verbose to see the differences.\n";
                }
                die $err;
            }

            if ("@$read_loop_counts" ne "@$loop_counts") {
                die "Can't merge multiple read files: differing loop counts:\n"
                . "  (previous=(@$loop_counts), "
                . "'$file'=(@$read_loop_counts))\n";
            }

            push @$perls, @{$read_perls};
            foreach my $test (keys %{$read_results}) {
                foreach my $label (keys %{$read_results->{$test}}) {
                    $results->{$test}{$label}= $read_results->{$test}{$label};
                }
            }
        }
    }
    die "Error: --benchfile cannot be used when --read is present\n"
        if $done_read && defined $OPTS{benchfile};

    # Gather list of perls to benchmark:

    if (@$cmd_line_args) {
        unless ($done_read) {
            # How many times to execute the loop for the two trials. The lower
            # value is intended to do the loop enough times that branch
            # prediction has taken hold; the higher loop allows us to see the
            # branch misses after that
            $loop_counts = [10, 20];

            ($tests, $order) =
                read_tests_file($OPTS{benchfile} // 't/perf/benchmarks');
        }

        @run_perls = process_executables_list($perls, @$cmd_line_args);
        push @$perls, @run_perls;
    }

    # strip @$order to just the actual tests present
    $order = [ grep exists $tests->{$_}, @$order ];

    # Now we know what perls and tests we have, do extra option processing
    # and checking (done before grinding, so time isn't wasted if we die).

    if (!$perls or !@$perls) {
        die "Error: nothing to do: no perls to run, no data to read.\n";
    }
    if (@$perls < 2 and $OPTS{show} and !$OPTS{raw}) {
        die "Error: need at least 2 perls for comparison.\n"
    }

    if ($OPTS{bisect}) {
        die "Error: exactly one perl executable must be specified for bisect\n"
            unless @$perls == 1;
        die "Error: only a single test may be specified with --bisect\n"
            unless keys %$tests == 1;
    }

    $OPTS{norm} = select_a_perl($OPTS{norm}, $perls, "--norm");

    if (defined $OPTS{'sort-perl'}) {
        $OPTS{'sort-perl'} =
                select_a_perl($OPTS{'sort-perl'}, $perls, "--sort");
    }

    if (defined $OPTS{'compact'}) {
        $OPTS{'compact'} =
                select_a_perl($OPTS{'compact'}, $perls, "--compact");
    }


    # Run the benchmarks; accumulate with any previously read # results.

    if (@run_perls) {
        $results = grind_run($tests, $order, \@run_perls, $loop_counts, $results);
    }


    # Handle the 3 forms of output

    if (defined $OPTS{write}) {
        my $json = JSON::PP::encode_json({
                    version      => $FORMAT_VERSION,
                    loop_counts  => $loop_counts,
                    perls        => $perls,
                    results      => $results,
                    tests        => $tests,
                    order        => $order,
                });

        open my $out, '>:encoding(UTF-8)', $OPTS{write}
            or die "Error: can't open '$OPTS{write}' for writing: $!\n";
        print $out $json or die "Error: writing to file '$OPTS{write}': $!\n";
        close $out       or die "Error: closing file '$OPTS{write}': $!\n";
    }

    if ($OPTS{show} or $OPTS{bisect}) {
        # numerically process the raw data
        ($processed, $averages) =
                    grind_process($results, $perls, $loop_counts);
    }

    if ($OPTS{show}) {
        if (defined $OPTS{compact}) {
            grind_print_compact($processed, $averages, $OPTS{compact},
                                $perls, $tests, $order);
        }
        else {
            grind_print($processed, $averages, $perls, $tests, $order);
        }
    }

    if ($OPTS{bisect}) {
        # these panics shouldn't happen if the bisect checks above are sound
        my @r = values %$results;
        die "Panic: expected exactly one test result in bisect\n"
                                                        if @r != 1;
        @r = values %{$r[0]};
        die "Panic: expected exactly one perl result in bisect\n"
                                                        if @r != 1;
        my $c = $r[0]{$bisect_field};
        die "Panic: no result in bisect for field '$bisect_field'\n"
                                                        unless defined $c;

        print "Bisect: $bisect_field had the value $c\n";

        exit 0 if $bisect_min <= $c and $c <= $bisect_max;
        exit 1;
    }
}


# Run cachegrind for every test/perl combo.
# It may run several processes in parallel when -j is specified.
# Return a hash ref suitable for input to grind_process()

sub grind_run {
    my ($tests, $order, $perls, $counts, $results) = @_;

    # Build a list of all the jobs to run

    my @jobs;

    for my $test (grep $tests->{$_}, @$order) {

        # Create two test progs: one with an empty loop and one with code.
        my @prog = (
            make_perl_prog($test, $tests->{$test}, 0),
            make_perl_prog($test, $tests->{$test}, 1),
        );

        for my $p (@$perls) {
            my ($perl, $label, $env, $args) = @$p;

            # Run both the empty loop and the active loop
            # $counts->[0] and $counts->[1] times.

            for my $i (0,1) {
                for my $j (0,1) {
                    my $envstr = '';
                    if (ref $env) {
                        $envstr .= "$_=$env->{$_} " for sort keys %$env;
                    }
                    my $cmd = "PERL_HASH_SEED=0 $envstr"
                            . "valgrind --tool=cachegrind  --branch-sim=yes --cache-sim=yes "
                            . "--cachegrind-out-file=/dev/null "
                            . "$OPTS{grindargs} "
                            . "$perl $OPTS{perlargs} $args - $counts->[$j] 2>&1";
                    # for debugging and error messages
                    my $id = "$test/$label "
                        . ($i ? "active" : "empty") . "/"
                        . ($j ? "long"   : "short") . " loop";

                    push @jobs, {
                        test   => $test,
                        perl   => $perl,
                        plabel => $label,
                        cmd    => $cmd,
                        prog   => $prog[$i],
                        active => $i,
                        loopix => $j,
                        id     => $id,
                    };
                }
            }
        }
    }

    # Execute each cachegrind and store the results in %results.

    local $SIG{PIPE} = 'IGNORE';

    my $max_jobs = $OPTS{jobs};
    my $running  = 0; # count of executing jobs
    my %pids;         # map pids to jobs
    my %fds;          # map fds  to jobs
    my $select = IO::Select->new();

    my $njobs     = scalar @jobs;
    my $donejobs  = 0;
    my $starttime = time();

    while (@jobs or $running) {

        if ($OPTS{debug}) {
            printf "Main loop: pending=%d running=%d\n",
                scalar(@jobs), $running;
        }

        # Start new jobs

        while (@jobs && $running < $max_jobs) {
            my $job = shift @jobs;
            my ($id, $cmd) =@$job{qw(id cmd)};

            my ($in, $out, $pid);
            $donejobs++;
            if($OPTS{verbose}) {
                my $donefrac = $donejobs / $njobs;
                my $eta = "";
                # Once we've done at least 20% we'll have a good estimate of
                # the total runtime, hence ETA
                if($donefrac >= 0.2) {
                    my $now = time();
                    my $duration  = ($now - $starttime) / $donefrac;
                    my $remaining = ($starttime + $duration) - $now;
                    $eta = sprintf ", remaining %d:%02d",
                        $remaining / 60, $remaining % 60;
                }
                warn sprintf "Starting %s (%d of %d, %.2f%%%s)\n",
                    $id, $donejobs, $njobs, 100 * $donefrac, $eta;
            }
            eval { $pid = IPC::Open2::open2($out, $in, $cmd); 1; }
                or die "Error: while starting cachegrind subprocess"
                   ." for $id:\n$@";
            $running++;
            $pids{$pid}    = $job;
            $fds{"$out"}   = $job;
            $job->{out_fd} = $out;
            $job->{output} = '';
            $job->{pid}    = $pid;

            $out->blocking(0);
            $select->add($out);

            if ($OPTS{debug}) {
                print "Started pid $pid for $id\n";
            }

            # Note:
            # In principle we should write to $in in the main select loop,
            # since it may block. In reality,
            #  a) the code we write to the perl process's stdin is likely
            #     to be less than the OS's pipe buffer size;
            #  b) by the time the perl process has read in all its stdin,
            #     the only output it should have generated is a few lines
            #     of cachegrind output preamble.
            # If these assumptions change, then perform the following print
            # in the select loop instead.

            print $in $job->{prog};
            close $in;
        }

        # Get output of running jobs

        if ($OPTS{debug}) {
            printf "Select: waiting on (%s)\n",
                join ', ', sort { $a <=> $b } map $fds{$_}{pid},
                            $select->handles;
        }

        my @ready = $select->can_read;

        if ($OPTS{debug}) {
            printf "Select: pids (%s) ready\n",
                join ', ', sort { $a <=> $b } map $fds{$_}{pid}, @ready;
        }

        unless (@ready) {
            die "Panic: select returned no file handles\n";
        }

        for my $fd (@ready) {
            my $j = $fds{"$fd"};
            my $r = sysread $fd, $j->{output}, 8192, length($j->{output});
            unless (defined $r) {
                die "Panic: Read from process running $j->{id} gave:\n$!";
            }
            next if $r;

            # EOF

            if ($OPTS{debug}) {
                print "Got eof for pid $fds{$fd}{pid} ($j->{id})\n";
            }

            $select->remove($j->{out_fd});
            close($j->{out_fd})
                or die "Panic: closing output fh on $j->{id} gave:\n$!\n";
            $running--;
            delete $fds{"$j->{out_fd}"};
            my $output = $j->{output};

            if ($OPTS{debug}) {
                my $p = $j->{prog};
                $p =~ s/^/    : /mg;
                my $o = $output;
                $o =~ s/^/    : /mg;

                print "\n$j->{id}/\nCommand: $j->{cmd}\n"
                    . "Input:\n$p"
                    . "Output\n$o";
            }

            $results->{$j->{test}}{$j->{plabel}}[$j->{active}][$j->{loopix}]
                    = parse_cachegrind($output, $j->{id}, $j->{perl});
        }

        # Reap finished jobs

        while (1) {
            my $kid = waitpid(-1, WNOHANG);
            my $ret = $?;
            last if $kid <= 0;

            unless (exists $pids{$kid}) {
                die "Panic: reaped unexpected child $kid";
            }
            my $j = $pids{$kid};
            if ($ret) {
                die sprintf("Error: $j->{id} gave return status 0x%04x\n", $ret)
                    . "with the following output\n:$j->{output}\n";
            }
            delete $pids{$kid};
        }
    }

    return $results;
}




# grind_process(): process the data that has been extracted from
# cachgegrind's output.
#
# $res is of the form ->{benchmark_name}{perl_label}[active][count]{field_name},
# where active is 0 or 1 indicating an empty or active loop,
# count is 0 or 1 indicating a short or long loop. E.g.
#
#    $res->{'expr::assign::scalar_lex'}{perl-5.21.1}[0][10]{Dw_mm}
#
# The $res data structure is modified in-place by this sub.
#
# $perls is [ [ perl-exe, perl-label], .... ].
#
# $counts is [ N, M ] indicating the counts for the short and long loops.
#
#
# return \%output, \%averages, where
#
# $output{benchmark_name}{perl_label}{field_name} = N
# $averages{perl_label}{field_name} = M
#
# where N is the raw count ($OPTS{raw}), or count_perl0/count_perlI otherwise;
# M is the average raw count over all tests ($OPTS{raw}), or
# 1/(sum(count_perlI/count_perl0)/num_tests) otherwise.

sub grind_process {
    my ($res, $perls, $counts) = @_;

    # Process the four results for each test/perf combo:
    # Convert
    #    $res->{benchmark_name}{perl_label}[active][count]{field_name} = n
    # to
    #    $res->{benchmark_name}{perl_label}{field_name} = averaged_n
    #
    # $r[0][1] - $r[0][0] is the time to do ($counts->[1]-$counts->[0])
    #                     empty loops, eliminating startup time
    # $r[1][1] - $r[1][0] is the time to do ($counts->[1]-$counts->[0])
    #                     active loops, eliminating startup time
    # (the two startup times may be different because different code
    # is being compiled); the difference of the two results above
    # divided by the count difference is the time to execute the
    # active code once, eliminating both startup and loop overhead.

    for my $tests (values %$res) {
        for my $r (values %$tests) {
            my $r2;
            for (keys %{$r->[0][0]}) {
                my $n = (  ($r->[1][1]{$_} - $r->[1][0]{$_})
                         - ($r->[0][1]{$_} - $r->[0][0]{$_})
                        ) / ($counts->[1] - $counts->[0]);
                $r2->{$_} = $n;
            }
            $r = $r2;
        }
    }

    my %totals;
    my %counts;
    my %data;

    my $perl_norm = $perls->[$OPTS{norm}][1]; # the label of the reference perl

    for my $test_name (keys %$res) {
        my $res1 = $res->{$test_name};
        my $res2_norm = $res1->{$perl_norm};
        for my $perl (keys %$res1) {
            my $res2 = $res1->{$perl};
            for my $field (keys %$res2) {
                my ($p, $q) = ($res2_norm->{$field}, $res2->{$field});

                if ($OPTS{raw}) {
                    # Avoid annoying '-0.0' displays. Ideally this number
                    # should never be negative, but fluctuations in
                    # startup etc can theoretically make this happen
                    $q = 0 if ($q <= 0 && $q > -0.1);
                    $totals{$perl}{$field} += $q;
                    $counts{$perl}{$field}++;
                    $data{$test_name}{$perl}{$field} = $q;
                    next;
                }

                # $p and $q are notionally integer counts, but
                # due to variations in startup etc, it's possible for a
                # count which is supposedly zero to be calculated as a
                # small positive or negative value.
                # In this case, set it to zero. Further below we
                # special-case zeros to avoid division by zero errors etc.

                $p = 0.0 if $p < 0.01;
                $q = 0.0 if $q < 0.01;

                if ($p == 0.0 && $q == 0.0) {
                    # Both perls gave a count of zero, so no change:
                    # treat as 100%
                    $totals{$perl}{$field} += 1;
                    $counts{$perl}{$field}++;
                    $data{$test_name}{$perl}{$field} = 1;
                }
                elsif ($p == 0.0 || $q == 0.0) {
                    # If either count is zero, there were too few events
                    # to give a meaningful ratio (and we will end up with
                    # division by zero if we try). Mark the result undef,
                    # indicating that it shouldn't be displayed; and skip
                    # adding to the average
                    $data{$test_name}{$perl}{$field} = undef;
                }
                else {
                    # For averages, we record q/p rather than p/q.
                    # Consider a test where perl_norm took 1000 cycles
                    # and perlN took 800 cycles. For the individual
                    # results we display p/q, or 1.25; i.e. a quarter
                    # quicker. For the averages, we instead sum all
                    # the 0.8's, which gives the total cycles required to
                    # execute all tests, with all tests given equal
                    # weight. Later we reciprocate the final result,
                    # i.e. 1/(sum(qi/pi)/n)

                    $totals{$perl}{$field} += $q/$p;
                    $counts{$perl}{$field}++;
                    $data{$test_name}{$perl}{$field} = $p/$q;
                }
            }
        }
    }

    # Calculate averages based on %totals and %counts accumulated earlier.

    my %averages;
    for my $perl (keys %totals) {
        my $t = $totals{$perl};
        for my $field (keys %$t) {
            $averages{$perl}{$field} = $OPTS{raw}
                ? $t->{$field} / $counts{$perl}{$field}
                  # reciprocal - see comments above
                : $counts{$perl}{$field} / $t->{$field};
        }
    }

    return \%data, \%averages;
}



# print a standard blurb at the start of the grind display

sub grind_blurb {
    my ($perls) = @_;

    print <<EOF;
Key:
    Ir   Instruction read
    Dr   Data read
    Dw   Data write
    COND conditional branches
    IND  indirect branches
    _m   branch predict miss
    _m1  level 1 cache miss
    _mm  last cache (e.g. L3) miss
    -    indeterminate percentage (e.g. 1/0)

EOF

    if ($OPTS{raw}) {
        print "The numbers represent raw counts per loop iteration.\n";
    }
    else {
        print <<EOF;
The numbers represent relative counts per loop iteration, compared to
$perls->[$OPTS{norm}][1] at 100.0%.
Higher is better: for example, using half as many instructions gives 200%,
while using twice as many gives 50%.
EOF
    }
}


# return a sorted list of the test names, plus 'AVERAGE'

sub sorted_test_names {
    my ($results, $order, $perls) = @_;

    my @names;
    unless ($OPTS{average}) {
        if (defined $OPTS{'sort-field'}) {
            my ($field, $perlix) = @OPTS{'sort-field', 'sort-perl'};
            my $perl = $perls->[$perlix][1];
            @names = sort
                {
                        $results->{$a}{$perl}{$field}
                    <=> $results->{$b}{$perl}{$field}
                }
                keys %$results;
        }
        else {
            @names = grep $results->{$_}, @$order;
        }
    }

    # No point in displaying average for only one test.
    push @names,  'AVERAGE' unless @names == 1;
    @names;
}


# format one cell data item

sub grind_format_cell {
    my ($val, $width) = @_;
    my $s;
    if (!defined $val) {
        return sprintf "%*s", $width, '-';
    }
    elsif (abs($val) >= 1_000_000) {
        # avoid displaying very large numbers (which might be the
        # result of e.g. 1 / 0.000001)
        return sprintf "%*s", $width, 'Inf';
    }
    elsif ($OPTS{raw}) {
        return sprintf "%*.1f", $width, $val;
    }
    else {
        return sprintf "%*.2f", $width, $val * 100;
    }
}

# grind_print(): display the tabulated results of all the cachegrinds.
#
# Arguments are of the form:
#    $results->{benchmark_name}{perl_label}{field_name} = N
#    $averages->{perl_label}{field_name} = M
#    $perls = [ [ perl-exe, perl-label ], ... ]
#    $tests->{test_name}{desc => ..., ...}
#    $order = [ 'foo::bar1', ... ]  # order to display tests

sub grind_print {
    my ($results, $averages, $perls, $tests, $order) = @_;

    my @perl_names = map $_->[0], @$perls;
    my @perl_labels = map $_->[1], @$perls;
    my %perl_labels;
    $perl_labels{$_->[0]} = $_->[1] for @$perls;

    # Print standard header.
    grind_blurb($perls);

    my @test_names = sorted_test_names($results, $order, $perls);

    my @fields = qw(Ir Dr Dw COND IND
                     COND_m IND_m
                     Ir_m1 Dr_m1 Dw_m1
                     Ir_mm Dr_mm Dw_mm
                  );

    if ($OPTS{fields}) {
        @fields = grep exists $OPTS{fields}{$_}, @fields;
    }

    # If only a single field is to be displayed, use a more compact
    # format with only a single line of output per test.

    my $one_field = @fields == 1;

    # The width of column 0: this is either field names, or for
    # $one_field, test names

    my $width0 = 0;
    for ($one_field ? @test_names : @fields) {
        $width0 = length if length > $width0;
    }

    # Calculate the widths of the data columns

    my @widths = map length, @perl_labels;

    for my $test (@test_names) {
        my $res = ($test eq 'AVERAGE') ? $averages : $results->{$test};
        for my $field (@fields) {
            for my $i (0..$#widths) {
                my $l = length grind_format_cell(
                                    $res->{$perl_labels[$i]}{$field}, 1);
                $widths[$i] = $l if $l > $widths[$i];
            }
        }
    }

    # Print the results for each test

    for my $test (0..$#test_names) {
        my $test_name = $test_names[$test];
        my $doing_ave = ($test_name eq 'AVERAGE');
        my $res = $doing_ave ? $averages : $results->{$test_name};

        # print per-test header

        if ($one_field) {
            print "\nResults for field $fields[0]\n\n" if $test == 0;
        }
        else {
            print "\n$test_name";
            print "\n$tests->{$test_name}{desc}" unless $doing_ave;
            print "\n\n";
        }

        # Print the perl executable names header.

        if (!$one_field || $test == 0) {
            for my $i (0,1) {
                print " " x $width0;
                for (0..$#widths) {
                    printf " %*s", $widths[$_],
                        $i ? ('-' x$widths[$_]) : $perl_labels[$_];
                }
                print "\n";
            }
        }

        my $field_suffix = '';

        # print a line of data

        for my $field (@fields) {
            if ($one_field) {
                printf "%-*s", $width0, $test_name;
            }
            else {
                # If there are enough fields, print a blank line
                # between groups of fields that have the same suffix
                if (@fields > 4) {
                    my $s = '';
                    $s = $1 if $field =~ /(_\w+)$/;
                    print "\n" if $s ne $field_suffix;
                    $field_suffix = $s;
                }
                printf "%*s", $width0, $field;
            }

            for my $i (0..$#widths) {
                print " ", grind_format_cell($res->{$perl_labels[$i]}{$field},
                                            $widths[$i]);
            }
            print "\n";
        }
    }
}



# grind_print_compact(): like grind_print(), but display a single perl
# in a compact form. Has an additional arg, $which_perl, which specifies
# which perl to display.
#
# Arguments are of the form:
#    $results->{benchmark_name}{perl_label}{field_name} = N
#    $averages->{perl_label}{field_name} = M
#    $perls = [ [ perl-exe, perl-label ], ... ]
#    $tests->{test_name}{desc => ..., ...}
#    $order = [ 'foo::bar1', ... ]  # order to display tests

sub grind_print_compact {
    my ($results, $averages, $which_perl, $perls, $tests, $order) = @_;

    # Print standard header.
    grind_blurb($perls);

    print "\nResults for $perls->[$which_perl][1]\n\n";

    my @test_names = sorted_test_names($results, $order, $perls);

    # Dump the results for each test.

     my @fields = qw( Ir Dr Dw
                      COND IND
                      COND_m IND_m
                      Ir_m1 Dr_m1 Dw_m1
                      Ir_mm Dr_mm Dw_mm
                    );
    if ($OPTS{fields}) {
        @fields = grep exists $OPTS{fields}{$_}, @fields;
    }

    # calculate the max width of the test names

    my $name_width = 0;
    for (@test_names) {
        $name_width = length if length > $name_width;
    }

    # Calculate the widths of the data columns

    my @widths = map length, @fields;

    for my $test (@test_names) {
        my $res = ($test eq 'AVERAGE') ? $averages : $results->{$test};
        $res = $res->{$perls->[$which_perl][1]};
        for my $i (0..$#fields) {
            my $l = length grind_format_cell($res->{$fields[$i]}, 1);
            $widths[$i] = $l if $l > $widths[$i];
        }
    }

    # Print header

    printf " %*s", $widths[$_], $fields[$_] for 0..$#fields;
    print "\n";
    printf " %*s", $_, ('-' x $_) for @widths;
    print "\n";

    # Print the results for each test

    for my $test_name (@test_names) {
        my $doing_ave = ($test_name eq 'AVERAGE');
        my $res = $doing_ave ? $averages : $results->{$test_name};
        $res = $res->{$perls->[$which_perl][1]};
        my $desc = $doing_ave
            ? $test_name
            : sprintf "%-*s   %s", $name_width, $test_name,
                                 $tests->{$test_name}{desc};

        for my $i (0..$#fields) {
            print " ", grind_format_cell($res->{$fields[$i]}, $widths[$i]);
        }
        print "  $desc\n";
    }
}


# do_selftest(): check that we can parse known cachegrind()
# output formats. If the output of cachegrind changes, add a *new*
# test here; keep the old tests to make sure we continue to parse
# old cachegrinds

sub do_selftest {

    my @tests = (
        'standard',
        <<'EOF',
==32350== Cachegrind, a cache and branch-prediction profiler
==32350== Copyright (C) 2002-2013, and GNU GPL'd, by Nicholas Nethercote et al.
==32350== Using Valgrind-3.9.0 and LibVEX; rerun with -h for copyright info
==32350== Command: perl5211o /tmp/uiS2gjdqe5 1
==32350== 
--32350-- warning: L3 cache found, using its data for the LL simulation.
==32350== 
==32350== I   refs:      1,124,055
==32350== I1  misses:        5,573
==32350== LLi misses:        3,338
==32350== I1  miss rate:      0.49%
==32350== LLi miss rate:      0.29%
==32350== 
==32350== D   refs:        404,275  (259,191 rd   + 145,084 wr)
==32350== D1  misses:        9,608  (  6,098 rd   +   3,510 wr)
==32350== LLd misses:        5,794  (  2,781 rd   +   3,013 wr)
==32350== D1  miss rate:       2.3% (    2.3%     +     2.4%  )
==32350== LLd miss rate:       1.4% (    1.0%     +     2.0%  )
==32350== 
==32350== LL refs:          15,181  ( 11,671 rd   +   3,510 wr)
==32350== LL misses:         9,132  (  6,119 rd   +   3,013 wr)
==32350== LL miss rate:        0.5% (    0.4%     +     2.0%  )
==32350== 
==32350== Branches:        202,372  (197,050 cond +   5,322 ind)
==32350== Mispredicts:      19,153  ( 17,742 cond +   1,411 ind)
==32350== Mispred rate:        9.4% (    9.0%     +    26.5%   )
EOF
        {
            COND    =>  197050,
            COND_m  =>   17742,
            Dr      =>  259191,
            Dr_m1   =>    6098,
            Dr_mm   =>    2781,
            Dw      =>  145084,
            Dw_m1   =>    3510,
            Dw_mm   =>    3013,
            IND     =>    5322,
            IND_m   =>    1411,
            Ir      => 1124055,
            Ir_m1   =>    5573,
            Ir_mm   =>    3338,
        },
    );

    for ('./t', '.') {
        my $t = "$_/test.pl";
        next unless  -f $t;
        require $t;
    }
    plan(@tests / 3 * keys %VALID_FIELDS);

    while (@tests) {
        my $desc     = shift @tests;
        my $output   = shift @tests;
        my $expected = shift @tests;
        my $p = parse_cachegrind($output);
        for (sort keys %VALID_FIELDS) {
            is($p->{$_}, $expected->{$_}, "$desc, $_");
        }
    }
}
