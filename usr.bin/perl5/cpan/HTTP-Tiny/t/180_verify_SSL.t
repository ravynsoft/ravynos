#!perl

use strict;
use warnings;
use Test::More 0.88;
use lib 't';

use HTTP::Tiny;

delete $ENV{PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT};

{
    my $ht = HTTP::Tiny->new();
    is($ht->verify_SSL, 1, "verify_SSL is 1 by default");
}

{
    my $ht = HTTP::Tiny->new(
        verify_SSL => 0
    );
    is($ht->verify_SSL, 0, "verify_SSL=>0 sets 0");
}

{
    my $ht = HTTP::Tiny->new(
        verify_ssl => 0
    );
    is($ht->verify_SSL, 0, "verify_ssl=>0 sets 0");
}

{
    my $ht = HTTP::Tiny->new(
        verify_SSL => 1,
        verify_ssl => 0
    );
    is($ht->verify_SSL, 1, "verify_SSL=>1 and verify_ssl=>0 sets 1");
}

{
    my $ht = HTTP::Tiny->new(
        verify_SSL => 0,
        verify_ssl => 1
    );
    is($ht->verify_SSL, 1, "verify_SSL=>0 and verify_ssl=>1 sets 1");
}

{
    my $ht = HTTP::Tiny->new(
        verify_SSL => 0,
        verify_ssl => 0
    );
    is($ht->verify_SSL, 0, "verify_SSL=>0 and verify_ssl=>0 sets 0");
}

{
    local $ENV{PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT} = "1";
    my $ht = HTTP::Tiny->new();
    is($ht->verify_SSL, 0, "PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT=1 changes verify_SSL default to 0");
}

{
    local $ENV{PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT} = "0";
    my $ht = HTTP::Tiny->new();
    is($ht->verify_SSL, 1, "PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT=0 keeps verify_SSL default at 1");
}

{
    local $ENV{PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT} = "False";
    my $ht = HTTP::Tiny->new();
    is($ht->verify_SSL, 1, "Unsupported PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT=False keeps verify_SSL default at 1");
}

{
    local $ENV{PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT} = "1";
    my $ht = HTTP::Tiny->new(verify_SSL=>1);
    is($ht->verify_SSL, 1, "PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT=1 does not override verify_SSL attribute set to 1");
}

{
    local $ENV{PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT} = "1";
    my $ht = HTTP::Tiny->new(
        verify_SSL => 1,
        verify_ssl => 1
    );
    is($ht->verify_SSL, 1, "PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT=1, verify_SSL=>1 and verify_ssl=>1 sets 1");
}

{
    local $ENV{PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT} = "1";
    my $ht = HTTP::Tiny->new(
        verify_SSL => 1,
        verify_ssl => 0
    );
    is($ht->verify_SSL, 1, "PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT=1, verify_SSL=>1 and verify_ssl=>0 sets 1");
}

{
    local $ENV{PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT} = "1";
    my $ht = HTTP::Tiny->new(
        verify_SSL => 0,
        verify_ssl => 0
    );
    is($ht->verify_SSL, 0, "PERL_HTTP_TINY_SSL_INSECURE_BY_DEFAULT=1, verify_SSL=>0 and verify_ssl=>0 sets 0");
}



done_testing;

