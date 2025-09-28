#!/usr/bin/perl -w

#
# cmpVERSION - compare the current Perl source tree and a given tag
# for modules that have identical version numbers but different contents.
#
# with -d option, output the diffs too
# with -x option, exclude files from modules where blead is not upstream
#
# (after all, there are tools like core-cpan-diff that can already deal with
# them)
#
# Original by slaven@rezic.de, modified by jhi and matt.w.johnson@gmail.com.
# Adaptation to produce TAP by Abigail, folded back into this file by Nicholas

use strict;
use 5.006;

use ExtUtils::MakeMaker;
use File::Spec::Functions qw(devnull);
use Getopt::Long;
use Time::Local qw(timelocal_posix);

my ($diffs, $exclude_upstream, $tag_to_compare, $tap);
unless (GetOptions('diffs' => \$diffs,
                   'exclude|x' => \$exclude_upstream,
                   'tag=s' => \$tag_to_compare,
                   'tap' => \$tap,
                   ) && @ARGV == 0) {
    die "usage: $0 [ -d -x --tag TAG --tap]";
}

die "$0: This does not look like a Perl directory\n"
    unless -f "perl.h" && -d "Porting";

if (-d ".git" || (exists $ENV{GIT_DIR} && -d $ENV{GIT_DIR})) {
    # Looks good
} else {
    # Also handle linked worktrees created by git-worktree:
    my $found;
    if (-f '.git') {
        # the hash of the initial commit in perl.git (perl-1.0)
        my $commit = '8d063cd8450e59ea1c611a2f4f5a21059a2804f1';
        my $out = `git rev-parse --verify --quiet '$commit^{commit}'`;
        chomp $out;
        if($out eq $commit) {
            ++$found;
        }
    }

    die "$0: This is a Perl directory but does not look like Git working directory\n"
        unless $found;
}

my $null = devnull();

unless (defined $tag_to_compare) {
    my $check = 'HEAD';
    while(1) {
        $check = `git describe --abbrev=0 $check 2>$null`;
        chomp $check;
        last unless $check =~ /-RC/;
        $check .= '~1';
    }
    $tag_to_compare = $check;
    # Thanks to David Golden for this suggestion.

}

unless (length $tag_to_compare) {
    die "$0: Git found, but no Git tags found\n"
        unless $tap;
    print "1..0 # SKIP: Git found, but no Git tags found\n";
    exit 0;
}

my $tag_exists = `git --no-pager tag -l $tag_to_compare 2>$null`;
chomp $tag_exists;

unless ($tag_exists eq $tag_to_compare) {
    die "$0: '$tag_to_compare' is not a known Git tag\n" unless $tap;
    print "1..0 # SKIP: '$tag_to_compare' is not a known Git tag\n";
    exit 0;
}

my $commit_epoch = `git log -1 --format="%ct"`;
chomp($commit_epoch);
# old git versions dont support taggerdate:unix. so use :iso8601 and then
# use timelocal_posix() to convert to an epoch.
my $tag_date = `git for-each-ref --format="%(taggerdate:iso8601)" refs/tags/$tag_to_compare`;
chomp($tag_date);
my $tag_epoch= do {
    my ($Y,$M,$D,$h,$m,$s) = split /[- :]/, $tag_date; # 2023-03-20 22:49:09
    timelocal_posix($s,$m,$h,$D,$M,$Y);
};

if ($commit_epoch - $tag_epoch > 60 * 24 * 60 * 60) {
    my $months = sprintf "%.2f", ($commit_epoch - $tag_epoch) / (30 * 24 * 60 * 60);
    my $message=
        "Tag '$tag_to_compare' is very old compared to the most recent commit.\n"
      . "We normally release a new version every month, and this one is $months months\n"
      . "older than the current commit.  You probably have not synchronized your tags.\n"
      . "This is common with github clones.  You can try the following:\n"
      . "\n"
      . "  git remote add -f upstream git\@github.com:Perl/perl5.git\n"
      . "\n"
      . "to fix your checkout.\n";
    die "$0: $message" unless $tap;
    $message= "$message";
    $message=~s/^/# /mg;
    print STDERR "\n$message";
    print "1..0 # SKIP: Tag '$tag_to_compare' is $months months old. Update your tags!\n";
    exit 0;
}

my %upstream_files;
if ($exclude_upstream) {
    unshift @INC, 'Porting';
    require Maintainers;

    for my $m (grep {!defined $Maintainers::Modules{$_}{UPSTREAM}
                         or $Maintainers::Modules{$_}{UPSTREAM} ne 'blead'}
               keys %Maintainers::Modules) {
        $upstream_files{$_} = 1 for Maintainers::get_module_files($m);
    }
}

# Files to skip from the check for one reason or another,
# usually because they pull in their version from some other file.
my %skip;
@skip{
    'cpan/Digest/t/lib/Digest/Dummy.pm', # just a test module
    'cpan/ExtUtils-Install/t/lib/MakeMaker/Test/Setup/BFD.pm', # just a test module
    'cpan/ExtUtils-MakeMaker/t/lib/MakeMaker/Test/Setup/BFD.pm', # just a test module
    'cpan/ExtUtils-MakeMaker/t/lib/MakeMaker/Test/Setup/XS.pm',  # just a test module
    'cpan/IO-Compress/lib/File/GlobMapper.pm', # upstream needs to supply $VERSION
    'cpan/Math-BigInt/t/Math/BigFloat/Subclass.pm', # just a test module
    'cpan/Math-BigInt/t/Math/BigInt/BareCalc.pm',   # just a test module
    'cpan/Math-BigInt/t/Math/BigInt/Scalar.pm',     # just a test module
    'cpan/Math-BigInt/t/Math/BigInt/Subclass.pm',   # just a test module
    'cpan/Math-BigRat/t/Math/BigRat/Test.pm',       # just a test module
    'cpan/Module-Load/t/to_load/LoadIt.pm',         # just a test module
    'cpan/Module-Load/t/to_load/Must/Be/Loaded.pm', # just a test module
    'cpan/Module-Load-Conditional/t/test_lib/a/X.pm',          # just a test module
    'cpan/Module-Load-Conditional/t/test_lib/b/X.pm',          # just a test module
    'cpan/Module-Load-Conditional/t/to_load/Commented.pm',     # just a test module
    'cpan/Module-Load-Conditional/t/to_load/HereDoc.pm',       # just a test module
    'cpan/Module-Load-Conditional/t/to_load/InPod.pm',         # just a test module
    'cpan/Module-Load-Conditional/t/to_load/LoadIt.pm',        # just a test module
    'cpan/Module-Load-Conditional/t/to_load/MustBe/Loaded.pm', # just a test module
    'cpan/Module-Load-Conditional/t/to_load/NotMain.pm',       # just a test module
    'cpan/Module-Load-Conditional/t/to_load/NotX.pm',          # just a test module
    'cpan/Pod-Usage/t/inc/Pod/InputObjects.pm',     # just a test module
    'cpan/Pod-Usage/t/inc/Pod/Parser.pm',           # just a test module
    'cpan/Pod-Usage/t/inc/Pod/PlainText.pm',        # just a test module
    'cpan/Pod-Usage/t/inc/Pod/Select.pm',           # just a test module
    'cpan/podlators/t/lib/Test/Podlators.pm',       # just a test module
    'cpan/podlators/t/lib/Test/RRA.pm',             # just a test module
    'cpan/podlators/t/lib/Test/RRA/Config.pm',      # just a test module
    'cpan/version/t/coretests.pm', # just a test module
    'dist/Attribute-Handlers/demo/MyClass.pm', # it's just demonstration code
    'dist/Exporter/lib/Exporter/Heavy.pm',
    'dist/Module-CoreList/lib/Module/CoreList.pm',
    'dist/Module-CoreList/lib/Module/CoreList/Utils.pm',
    'lib/Carp/Heavy.pm',
    'lib/Config.pm',		# no version number but contents will vary
    'win32/FindExt.pm',
} = ();

# Files to skip just for particular version(s),
# usually due to some # mix-up

my %skip_versions = (
    # 'some/sample/file.pm' => [ '1.23', '1.24' ],
);

my $skip_dirs = qr|^t/lib|;

sub pm_file_from_xs {
    my $xs = shift;

    foreach my $try (sub {
                         # First try a .pm at the same level as the .xs file
                         # with the same basename
                         return shift =~ s/\.xs\z//r;
                     },
                     sub {
                         # Try for a (different) .pm at the same level, based
                         # on the directory name:
                         my ($path) = shift =~ m!^(.*)/!;
                         my ($last) = $path =~ m!([^-/]+)\z!;
                         return "$path/$last";
                     },
                     sub {
                         # Try to work out the extension's full package, and
                         # look for a .pm in lib/ based on that:
                         my ($path) = shift =~ m!^(.*)/!;
                         my ($last) = $path =~ m!([^/]+)\z!;
                         $last = 'List-Util' if $last eq 'Scalar-List-Utils';
                         $last =~ tr !-!/!;
                         return "$path/lib/$last";
                     }) {
        # For all cases, first look to see if the .pm file is generated.
        my $base = $try->($xs);
        return "${base}_pm.PL" if -f "${base}_pm.PL";
        return "${base}.pm" if -f "${base}.pm";
    }

    die "No idea which .pm file corresponds to '$xs', so aborting";
}

# Key is the .pm file from which we check the version.
# Value is a reference to an array of files to check for differences
# The trivial case is a pure perl module, where the array holds one element,
# the perl module's file. The "fun" comes with XS modules, and the real fun
# with XS modules with more than one XS file, and "interesting" layouts.

my %module_diffs;
my %dist_diffs;

foreach (`git --no-pager diff --name-only $tag_to_compare --diff-filter=ACMRTUXB`) {
    chomp;
    next unless m/^(.*)\//;
    my $this_dir = $1;
    next if $this_dir =~ $skip_dirs || exists $skip{$_};
    next if exists $upstream_files{$_};
    if (/\.pm\z/ || m|^lib/.*\.pl\z| || /_pm\.PL\z/) {
        push @{$module_diffs{$_}}, $_;
    } elsif (/\.xs\z/ && !/\bt\b/) {
        push @{$module_diffs{pm_file_from_xs($_)}}, $_;
    } elsif (!/\bt\b/ && /\.[ch]\z/ && m!^((?:dist|ext|cpan)/[^/]+)/!) {
       push @{ $dist_diffs{$1} }, $_;
    }
}

unless (%module_diffs || %dist_diffs) {
    print "1..1\nok 1 - No difference found\n" if $tap;
    exit;
}

printf "1..%d\n" => (keys(%module_diffs) + keys (%dist_diffs)) if $tap;
print "#\n# Comparing against $tag_to_compare ....\n#\n" if $tap;

my $count;
my $diff_cmd = "git --no-pager diff $tag_to_compare ";
my $q = ($^O eq 'MSWin32' || $^O eq 'VMS') ? '"' : "'";
my (@diff);
my %dist_bumped;

foreach my $pm_file (sort keys %module_diffs) {
    # git has already told us that the files differ, so no need to grab each as
    # a blob from git, and do the comparison ourselves.
    my $pm_version = eval {MM->parse_version($pm_file)};
    my $orig_pm_content = get_file_from_git($pm_file, $tag_to_compare);
    my $orig_pm_version = eval {MM->parse_version(\$orig_pm_content)};
    ++$count;

    if (!defined $orig_pm_version || $orig_pm_version eq 'undef') { # sigh
        print "ok $count - SKIP Can't parse \$VERSION in $pm_file\n"
          if $tap;

        # Behave like a version bump if the orig version could not be parsed,
        # but the current file could
        if (defined $pm_version && $pm_version ne 'undef' && $pm_file =~ m!^((?:dist|ext|cpan)/[^/]+)/!) {
            $dist_bumped{$1}++;
        }
    } elsif (!defined $pm_version || $pm_version eq 'undef') {
        my $nok = "not ok $count - in $pm_file version was $orig_pm_version, now unparsable\n";
        print $nok if $tap;
        print STDERR "# $nok\n";
    } elsif ($pm_version ne $orig_pm_version) { # good
        print "ok $count - $pm_file\n" if $tap;
        if ($pm_file =~ m!^((?:dist|ext|cpan)/[^/]+)/!) {
           $dist_bumped{$1}++;
        }
    } else {
        if ($tap) {
            print "#\n# " . '-' x 75 . "\n"
            . "# Version number ($pm_version) unchanged since"
            . " $tag_to_compare, but contents have changed:\n#\n";
            foreach (sort @{$module_diffs{$pm_file}}) {
                print "# $_" for `$diff_cmd $q$_$q`;
            }
            print "# " . '-' x 75 . "\n";

            if (exists $skip_versions{$pm_file}
                and grep $pm_version eq $_, @{$skip_versions{$pm_file}}) {
                print "ok $count - SKIP $pm_file version $pm_version\n";
            } else {
                my $nok = "not ok $count - $pm_file version $pm_version\n";
                print $nok;
                print STDERR "# $nok";
            }
        } else {
            push @diff, @{$module_diffs{$pm_file}};
            print "$pm_file version $pm_version\n";
        }
    }
}

foreach my $dist (sort keys %dist_diffs) {
    my $file_count = @{ $dist_diffs{$dist} };
    my $msg = $file_count == 1 ? "file was" : "files were";
    ++$count;

    if ($dist_bumped{$dist}) {
         print "ok $count - in $dist $file_count $msg modified and a version was bumped\n";
    } else {
        my $nok = "not ok $count - in $dist $file_count $msg modified but no versions were bumped\n";
        print "# No versions bumped in $dist but $file_count $msg modified\n";
        print "# $_\n" for (sort @{$dist_diffs{$dist}});
        print $nok if $tap;
        print STDERR "# $nok\n";
    }
}

sub get_file_from_git {
    my ($file, $tag) = @_;
    local $/;

    use open IN => ':raw';
    return scalar `git --no-pager show $tag:$file 2>$null`;
}

if ($diffs) {
    for (sort @diff) {
        print "\n";
        system "$diff_cmd $q$_$q";
    }
}
