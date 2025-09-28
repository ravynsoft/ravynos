#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bFile\/Glob\b/i) {
        print "1..0\n";
        exit 0;
    }
}
use strict;
use warnings;
# Test::More needs threads pre-loaded
use if $Config{useithreads}, 'threads';
use Test::More;

BEGIN {
    if (! $Config{'useithreads'}) {
        plan skip_all => "Perl not compiled with 'useithreads'";
    }
}

use File::Temp qw(tempdir);
use File::Spec qw();
use File::Glob qw(csh_glob);

my($dir) = tempdir(CLEANUP => 1)
    or die "Could not create temporary directory";

my @temp_files = qw(1_file.tmp 2_file.tmp 3_file.tmp);
for my $file (@temp_files) {
    open my $fh, ">", File::Spec->catfile($dir, $file)
        or die "Could not create file $dir/$file: $!";
    close $fh;
}
my $cwd = Cwd::cwd();
chdir $dir
    or die "Could not chdir to $dir: $!";

sub do_glob { scalar csh_glob("*") }
# Stablish some glob state
my $first_file = do_glob();
is($first_file, $temp_files[0]);

my @files;
push @files, threads->create(\&do_glob)->join() for 1..5;
is_deeply(
    \@files,
    [($temp_files[1]) x 5],
    "glob() state is cloned for new threads"
);

@files = threads->create({'context' => 'list'},
    sub {
        return do_glob(), threads->create(\&do_glob)->join()
    })->join();

is_deeply(
    \@files,
    [@temp_files[1,2]],
    "..and for new threads inside threads"
);

my $second_file = do_glob();
is($second_file, $temp_files[1], "state doesn't leak from threads");

chdir $cwd
    or die "Could not chdir back to $cwd: $!";

done_testing;
