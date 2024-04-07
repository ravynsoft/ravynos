#!perl

use strict;
use warnings;

use File::Basename;
use Test::More 0.88;

use lib 't';
use Util qw[ monkey_patch ];
use HTTP::Tiny;

BEGIN {
    monkey_patch();
}


# Require a true value
for my $proxy (undef, "", 0){
    no warnings 'uninitialized';
    local $ENV{all_proxy};
    local $ENV{ALL_PROXY};
    local $ENV{http_proxy} = $proxy;
    my $c = HTTP::Tiny->new();
    ok(!defined $c->http_proxy);
}

# trailing / is optional
for my $proxy ("http://localhost:8080/", "http://localhost:8080"){
    local $ENV{http_proxy} = $proxy;
    my $c = HTTP::Tiny->new();
    is($c->http_proxy, $proxy);
}

# http_proxy must be http://<host>:<port> format
{
    local $ENV{http_proxy} = "localhost:8080";
    eval {
        my $c = HTTP::Tiny->new();
    };
    like($@, qr{http_proxy URL must be in format http\[s\]://\[auth\@\]<host>:<port>/});
}

# Explicitly disable proxy
{
    local $ENV{all_proxy} = "http://localhost:8080";
    local $ENV{http_proxy} = "http://localhost:8080";
    local $ENV{https_proxy} = "http://localhost:8080";
    my $c = HTTP::Tiny->new(
        proxy => undef,
        http_proxy => undef,
        https_proxy => undef,
    );
    ok(!defined $c->proxy, "proxy => undef disables ENV proxy");
    ok(!defined $c->http_proxy, "http_proxy => undef disables ENV proxy");
    ok(!defined $c->https_proxy, "https_proxy => undef disables ENV proxy");
}

# case variations
for my $var ( qw/http_proxy https_proxy all_proxy/ ) {
    my $proxy = "http://localhost:8080";
    for my $s ( uc($var), lc($var) ) {
        local $ENV{$s} = $proxy;
        my $c = HTTP::Tiny->new();
        my $m = ($s =~ /all/i) ? 'proxy' : lc($s);
        is( $c->$m, $proxy, "set $m from $s" );
    }
}

# ignore HTTP_PROXY with REQUEST_METHOD
{
    # in case previous clean-up failed for some reason
    delete local @ENV{'http_proxy', 'https_proxy', 'all_proxy',
                      'HTTP_PROXY', 'HTTPS_PROXY', 'ALL_PROXY'};

    local $ENV{HTTP_PROXY} = "http://localhost:8080";
    local $ENV{REQUEST_METHOD} = 'GET';
    my $c = HTTP::Tiny->new();
    ok(!defined $c->http_proxy,
        "http_proxy not set from HTTP_PROXY if REQUEST_METHOD set");

}

# allow CGI_HTTP_PROXY with REQUEST_METHOD
{
    local $ENV{HTTP_PROXY} = "http://localhost:8080";
    local $ENV{CGI_HTTP_PROXY} = "http://localhost:9090";
    local $ENV{REQUEST_METHOD} = 'GET';
    my $c = HTTP::Tiny->new();
    is($c->http_proxy, "http://localhost:9090",
        "http_proxy set from CGI_HTTP_PROXY if REQUEST_METHOD set");
}

done_testing();
