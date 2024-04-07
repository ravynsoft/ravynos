# Adapted from script by Mark Lawrence (ref. rt.cpan.org #94830)

use strict;
use Digest::SHA qw(sha1);

package P1;
use vars qw(@ISA);
@ISA = ("Digest::SHA");

package main;

print "1..1\n";

my $data = 'a';
my $d = P1->new;
print "not " unless $d->add($data)->digest eq sha1($data);
print "ok 1\n";
