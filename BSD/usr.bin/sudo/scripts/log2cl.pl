#!/usr/bin/env perl
#
# SPDX-License-Identifier: ISC
#
# Copyright (c) 2017, 2020 Todd C. Miller <Todd.Miller@sudo.ws>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# Simple script to massage "git log" output into a GNU style ChangeLog.
# The goal is to emulate "hg log --template=changelog" via perl format.

use Getopt::Std;
use Text::Wrap;
use strict;
use warnings;

# Git log format: author date, author name, author email
#                 abbreviated commit hash
#		  raw commit body
my $format="%ad  %aN  <%aE>%n%h%n%B%n";

# Parse options and build up "git log" command
my @cmd = ( "git" );
my %opts;
getopts('b:R:', \%opts);
push(@cmd, "-b", $opts{"b"}) if exists $opts{"b"};
push(@cmd, "--git-dir", $opts{"R"}) if exists $opts{"R"};
push(@cmd, "log", "--log-size", "--name-only", "--date=short", "--format=$format", @ARGV);

open(LOG, '-|', @cmd) || die "$0: unable to run git log: $!";

my $hash;
my $body;
my @files;
my $key_date = "";
my $log_size = 0;
my @lines;

# Wrap like "hg log --template=changelog"
$Text::Wrap::columns = 77;

while (<LOG>) {
    chomp;
    if (/^log size (\d+)$/) {
	$log_size = $1;

	# Print previous entry if there is one
	print_entry($hash, $body, @files) if defined($hash);

	# Init new entry
	undef $hash;
	undef $body;
	undef @files;
	undef @lines;

	# Read entry and split on newlines
	read(LOG, my $buf, $log_size) ||
	    die "$0: unable to read $log_size bytes: $!\n";
	@lines = split(/\r?\n/, $buf);

	# Check for continued entry (duplicate Date + Author)
	$_ = shift(@lines);
	if ($_ ne $key_date) {
	    # New entry
	    print "$_\n\n";
	    $key_date = $_;
	}

	# Hash comes first
	$hash = shift(@lines);

	# Commit message body (multi-line)
	my $sep = "";
	foreach (@lines) {
	    last if $_ eq "--HG--";
	    if ($_ eq "") {
		$sep = "\n\n";
		next;
	    }
	    s/^\s+//;
	    s/\s+$//;
	    $body .= ${sep} . $_;
	    $sep = " ";
	}
    } else {
	# Not a log entry, must be the file list
	push(@files, $_) unless $_ eq "";
    }
}

# Print the last entry
print_entry($hash, $body, @files) if defined($hash);

exit(0);

sub print_entry
{
    my $hash = shift;
    my $body = shift;
    my $files = "* " . join(", ", @_) . ":";

    print wrap("\t", "\t", $files) . "\n";
    print wrap("\t", "\t", $body) . "\n";
    print "\t[$hash]\n\n";
}
