#!/usr/bin/perl -w

use strict;
use warnings;
use File::Find;
use File::Spec;

# only param: directory we shall collect stuff recursively in
my $dir = shift;
if (!defined($dir)) {
    die "Usage: metaproject.pl <directory>\n";
}

# collect all .kateprojects below the passed directory
my @projects;
sub find_project
{
    if ($File::Find::name =~ /^.+\/\.kateproject/) {
        push @projects, $File::Find::dir;
    }
}
find(\&find_project, $dir);

# construct kate project json file
open P, ">$dir/.kateproject";
print P "{\n";
print P "    \"files\": [\n";

# insert all projects we found
print P "        { \"projects\": [";
my $first = 1;
foreach my $project (sort @projects) {
    # compute relative project path, skip ourself
    my $relproject = File::Spec->abs2rel($project, $dir);
    $relproject =~ s/[\/\\]?\.kateproject$//;
    next if ($relproject eq "" || $relproject eq ".");

    # add to our projects list
    if ($first == 0) {
        print P ",";
    }
    print P "\n            \"".$relproject."\"";
    $first = 0;
}
print P " ]\n        } ]\n";

# be done ;)
print P "}\n";
close P;
