#!perl

use strict;
use warnings;

use Amiga::ARexx qw(DoRexx);

my ($result,$rc,$rc2) = DoRexx("WORKBENCH","HELP");

print $result , "\n" , $rc, "\n", $rc2 , "\n";

($result,$rc,$rc2) = DoRexx("WORKBENCH","NOHELP");

print $result , "\n" , $rc, "\n", $rc2 , "\n";
