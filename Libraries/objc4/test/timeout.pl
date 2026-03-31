#!/usr/bin/perl -w

use strict;

my $usage = "timeout <seconds> <command ...>\n";
my $timeout = shift || die $usage;
alarm($timeout);
exec @ARGV;
die "exec failed: @ARGV";
