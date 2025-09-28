#!/usr/bin/perl
package GitUtils;
use strict;
use warnings;
use POSIX qw(strftime);

use base qw/Exporter/;
our @EXPORT_OK=qw(iso_time_with_dot gen_dot_patch);

sub iso_time_with_dot {
    strftime "%Y-%m-%d.%H:%M:%S",gmtime(shift||time)
}

# generate the contents of a .patch file for an arbitrary commitish, or for HEAD if none is supplied
# assumes the CWD is inside of a perl git repository. If the repository is bare then refs/heads/*
# is used to determine the branch. If the repository is not bare then refs/remotes/origin/* is used
# to determine the branch. (The assumption being that if its bare then this is running inside of
# the master git repo - if its not bare then it is a checkout which may not have all the branches)
sub gen_dot_patch {
    my $target= shift || 'HEAD';
    chomp(my ($git_dir, $is_bare, $sha1)=`git rev-parse --git-dir --is-bare-repository $target`);
    die "Not in a git repository!" if !$git_dir;
    $is_bare= "" if $is_bare and $is_bare eq 'false';

    # which branches to scan - the order here is important, the first hit we find we use
    # so if two branches can both reach a ref we want the right one first.
    my @branches=(
              'blead',
              'maint-5.10',
              'maint-5.8',
              'maint-5.8-dor',
              'maint-5.6',
              'maint-5.005',
              'maint-5.004',
              # and more generalized searches...
              'refs/heads/*',
              'refs/remotes/*',
              'refs/*',
    );
    my $reftype= $is_bare ? "heads" : "remotes/origin";
    my $branch;
    foreach my $name (@branches) {
        my $refs= $name=~m!^refs/! ? $name : "refs/$reftype/$name";
        my $cmd= "git name-rev --name-only --refs=$refs $sha1";
        chomp($branch= `$cmd`);
        last if $branch ne 'undefined';
    }
    for ($branch) {
        $_  ||= "error";            # hmm, we did not get /anything/ from name-rev?
        s!^\Q$reftype\E/!! ||       # strip off the reftype
        s!^refs/heads/!!   ||       # possible other places it was found
        s!^refs/remotes/!! ||       # ...
        s!^refs/!!;                 # might even be a tag or something weirdo...
        s![~^].*\z!!;               # strip off how far we are from the item
    }
    my $tstamp= iso_time_with_dot(`git log -1 --pretty="format:%ct" $sha1`);
    chomp(my $describe= `git describe $sha1`);
    join(" ", $branch, $tstamp, $sha1, $describe);
}

1;
