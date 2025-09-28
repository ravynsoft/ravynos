# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Exception::blocked_urllist;
use strict;
use overload '""' => "as_string";

use vars qw(
            $VERSION
);
$VERSION = "1.001";


sub new {
    my($class) = @_;
    bless {}, $class;
}

sub as_string {
    my($self) = shift;
    if ($CPAN::Config->{connect_to_internet_ok}) {
        return qq{

You have not configured a urllist for CPAN mirrors. Configure it with

    o conf init urllist

};
    } else {
        return qq{

You have not configured a urllist and do not allow connections to the
internet to get a list of mirrors.  If you wish to get a list of CPAN
mirrors to pick from, use this command

    o conf init connect_to_internet_ok urllist

If you do not wish to get a list of mirrors and would prefer to set
your urllist manually, use just this command instead

    o conf init urllist

};
    }
}

1;
