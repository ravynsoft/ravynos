#!/usr/bin/perl -w

use strict;
our (%Build, %Targets, $Verbose, $Test);
use Text::Tabs;
use Text::Wrap;
use Getopt::Long;

if (ord("A") == 193) {
    print "1..0 # EBCDIC sort order is different\n";
    exit;
}

# Generate the sections of files listed in %Targets from pod/perl.pod
# Mostly these are rules in Makefiles
#
# --verbose gives slightly more output
# --build-all tries to build everything
# --build-foo updates foo as follows
# --showfiles shows the files to be changed
# --tap emit TAP (testing) output describing the state of the pod files

%Targets = (
            manifest => 'MANIFEST',
            vms => 'vms/descrip_mms.template',
            nmake => 'win32/Makefile',
            gmake => 'win32/GNUmakefile',
            podmak => 'win32/pod.mak',
            unix => 'Makefile.SH',
            # plan9 =>  'plan9/mkfile',
           );

require './Porting/pod_lib.pl';
require './Porting/manifest_lib.pl';
sub my_die;

# process command-line switches
{
    my @files = keys %Targets;
    my $filesopts = join(" | ", map { "--build-$_" } "all", sort @files);
    my $showfiles;
    my %build_these;
    die "$0: Usage: $0 [--verbose] [--showfiles] [$filesopts]\n"
        unless GetOptions (verbose => \$Verbose,
                           showfiles => \$showfiles,
                           tap => \$Test,
                           map {+"build-$_", \$build_these{$_}} @files, 'all')
            && !@ARGV;
    if ($build_these{all}) {
        %Build = %Targets;
    } else {
        while (my ($file, $want) = each %build_these) {
            $Build{$file} = $Targets{$file} if $want;
        }
        # Default to --build-all if no targets given.
        %Build = %Targets if !%Build;
    }
    if ($showfiles) {
        print join(" ", sort { lc $a cmp lc $b } values %Build), "\n";
        exit(0);
    }
}

if ($Verbose) {
    print "I will be building $_\n" foreach sort keys %Build;
}

my $test = 1;
# For testing, generated files must be present and we're rebuilding nothing.
# For normal rebuilding, generated files may not be present, and we mute
# warnings about inconsistencies in any file we're about to rebuild.
my $state = $Test
    ? get_pod_metadata(0, sub {
                           printf "1..%d\n", 1 + scalar keys %Build;
                           if (@_) {
                               print "not ok $test # got Pod metadata\n";
                               die @_;
                           }
                           print "ok $test # got Pod metadata\n";
                       })
    : get_pod_metadata(1, sub { warn @_ if @_ }, values %Build);

sub generate_manifest {
    # Annoyingly, unexpand doesn't consider it good form to replace a single
    # space before a tab with a tab
    # Annoyingly (2) it returns read only values.
    my @temp = unexpand (map {sprintf "%-32s%s", @$_} @_);
    map {s/ \t/\t\t/g; $_} @temp;
}

sub generate_manifest_pod {
    generate_manifest map {["pod/$_.pod", $state->{pods}{$_}]}
        sort grep {
            !$state->{copies}{"$_.pod"}
                && !$state->{generated}{"$_.pod"}
                    && !-e "$_.pod"
                } keys %{$state->{pods}};
}

sub generate_manifest_readme {
    generate_manifest sort {$a->[0] cmp $b->[0]}
        ["README.vms", "Notes about installing the VMS port"],
            map {["README.$_", $state->{readmes}{$_}]} keys %{$state->{readmes}};
}

sub generate_nmake_1 {
    # XXX Fix this with File::Spec
    (map {sprintf "\tcopy ..\\README.%-8s ..\\pod\\perl$_.pod\n", $_}
     sort keys %{$state->{readmes}}),
         (map {"\tcopy ..\\pod\\$state->{copies}{$_} ..\\pod\\$_\n"}
          sort keys %{$state->{copies}});
}

# This doesn't have a trailing newline
sub generate_nmake_2 {
    # Spot the special case
    local $Text::Wrap::columns = 76;
    my $line = wrap ("\t    ", "\t    ",
                     join " ", sort(keys %{$state->{copies}},
                                    keys %{$state->{generated}},
                                    map {"perl$_.pod"} keys %{$state->{readmes}}));
    $line =~ s/$/ \\/mg;
    $line =~ s/ \\$//;
    $line;
}

sub generate_pod_mak {
    my $variable = shift;
    my @lines;
    my $line = "\U$variable = " . join "\t\\\n\t",
        map {"$_.$variable"} sort grep { $_ !~ m{/} } keys %{$state->{pods}};
    # Special case
    $line =~ s/.*perltoc.html.*\n//m;
    $line;
}

sub do_manifest {
    my ($name, $prev) = @_;
    my @manifest =
        grep {! m!^pod/[^. \t]+\.pod.*!}
            grep {! m!^README\.(\S+)! || $state->{ignore}{$1}} split "\n", $prev;
    # NOTE - the sort code here is shared with Porting/manisort currently.
    # If you change one, change the other. Or refactor them. :-)
    join "\n",  sort_manifest(
                    @manifest,
                    &generate_manifest_pod(),
                    &generate_manifest_readme()
                ),
                '', # elegant way to add a newline to the end
    ;
}

sub do_nmake {
    my ($name, $makefile) = @_;
    my $re = qr/^\tcopy \.\.\\README[^\n]*\n/sm;
    $makefile = verify_contiguous($name, $makefile, $re, 'README copies');
    # Now remove the other copies that follow
    1 while $makefile =~ s/\0\tcopy .*\n/\0/gm;
    $makefile =~ s/\0+/join ("", &generate_nmake_1)/se;

    $makefile =~ s{(-cd \$\(PODDIR\) && del /f[^\n]+).*?(-cd \.\.\\utils && del /f)}
                  {"$1\n" . &generate_nmake_2."\n\t$2"}se;
    $makefile;
}

# shut up used only once warning
*do_gmake = *do_gmake = \&do_nmake;

sub do_podmak {
    my ($name, $body) = @_;
    foreach my $variable (qw(pod man html tex)) {
        my_die "could not find $variable in $name"
            unless $body =~ s{\n\U$variable\E = (?:[^\n]*\\\n)*[^\n]*}
                             {"\n" . generate_pod_mak ($variable)}se;
    }
    $body;
}

sub do_vms {
    my ($name, $makefile) = @_;

    # Looking for the macro defining the current perldelta:
    #PERLDELTA_CURRENT = [.pod]perl5139delta.pod

    my $re = qr{\nPERLDELTA_CURRENT\s+=\s+\Q[.pod]perl\E\d+delta\.pod\n}smx;
    $makefile
        = verify_contiguous($name, $makefile, $re, 'current perldelta macro');
    $makefile =~ s/\0+/join "\n", '', "PERLDELTA_CURRENT = [.pod]$state->{delta_target}", ''/se;

    $makefile;
}

sub do_unix {
    my ($name, $makefile_SH) = @_;

    $makefile_SH =~ s{^(perltoc_pod_prereqs = extra.pods).*}
                     {join ' ', $1, map "pod/$_",
                          sort(keys %{$state->{copies}},
                               grep {!/perltoc/} keys %{$state->{generated}})
                      }mge;

    # pod/perl511delta.pod: pod/perldelta.pod
    #         cd pod && $(LNS) perldelta.pod perl511delta.pod

    # although it seems that HP-UX make gets confused, always tried to
    # regenerate the symlink, and then the ln -s fails, as the target exists.

    my $re = qr{(
pod/perl[a-z0-9_]+\.pod: pod/perl[a-z0-9_]+\.pod
	\$\(RMS\) pod/perl[a-z0-9_]+\.pod
	\$\(LNS\) perl[a-z0-9_]+\.pod pod/perl[a-z0-9_]+\.pod
)+}sm;
    $makefile_SH = verify_contiguous($name, $makefile_SH, $re, 'copy rules');

    my @copy_rules = map "
pod/$_: pod/$state->{copies}{$_}
	\$(RMS) pod/$_
	\$(LNS) $state->{copies}{$_} pod/$_
", keys %{$state->{copies}};

    $makefile_SH =~ s/\0+/join '', @copy_rules/se;
    $makefile_SH;
}

# Do stuff
process($_, $Build{$_}, main->can("do_$_"), $Test && ++$test, $Verbose)
    foreach sort keys %Build;

# ex: set ts=8 sts=4 sw=4 et:
