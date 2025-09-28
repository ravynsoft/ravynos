#!/usr/bin/perl
use strict;
use warnings;
use File::Path;
use Cwd;

# This is a quick and dirty snapshot generator for the perl5.git.perl.org web page
# to use to generate the snapshot files. Yes it is ugly and contains hard coded crap
# and could use some love. But for this todo I am out of time now. -- Yves

$ENV{PATH}="/usr/local/bin:/bin/";

use POSIX qw(strftime);
sub isotime { strftime "%Y-%m-%d.%H:%M:%S",gmtime(shift||time) }

my ($abbr,$sha1,$tstamp);
$sha1= shift || "HEAD";
my $zip_root= $ENV{PERL_SNAPSHOT_ZIP_ROOT} || "/gitcommon/snapshot/tgz";
my $gitdir= shift || `git rev-parse --git-dir`
    or die "Not a git repo!\n";
chomp( $gitdir,$sha1);
my $workdir= $gitdir;
my $is_bare;
if ( $workdir =~ s!/\.git\z!! ) {

    chdir $workdir
        or die "Failed to chdir to $workdir\n";
} else {
    $is_bare= 1;
    chdir $workdir
        or die "Failed to chdir to bare repo $workdir\n";
}
#'die $workdir;

($sha1, $abbr,$tstamp)= split /\s+/, `git log --pretty='format:%H %h %ct' -1 $sha1`
    or die "Failed to parse '$sha1'\n";
chomp($sha1,$abbr,$tstamp);

#die "'$sha1','$abbr'\n";

my $path= join "/", $zip_root, substr($sha1,0,2), substr($sha1,0,4);
my $tar_file= "$sha1.tar.$$";
my $gz_file= "$sha1.tar.gz";
my $prefix= "perl-$abbr/";

if (!-e "$path/$gz_file") {
    mkpath $path if !-d $path;

    system("git archive --format=tar --prefix=$prefix $sha1 > $path/$tar_file");
    my @branches=map { $is_bare ? $_ : "origin/$_" } (
              'blead',
              'maint-5.10',
              'maint-5.8',
              'maint-5.8-dor',
              'maint-5.6',
              'maint-5.005',
              'maint-5.004',
    );
    my $branch;
    foreach my $b (@branches) {
        $branch= $b and last
            if `git log --pretty='format:%H' $b | grep $sha1`;
    }

    $branch ||= "unknown-branch";
    chomp(my $describe= `git describe`);
    chdir $path;
    {
        open my $fh,">","$path/$$.patch" or die "Failed to open $$.patch for writing\n";
        print $fh join(" ", $branch, isotime($tstamp), $sha1, $describe) . "\n";
        close $fh;
    }
    system("tar -f $tar_file --transform='s,^$$,$prefix,g' --owner=root --group=root --mode=664 --append $$.patch");
    unlink "$$.patch";
    system("gzip -S .gz -9 $tar_file");
    rename "$tar_file.gz", "$gz_file";
}
print "ok\tperl-$abbr.tar.gz\t$path/$gz_file", -t STDOUT ? "\n" :"";

