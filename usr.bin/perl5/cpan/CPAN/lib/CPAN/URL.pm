# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::URL;
use overload '""' => "as_string", fallback => 1;
# accessors: TEXT(the url string), FROM(DEF=>defaultlist,USER=>urllist),
# planned are things like age or quality

use vars qw(
            $VERSION
);
$VERSION = "5.5";

sub new {
    my($class,%args) = @_;
    bless {
           %args
          }, $class;
}
sub as_string {
    my($self) = @_;
    $self->text;
}
sub text {
    my($self,$set) = @_;
    if (defined $set) {
        $self->{TEXT} = $set;
    }
    $self->{TEXT};
}

1;
