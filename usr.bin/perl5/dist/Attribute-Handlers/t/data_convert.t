#!/usr/bin/perl -w

# Test attribute data conversion using examples from the docs

use Test::More tests => 8;

package LoudDecl;
use Attribute::Handlers;

sub Loud :ATTR {
    my ($package, $symbol, $referent, $attr, $data, $phase) = @_;

    ::is_deeply( $data, $referent->(), *{$symbol}{NAME} );
}


sub test1 :Loud(till=>ears=>are=>bleeding) {
    [qw(till ears are bleeding)]
}

sub test2 :Loud(['till','ears','are','bleeding']) {
    [[qw(till ears are bleeding)]]
}

sub test3 :Loud(qw/till ears are bleeding/) {
    [qw(till ears are bleeding)]
}

sub test4 :Loud(qw/my, ears, are, bleeding/) {
    [('my,', 'ears,', 'are,', 'bleeding')]
}

sub test5 :Loud(till,ears,are,bleeding) {
    [qw(till ears are bleeding)]
}

sub test6 :Loud(my,ears,are,bleeding) {
    'my,ears,are,bleeding';
}

sub test7 :Loud(qw/my ears are bleeding) {
    'qw/my ears are bleeding'; #'
}

sub test8 :Loud("turn it up to 11, man!") {
    ['turn it up to 11, man!'];
}
