BEGIN {
    chdir '..' if -d '../dist';
    push @INC, "lib";
    require './t/test.pl';
}

use strict;
use warnings;

# Test that t/TEST and t/harness test the same files, and that all the
# test files (.t files) listed in MANIFEST are tested by both.
#
# We enabled the various special tests as this simplifies our MANIFEST
# parsing.  In theory if someone adds a new test directory this should
# tell us if one of the files does not know about it.

plan tests => 3;

my (%th, %tt, %all);
$ENV{PERL_TORTURE_TEST} = 1;
$ENV{PERL_TEST_MEMORY} = 1;
$ENV{PERL_BENCHMARK} = 1;

for my $file (`$^X t/harness -dumptests`) {
    chomp $file;
    $all{$file}++;
    $th{$file}++;
}

for my $file (`$^X t/TEST -dumptests`) {
    chomp $file;
    $all{$file}++;
    delete $th{$file} or $tt{$file}++;
}

is(0+keys(%th), 0, "t/harness will not test anything that t/TEST does not")
    or print STDERR map { "# t/harness: $_\n" } sort keys %th;
is(0+keys(%tt), 0, "t/TEST will not test aything that t/harness does not")
    or print STDERR map { "# tTEST: $_\n" } sort keys %tt;

sub get_extensions {
    my %extensions;
    open my $ifh, "<", "config.sh"
        or die "Failed to open 'config.sh': $!";
    while (<$ifh>) {
        if (/^extensions='([^']+)'/) {
            my $list = $1;
            NAME:
            foreach my $name (split /\s+/, $list) {
                $name = "PathTools" if $name eq "Cwd";
                $name = "Scalar/List/Utils" if $name eq "List/Util";
                my $sub_dir = $name;
                $sub_dir =~ s!/!-!g;
                foreach my $dir (qw(cpan dist ext)) {
                    if (-e "$dir/$sub_dir") {
                        $extensions{"$dir/$sub_dir"} = $name;
                        next NAME;
                    }
                }
                die "Could not find '$name'\n";
            }
            last;
        }
    }
    close $ifh;
    return \%extensions;
}

sub find_in_manifest_but_missing {
    my $extension = get_extensions();
    my %missing;
    my $is_os2 = $^O eq "os2";
    my $is_win32 = $^O eq "MSWin32";
    open my $ifh, "<", "MANIFEST"
        or die "Failed to open 'MANIFEST' for read: $!";
    while (<$ifh>) {
        chomp;
        my ($file, $descr) = split /\t+/, $_;
        next if $file eq "t/test.pl"
             or $file!~m!(?:\.t|/test\.pl)\z!
             or (!$is_os2 and $file=~m!^(?:t/)?os2/!)
             or (!$is_win32 and $file=~m!^(?:t/)?win32/!);
        if ($file=~m!^(cpan|dist|ext/[^/]+)!) {
            my $path = $1;
            next unless $extension->{$path};
        }
        $missing{$file}++ unless $all{$file};
    }
    close $ifh;
    return \%missing;
}
my $missing = find_in_manifest_but_missing();
is(0+keys(%$missing), 0, "Nothing in manifest that we wouldn't test")
    or print STDERR map { "# $_\n" } sort keys %$missing;
