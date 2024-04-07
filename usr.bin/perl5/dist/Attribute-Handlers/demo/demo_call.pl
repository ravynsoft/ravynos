#! /usr/local/bin/perl -w

use Attribute::Handlers;

sub Call : ATTR {
	use Data::Dumper 'Dumper';
	print Dumper [ @_ ];
}


sub x : Call(some,data) { };
