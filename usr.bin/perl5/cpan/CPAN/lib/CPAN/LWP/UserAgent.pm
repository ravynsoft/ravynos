# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::LWP::UserAgent;
use strict;
use vars qw(@ISA $USER $PASSWD $SETUPDONE);
use CPAN::HTTP::Credentials;
# we delay requiring LWP::UserAgent and setting up inheritance until we need it

$CPAN::LWP::UserAgent::VERSION = $CPAN::LWP::UserAgent::VERSION = "1.9601";


sub config {
    return if $SETUPDONE;
    if ($CPAN::META->has_usable('LWP::UserAgent')) {
        require LWP::UserAgent;
        @ISA = qw(Exporter LWP::UserAgent); ## no critic
        $SETUPDONE++;
    } else {
        $CPAN::Frontend->mywarn("  LWP::UserAgent not available\n");
    }
}

sub get_basic_credentials {
    my($self, $realm, $uri, $proxy) = @_;
    if ( $proxy ) {
        return CPAN::HTTP::Credentials->get_proxy_credentials();
    } else {
        return CPAN::HTTP::Credentials->get_non_proxy_credentials();
    }
}

sub no_proxy {
    my ( $self, $no_proxy ) = @_;
    return $self->SUPER::no_proxy( split(',',$no_proxy) );
}

# mirror(): Its purpose is to deal with proxy authentication. When we
# call SUPER::mirror, we really call the mirror method in
# LWP::UserAgent. LWP::UserAgent will then call
# $self->get_basic_credentials or some equivalent and this will be
# $self->dispatched to our own get_basic_credentials method.

# Our own get_basic_credentials sets $USER and $PASSWD, two globals.

# 407 stands for HTTP_PROXY_AUTHENTICATION_REQUIRED. Which means
# although we have gone through our get_basic_credentials, the proxy
# server refuses to connect. This could be a case where the username or
# password has changed in the meantime, so I'm trying once again without
# $USER and $PASSWD to give the get_basic_credentials routine another
# chance to set $USER and $PASSWD.

sub mirror {
    my($self,$url,$aslocal) = @_;
    my $result = $self->SUPER::mirror($url,$aslocal);
    if ($result->code == 407) {
        CPAN::HTTP::Credentials->clear_credentials;
        $result = $self->SUPER::mirror($url,$aslocal);
    }
    $result;
}

1;
