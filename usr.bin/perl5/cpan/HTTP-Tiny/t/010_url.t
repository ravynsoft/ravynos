#!perl

use strict;
use warnings;

use Test::More 0.86;
use HTTP::Tiny;

my @tests = (
    [ 'HtTp://Example.COM/',                 'http',  'example.com',    80, '/', '',          ],
    [ 'HtTp://Example.com:1024/',            'http',  'example.com',  1024, '/', '',          ],
    [ 'http://example.com',                  'http',  'example.com',    80, '/', '',          ],
    [ 'http://example.com:',                 'http',  'example.com',    80, '/', '',          ],
    [ 'http://foo@example.com:',             'http',  'example.com',    80, '/', 'foo',          ],
    [ 'http://foo:pass@example.com:',        'http',  'example.com',    80, '/', 'foo:pass',          ],
    [ 'http://@example.com:',                'http',  'example.com',    80, '/', '',          ],
    [ 'http://example.com?foo=bar',          'http',  'example.com',    80, '/?foo=bar', '',  ],
    [ 'http://example.com?foo=bar#fragment', 'http',  'example.com',    80, '/?foo=bar', '',  ],
    [ 'http://example.com/path?foo=bar',     'http',  'example.com',    80, '/path?foo=bar', '',  ],
    [ 'http:///path?foo=bar',                'http',  'localhost',      80, '/path?foo=bar', '',  ],
    [ 'HTTPS://example.com/',                'https', 'example.com',   443, '/', '',          ],
    [ 'http://[::]:1024',                    'http',  '[::]',         1024, '/', '',          ],
    [ 'xxx://foo/',                          'xxx',   'foo',         undef, '/', '',          ],
);

plan tests => scalar @tests;

for my $test (@tests) {
    my $url = shift(@$test);
    my $got = [ HTTP::Tiny->_split_url($url) ];
    my $exp = $test;
    is_deeply($got, $exp, "->split_url('$url')") or diag explain $got;
}


