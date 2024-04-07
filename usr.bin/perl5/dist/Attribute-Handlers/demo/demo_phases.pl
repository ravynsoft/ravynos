#! /usr/local/bin/perl -w

use Attribute::Handlers;
use Data::Dumper 'Dumper';

sub UNIVERSAL::Beginner : ATTR(SCALAR,BEGIN,END)
	{ print STDERR "Beginner: ", Dumper \@_}

sub UNIVERSAL::Checker : ATTR(CHECK,SCALAR)
	{ print STDERR "Checker: ", Dumper \@_}

sub UNIVERSAL::Initer : ATTR(SCALAR,INIT)
	{ print STDERR "Initer: ", Dumper \@_}

package Other;

my $x :Initer(1) :Checker(2) :Beginner(3);
my $y :Initer(4) :Checker(5) :Beginner(6);
