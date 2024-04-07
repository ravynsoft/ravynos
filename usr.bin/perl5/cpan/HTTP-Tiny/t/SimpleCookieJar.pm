package SimpleCookieJar;

use strict;
use warnings;

sub new {
    my $class = shift;
    return bless {} => $class;
}

sub add {
    my ($self, $url, $cookie) = @_;
    
    my ($kv) = split qr/;/, $cookie;
    my ($k, $v) = split qr/\s*=\s*/, $kv, 2;

    $self->{$url}{$k} = $v;
}

sub cookie_header {
    my ($self, $url) = @_;

    my $cookies = $self->{$url}
        or return '';

    return join( "; ", map{ "$_=$cookies->{$_}" } sort keys %$cookies );
}

1;
