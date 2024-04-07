BEGIN {
    unshift @INC, 't/lib/';
}

use Filter::Simple::CodeNoComments qr/ok/ => 'not ok';

print "1..1\n";


# Perl bug #92436 (the second bug in the ticket)

sub method { $_[1] }
my $obj = bless[];

print $obj->method("ok 1\n");
