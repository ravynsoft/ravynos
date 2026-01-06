#!/usr/bin/perl

use strict;
use Cwd;

my $dir = getcwd;
#my $xxx = $ARGV[1];

while(<>)
{
	# get stabs lines that match "NNNNNNN - xxx"
    if(m/^([0-9a-f]+) - ([0-9a-f]+) (.*?)$/)
    {
		# replace any occurances of cwd path with $CWD
		my $line = $3;
        if($line =~ m/(.*?)$dir(.*?)$/)
		{
			$line = $1 . "CWD" . $2;
		}
		
		printf "$line\n";
	}
}


