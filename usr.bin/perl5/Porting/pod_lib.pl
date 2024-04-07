#!/usr/bin/perl -w

use strict;
use File::Find;

=head1 NAME

Porting/pod_lib.pl - functions for building and installing POD

=head1 SYNOPSIS

    require './Porting/pod_lib.pl';

=cut

=head1 DESCRIPTION

This program, when C<require>d into other programs in the Perl 5 core
distribution, provides functions useful during building and, secondarily,
testing.

As of this writing, the functions in this program are used in these other
programs:

    installman
    installperl
    pod/buildtoc
    pod/perl.pod
    Porting/new-perldelta.pl
    Porting/pod_rules.pl

Note:  Since these functions are used during the Perl build process, they must
work with F<miniperl>.  That necessarily implies that these functions must not
rely on XS modules, either directly or indirectly (e.g., C<autodie>).

=head1 SUBROUTINES

=head2 C<my_die()>

=over 4

=item * Purpose

Exit from a process with an error code and a message.

=item * Arguments

List of arguments to be passed with the error message.  Example:

    close $fh or my_die("close 'utils.lst': $!");

=item * Return Value

Exit code C<255>.

=item * Comment

Prints C<ABORTED> to STDERR.

=back

=cut

# In some situations, eg cross-compiling, we get run with miniperl, so we can't use Digest::MD5
my $has_md5;
BEGIN {
    use Carp;
    $has_md5 = eval { require Digest::MD5; Digest::MD5->import('md5');  1; };
}


# make it clearer when we haven't run to completion, as we can be quite
# noisy when things are working ok

sub my_die {
    print STDERR "$0: ", @_;
    print STDERR "\n" unless $_[-1] =~ /\n\z/;
    print STDERR "ABORTED\n";
    exit 255;
}

=head2 C<open_or_die()>

=over 4

=item * Purpose

Opens a file or fails if it cannot.

=item * Arguments

String holding filename to be opened.  Example:

    $fh = open_or_die('utils.lst');

=item * Return Value

Handle to opened file.

=back

=cut

sub open_or_die {
    my $filename = shift;
    open my $fh, '<', $filename or my_die "Can't open $filename: $!";
    return $fh;
}

=head2 C<slurp_or_die()>

=over 4

=item * Purpose

Read the contents of a file into memory as a single string.

=item * Arguments

String holding name of file to be read into memory.

    $olddelta = slurp_or_die('pod/perldelta.pod');

=item * Return Value

String holding contents of file.

=back

=cut

sub slurp_or_die {
    my $filename = shift;
    my $fh = open_or_die($filename);
    binmode $fh;
    local $/;
    my $contents = <$fh>;
    die "Can't read $filename: $!" unless defined $contents and close $fh;
    return $contents;
}

=head2 C<write_or_die()>

=over 4

=item * Purpose

Write out a string to a file.

=item * Arguments

List of two arguments:  (i) String holding name of file to be written to; (ii)
String holding contents to be written.

    write_or_die($olddeltaname, $olddelta);

=item * Return Value

Implicitly returns true value upon success.

=back

=cut

sub write_or_die {
    my ($filename, $contents) = @_;
    open my $fh, '>', $filename or die "Can't open $filename for writing: $!";
    binmode $fh;
    print $fh $contents or die "Can't write to $filename: $!";
    close $fh or die "Can't close $filename: $!";
}

=head2 C<verify_contiguous()>

=over 4

=item * Purpose

Verify that a makefile or makefile constructor contains exactly one contiguous
run of lines which matches a given pattern. C<croak()>s if the pattern is not
found, or found in more than one place.

By "makefile or makefile constructor" we mean a file which is one of the
right-hand values in this list of key-value pairs:

            manifest => 'MANIFEST',
            vms => 'vms/descrip_mms.template',
            nmake => 'win32/Makefile',
            gmake => 'win32/GNUmakefile',
            podmak => 'win32/pod.mak',
            unix => 'Makefile.SH',

(Currently found in C<%Targets> in F<Porting/pod_rules.pl>.)

=item * Arguments

=over 4

=item * Name of target

String holding the key of one element in C<%Targets> in F<Porting/pod_rules.pl>.

=item * Contents of file

String holding slurped contents of the file named in the value of the element
in C<%Targets> in F<Porting/pod_rules.pl> named in the first argument.

=item * Pattern of interest

Compiled regular expression pertinent to a particular makefile constructor.

=item * Name to report on error

String holding description.

=back

=item * Return Value

The contents of the file, with C<qr/\0+/> substituted for the pattern.

=item * Example (drawn from F<Porting/pod_rules.pl> C<do_unix()>):

    my $makefile_SH = slurp_or_die('./Makefile.SH');
    my $re = qr/some\s+pattern/;
    my $makefile_SH_out =
        verify_contiguous('unix', $makefile_SH, $re, 'copy rules');

=back

=cut

sub verify_contiguous {
    my ($name, $content, $re, $what) = @_;
    require Carp;
    $content =~ s/$re/\0/g;
    my $sections = () = $content =~ m/\0+/g;
    Carp::croak("$0: $name contains no $what") if $sections < 1;
    Carp::croak("$0: $name contains discontiguous $what") if $sections > 1;
    return $content;
}

=head2 C<process()>

=over 4

=item * Purpose

Read a file from disk, pass the contents to the callback, and either update
the file on disk (if changed) or generate TAP output to confirm that the
version on disk is up to date. C<die>s if the file contains any C<NUL> bytes.
This permits the callback routine to use C<NUL> bytes as placeholders while
manipulating the file's contents.

=item * Arguments

=over 4

=item * Description for use in error messages

=item * Name of file

=item * Callback

Passed description and file contents, should return updated file contents.

=item * Test number

If defined, generate TAP output to C<STDOUT>. If defined and false, generate
an unnumbered test. Otherwise this is the test number in the I<ok> line.

=item * Verbose flag

If true, generate verbose output.

=back

=item * Return Value

Does not return anything.

=back

=cut

sub process {
    my ($desc, $filename, $callback, $test, $verbose) = @_;

    print "Now processing $filename\n" if $verbose;
    my $orig = slurp_or_die($filename);
    my_die "$filename contains NUL bytes" if $orig =~ /\0/;

    my $new = $callback->($desc, $orig);

    if (defined $test) {
        printf "%s%s # $filename is up to date\n",
            ($new eq $orig ? 'ok' : 'not ok'), ($test ? " $test" : '');
        return;
    } elsif ($new eq $orig) {
        print "Was not modified\n"
            if $verbose;
        return;
    }

    my $mode = (stat $filename)[2];
    my_die "Can't stat $filename: $!"
        unless defined $mode;
    rename $filename, "$filename.old"
        or my_die "Can't rename $filename to $filename.old: $!";

    write_or_die($filename, $new);
    chmod $mode & 0777, $filename or my_die "can't chmod $mode $filename: $!";
}

=head2 C<pods_to_install()>

=over 4

=item * Purpose

Create a lookup table holding information about PODs to be installed.

=item * Arguments

None.

=item * Return Value

Reference to a hash with a structure like this:

    $found = {
      'MODULE' => {
        'CPAN::Bundle' => 'lib/CPAN/Bundle.pm',
        'Locale::Codes::Script_Retired' =>
            'lib/Locale/Codes/Script_Retired.pm',
        'Pod::Simple::DumpAsText' =>
            'lib/Pod/Simple/DumpAsText.pm',
        # ...
        'Locale::Codes::LangVar' =>
            'lib/Locale/Codes/LangVar.pod'
      },
      'PRAGMA' => {
        'fields' => 'lib/fields.pm',
        'subs' => 'lib/subs.pm',
        # ...
      },

=item * Comment

Broadly speaking, the function assembles a list of all F<.pm> and F<.pod>
files in the distribution and then excludes certain files from installation.

=back

=cut

sub pods_to_install {
    # manpages not to be installed
    my %do_not_install = map { ($_ => 1) }
        qw(Pod::Functions XS::APItest XS::Typemap);
    $do_not_install{"ExtUtils::XSSymSet"} = 1
        unless $^O eq "VMS";

    my (%done, %found);

    File::Find::find({no_chdir=>1,
                      wanted => sub {
                          if (m!/t\z!) {
                              ++$File::Find::prune;
                              return;
                          }

                          # $_ is $File::Find::name when using no_chdir
                          return unless m!\.p(?:m|od)\z! && -f $_;
                          return if m!lib/Net/FTP/.+\.pm\z!; # Hi, Graham! :-)
                          # Skip .pm files that have corresponding .pod files
                          return if s!\.pm\z!.pod! && -e $_;
                          s!\.pod\z!!;
                          s!\Alib/!!;
                          s!/!::!g;

                          my_die("Duplicate files for $_, '$done{$_}' and '$File::Find::name'")
                              if exists $done{$_};
                          $done{$_} = $File::Find::name;

                          return if $do_not_install{$_};
                          return if is_duplicate_pod($File::Find::name);
                          $found{/\A[a-z]/ ? 'PRAGMA' : 'MODULE'}{$_}
                              = $File::Find::name;
                      }}, 'lib');
    return \%found;
}

my %state = (
             # Don't copy these top level READMEs
             ignore => {
                        micro => 1,
                        # vms => 1,
                       },
            );

{
    my (%Lengths, %MD5s);

    sub is_duplicate_pod {
        my $file = shift;
        local $_;

        return if !$has_md5;

        # Initialise the list of possible source files on the first call.
        unless (%Lengths) {
            __prime_state() unless $state{master};
            foreach (@{$state{master}}) {
                next unless $_->[2]{dual};
                # This is a dual-life perl*.pod file, which will have be copied
                # to lib/ by the build process, and hence also found there.
                # These are the only pod files that might become duplicated.
                ++$Lengths{-s $_->[1]};
                ++$MD5s{md5(slurp_or_die($_->[1]))};
            }
        }

        # We are a file in lib. Are we a duplicate?
        # Don't bother calculating the MD5 if there's no interesting file of
        # this length.
        return $Lengths{-s $file} && $MD5s{md5(slurp_or_die($file))};
    }
}

sub __prime_state {
    my $source = 'perldelta.pod';
    my $filename = "pod/$source";
    my $contents = slurp_or_die($filename);
    my @want =
        $contents =~ /perldelta - what is new for perl v(\d+)\.(\d+)\.(\d+)\r?\n/;
    die "Can't extract version from $filename" unless @want;
    my $delta_leaf = join '', 'perl', @want, 'delta';
    $state{delta_target} = "$delta_leaf.pod";
    $state{delta_version} = \@want;

    # This way round so that keys can act as a MANIFEST skip list
    # Targets will always be in the pod directory. Currently we can only cope
    # with sources being in the same directory.
    $state{copies}{$state{delta_target}} = $source;

    # The default flags if none explicitly set for the current file.
    my $current_flags = '';
    my (%flag_set, @paths);

    my $master = open_or_die('pod/perl.pod');

    while (<$master>) {
        last if /^=begin buildtoc$/;
    }
    die "Can't find '=begin buildtoc':" if eof $master;

    while (<$master>) {
        next if /^$/ or /^#/;
        last if /^=end buildtoc/;
        my ($command, @args) = split ' ';
        if ($command eq 'flag') {
            # For the named pods, use these flags, instead of $current_flags
            my $flags = shift @args;
            my_die("Malformed flag $flags")
                unless $flags =~ /\A=([a-z]*)\z/;
            $flag_set{$_} = $1 foreach @args;
        } elsif ($command eq 'path') {
            # If the pod's name matches the regex, prepend the given path.
            my_die("Malformed path for /$args[0]/")
                unless @args == 2;
            push @paths, [qr/\A$args[0]\z/, $args[1]];
        } elsif ($command eq 'aux') {
            # The contents of perltoc.pod's "AUXILIARY DOCUMENTATION" section
            $state{aux} = [sort @args];
        } else {
            my_die("Unknown buildtoc command '$command'");
        }
    }

    foreach (<$master>) {
        next if /^$/ or /^#/;
        next if /^=head2/;
        last if /^=for buildtoc __END__$/;

        if (my ($action, $flags) = /^=for buildtoc flag ([-+])([a-z]+)/) {
            if ($action eq '+') {
                $current_flags .= $flags;
            } else {
                my_die("Attempt to unset [$flags] failed - flags are '$current_flags")
                    unless $current_flags =~ s/[\Q$flags\E]//g;
            }
        } elsif (my ($leafname, $desc) = /^\s+(\S+)\s+(.*)/) {
            my $podname = $leafname;
            my $filename = "pod/$podname.pod";
            foreach (@paths) {
                my ($re, $path) = @$_;
                if ($leafname =~ $re) {
                    $podname = $path . $leafname;
                    $filename = "$podname.pod";
                    last;
                }
            }

            # Keep this compatible with pre-5.10
            my $flags = delete $flag_set{$leafname};
            $flags = $current_flags unless defined $flags;

            my %flags;
            $flags{toc_omit} = 1 if $flags =~ tr/o//d;
            $flags{dual} = $podname ne $leafname;

            $state{generated}{"$podname.pod"}++ if $flags =~ tr/g//d;

            if ($flags =~ tr/r//d) {
                my $readme = $podname;
                $readme =~ s/^perl//;
                $state{readmes}{$readme} = $desc;
                $flags{readme} = 1;
            } else {
                $state{pods}{$podname} = $desc;
            }
            my_die "Unknown flag found in section line: $_" if length $flags;

            push @{$state{master}},
                [$leafname, $filename, \%flags];

            if ($podname eq 'perldelta') {
                local $" = '.';
                push @{$state{master}},
                    [$delta_leaf, "pod/$state{delta_target}"];
                $state{pods}{$delta_leaf} = "Perl changes in version @want";
            }

        } else {
            my_die("Malformed line: $_");
        }
    }
    close $master or my_die("close pod/perl.pod: $!");

    my_die("perl.pod sets flags for unknown pods: "
           . join ' ', sort keys %flag_set)
        if keys %flag_set;
}

=head2 C<get_pod_metadata()>

=over 4

=item * Purpose

Create a data structure holding information about files containing text in POD format.

=item * Arguments

List of one or more arguments.

=over 4

=item * Boolean true or false

=item * Reference to a subroutine.

=item * Various other arguments.

=back

Example:

    $state = get_pod_metadata(
        0, sub { warn @_ if @_ }, 'pod/perltoc.pod');

    get_pod_metadata(
        1, sub { warn @_ if @_ }, values %Build);

=item * Return Value

Hash reference; each element provides either a list or a lookup table for
information about various types of POD files.

  'aux'             => [ # utility programs like
                            'h2xs' and 'perldoc' ]
  'generated'       => { # lookup table for generated POD files
                            like 'perlapi.pod' }
  'ignore'          => { # lookup table for files to be ignored }
  'pods'            => { # lookup table in "name" =>
                            "short description" format }
  'readmes'         => { # lookup table for OS-specific
                            and other READMEs }
  'delta_version'   => [ # major version number, minor no.,
                            patch no. ]
  'delta_target'    => 'perl<Mmmpp>delta.pod',
  'master'          => [ # list holding entries for files callable
                        by 'perldoc' ]
  'copies'          => { # patch version perldelta =>
                        minor version perldelta }

=item * Comment

Instances where this subroutine is used may be found in these files:

    pod/buildtoc
    Porting/new-perldelta.pl
    Porting/pod_rules.pl

=back

=cut

sub get_pod_metadata {
    # Do we expect to find generated pods on disk?
    my $permit_missing_generated = shift;
    # Do they want a consistency report?
    my $callback = shift;
    local $_;

    __prime_state() unless $state{master};
    return \%state unless $callback;

    my %BuildFiles;

    foreach my $path (@_) {
        $path =~ m!([^/]+)$!;
        ++$BuildFiles{$1};
    }

    # Sanity cross check

    my (%disk_pods, %manipods, %manireadmes);
    my (%cpanpods, %cpanpods_leaf);
    my (%our_pods);

    # There are files that we don't want to list in perl.pod.
    # Maybe the various stub manpages should be listed there.
    my %ignoredpods = map { ( "$_.pod" => 1 ) } qw( );

    # Convert these to a list of filenames.
    ++$our_pods{"$_.pod"} foreach keys %{$state{pods}};
    foreach (@{$state{master}}) {
        ++$our_pods{"$_->[0].pod"}
            if $_->[2]{readme};
    }

    opendir my $dh, 'pod';
    while (defined ($_ = readdir $dh)) {
        next unless /\.pod\z/;
        ++$disk_pods{$_};
    }

    # Things we copy from won't be in perl.pod
    # Things we copy to won't be in MANIFEST

    my $mani = open_or_die('MANIFEST');
    while (<$mani>) {
        chomp;
        s/\s+.*$//;
        if (m!^pod/([^.]+\.pod)!i) {
            ++$manipods{$1};
        } elsif (m!^README\.(\S+)!i) {
            next if $state{ignore}{$1};
            ++$manireadmes{"perl$1.pod"};
        } elsif (exists $our_pods{$_}) {
            ++$cpanpods{$_};
            m!([^/]+)$!;
            ++$cpanpods_leaf{$1};
            $disk_pods{$_}++
                if -e $_;
        }
    }
    close $mani or my_die "close MANIFEST: $!\n";

    # Are we running before known generated files have been generated?
    # (eg in a clean checkout)
    my %not_yet_there;
    if ($permit_missing_generated) {
        # If so, don't complain if these files aren't yet in place
        %not_yet_there = (%manireadmes, %{$state{generated}}, %{$state{copies}})
    }

    my @inconsistent;
    foreach my $i (sort keys %disk_pods) {
        push @inconsistent, "$0: $i exists but is unknown by buildtoc\n"
            unless $our_pods{$i} || $ignoredpods{$i};
        push @inconsistent, "$0: $i exists but is unknown by MANIFEST\n"
            if !$BuildFiles{'MANIFEST'} # Ignore if we're rebuilding MANIFEST
                && !$manipods{$i} && !$manireadmes{$i} && !$state{copies}{$i}
                    && !$state{generated}{$i} && !$cpanpods{$i};
    }
    foreach my $i (sort keys %our_pods) {
        push @inconsistent, "$0: $i is known by buildtoc but does not exist\n"
            unless $disk_pods{$i} or $BuildFiles{$i} or $not_yet_there{$i};
    }
    unless ($BuildFiles{'MANIFEST'}) {
        # Again, ignore these if we're about to rebuild MANIFEST
        foreach my $i (sort keys %manipods) {
            push @inconsistent, "$0: $i is known by MANIFEST but does not exist\n"
                unless $disk_pods{$i};
            push @inconsistent, "$0: $i is known by MANIFEST but is marked as generated\n"
                if $state{generated}{$i};
        }
    }
    &$callback(@inconsistent);
    return \%state;
}

1;

# ex: set ts=8 sts=4 sw=4 et:
