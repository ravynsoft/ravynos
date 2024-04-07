#!/usr/bin/perl
# These two should go upon release to make the script Perl 5.005 compatible
use strict;
use warnings;

=head1 NAME

make_patchnum.pl - make patchnum

=head1 SYNOPSIS

  miniperl make_patchnum.pl

  perl make_patchnum.pl

=head1 DESCRIPTION

This program creates the files holding the information
about locally applied patches to the source code. The created
files are  F<git_version.h> and F<lib/Config_git.pl>.

=head2 F<lib/Config_git.pl>

Contains status information from git in a form meant to be processed
by the tied hash logic of Config.pm. It is actually optional,
although -V:git.\* will be uninformative without it.

C<git_version.h> contains similar information in a C header file
format, designed to be used by patchlevel.h. This file is obtained
from stock_git_version.h if miniperl is not available, and then
later on replaced by the version created by this script.

=head1 AUTHOR

Yves Orton, Kenichi Ishigaki, Max Maischein

=head1 COPYRIGHT

Same terms as Perl itself.

=cut

# from a -Dmksymlink target dir, I need to cd to the git-src tree to
# use git (like script does).  Presuming that's not unique, one fix is
# to follow Configure's symlink-path to run git.  Maybe GIT_DIR or
# path-args can solve it, if so we should advise here, I tried only
# very briefly ('cd -' works too).

my ($subcd, $srcdir);
our $opt_v = scalar grep $_ eq '-v', @ARGV;

BEGIN {
    my $root=".";
    while (!-e "$root/perl.c" and length($root)<100) {
        if ($root eq '.') {
            $root="..";
        } else {
            $root.="/..";
        }
    }
    die "Can't find toplevel" if !-e "$root/perl.c";
    sub path_to { "$root/$_[0]" } # use $_[0] if this'd be placed in toplevel.

    # test to see if we're a -Dmksymlinks target dir
    $subcd = '';
    $srcdir = $root;
    if (-l "$root/Configure") {
        $srcdir = readlink("$root/Configure");
        $srcdir =~ s/Configure//;
        $subcd = "cd $srcdir &&"; # activate backtick fragment
    }
}

sub read_file {
    my $file = path_to(@_);
    return "" unless -e $file;
    open my $fh, '<', $file
        or die "Failed to open for read '$file':$!";
    return do { local $/; <$fh> };
}

sub write_file {
    my ($file, $content) = @_;
    $file= path_to($file);
    open my $fh, '>', $file
        or die "Failed to open for write '$file':$!";
    print $fh $content;
    close $fh;
}

sub backtick {
    # only for git.  If we're in a -Dmksymlinks build-dir, we need to
    # cd to src so git will work .  Probably a better way.
    my $command = shift;
    if (wantarray) {
        my @result= `$subcd $command`;
        #warn "$subcd $command: \$?=$?\n" if $?;
        print "#> $subcd $command ->\n @result\n" if !$? and $opt_v;
        chomp @result;
        return @result;
    } else {
        my $result= `$subcd $command`;
        $result="" if ! defined $result;
        #warn "$subcd $command: \$?=$?\n" if $?;
        print "#> $subcd $command ->\n $result\n" if !$? and $opt_v;
        chomp $result;
        return $result;
    }
}

sub write_files {
    my %content= map { /WARNING: '([^']+)'/ || die "Bad mojo!"; $1 => $_ } @_;
    my @files= sort keys %content;
    my $files= join " and ", map { "'$_'" } @files;
    foreach my $file (@files) {
        if (read_file($file) ne $content{$file}) {
            print "Updating $files\n";
            write_file($_,$content{$_}) for @files;
            return 1;
        }
    }
    print "Reusing $files\n";
    return 0;
}

my $unpushed_commits = '    ';
my ($read, $branch, $snapshot_created, $commit_id, $describe)= ("") x 5;
my ($changed, $extra_info, $commit_title)= ("") x 3;

my $git_patch_file;
if (my $patch_file= read_file(".patch")) {
    ($branch, $snapshot_created, $commit_id, $describe) = split /\s+/, $patch_file;
    $extra_info = "git_snapshot_date='$snapshot_created'";
    $commit_title = "Snapshot of:";
}
elsif ($git_patch_file = read_file(".git_patch") and $git_patch_file !~ /\A\$Format:%H/) {
    chomp $git_patch_file;
    ($commit_id, my $commit_date, my $names)
        = split /\|/, $git_patch_file;

    my @names = split /,\s*/, $names;

    ($branch) = map m{^HEAD -> (.*)}, @names;
    if (!$branch) {
        ($branch) = map m{^(blead|maint/.*)}, @names;
    }
    if (!$branch) {
        ($branch) = map m{^tag: (.*)}, @names;
        $describe = $branch;
    }
    if (!$branch) {
        my ($pr) = map m{^refs/pull/([0-9]+)/}, @names;
        $branch = "pull-request-$pr";
    }
    if (!$branch) {
        $branch = $names[0] || $commit_id;
    }

    $describe ||= $commit_id;
    $extra_info = "git_commit_date='$commit_date'\n";
    $extra_info .= "git_snapshot_date='$commit_date'\n";
    $commit_title = "Snapshot of:";
}
elsif (-d "$srcdir/.git") {
    ($branch) = backtick("git symbolic-ref -q HEAD") =~ m#^refs/heads/(.+)$#;
    $branch //= "";
    my ($remote,$merge);
    if (length $branch) {
        $merge= backtick("git config branch.$branch.merge");
        $merge = "" unless $? == 0;
        $merge =~ s!^refs/heads/!!;
        $remote= backtick("git config branch.$branch.remote");
        $remote = "" unless $? == 0;
    }
    $commit_id = backtick("git rev-parse HEAD");
    $describe = backtick("git describe");
    my $commit_created = backtick(qq{git log -1 --pretty="format:%ci"});
    $extra_info = "git_commit_date='$commit_created'";
    backtick("git diff --no-ext-diff --quiet --exit-code");
    $changed = $?;
    unless ($changed) {
        backtick("git diff-index --cached --quiet HEAD --");
        $changed = $?;
    }

    if (length $branch && length $remote) {
        # git cherry $remote/$branch | awk 'BEGIN{ORS=","} /\+/ {print $2}' | sed -e 's/,$//'
        my $unpushed_commit_list =
            join ",", map { (split /\s/, $_)[1] }
            grep {/\+/} backtick("git cherry $remote/$merge");
        # git cherry $remote/$branch | awk 'BEGIN{ORS="\t\\\\\n"} /\+/ {print ",\"" $2 "\""}'
        $unpushed_commits =
            join "", map { ',"'.(split /\s/, $_)[1]."\"\t\\\n" }
            grep {/\+/} backtick("git cherry $remote/$merge");
        if (length $unpushed_commits) {
            $commit_title = "Local Commit:";
            my $ancestor = backtick("git rev-parse $remote/$merge");
            $extra_info = "$extra_info
git_ancestor='$ancestor'
git_remote_branch='$remote/$merge'
git_unpushed='$unpushed_commit_list'";
        }
    }
    if ($changed) {
        $commit_title =  "Derived from:";
    }
    $commit_title ||= "Commit id:";
}

# we extract the filename out of the warning header, so don't mess with that
write_files(<<"EOF_HEADER", <<"EOF_CONFIG");
/**************************************************************************
* WARNING: 'git_version.h' is automatically generated by make_patchnum.pl
*          DO NOT EDIT DIRECTLY - edit make_patchnum.pl instead
***************************************************************************/
@{[$describe ? "#define PERL_PATCHNUM \"$describe\"" : ()]}
#define PERL_GIT_UNPUSHED_COMMITS\t\t\\
$unpushed_commits/*leave-this-comment*/
@{[$changed ? "#define PERL_GIT_UNCOMMITTED_CHANGES" : ()]}
EOF_HEADER
######################################################################
# WARNING: 'lib/Config_git.pl' is generated by make_patchnum.pl
#          DO NOT EDIT DIRECTLY - edit make_patchnum.pl instead
######################################################################
\$Config::Git_Data=<<'ENDOFGIT';
git_commit_id='$commit_id'
git_describe='$describe'
git_branch='$branch'
git_uncommitted_changes='$changed'
git_commit_id_title='$commit_title'
$extra_info
ENDOFGIT
EOF_CONFIG
# ex: set ts=8 sts=4 sw=4 et ft=perl:
