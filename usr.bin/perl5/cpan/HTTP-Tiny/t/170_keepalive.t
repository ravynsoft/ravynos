#!perl

use strict;
use warnings;
use File::Basename;
use Test::More 0.88;
use lib 't';
use Util qw[
    tmpfile monkey_patch dir_list clear_socket_source set_socket_source
    $CRLF
];
use HTTP::Tiny;
our $can_read;

BEGIN {
    no warnings qw/redefine once/;
    monkey_patch();
    *HTTP::Tiny::Handle::can_read = sub { $can_read++ };
}

my $response = <<'RESPONSE';
HTTP/1.1 200 OK
Date: Thu, 03 Feb 1994 00:00:00 GMT
Content-Type: text/html
Content-Length: 10

0123456789

RESPONSE

trim($response);

my $h;

new_ht();
test_ht( "Keep-alive", 1, 'http://foo.com' );

new_ht();
test_ht( "Different scheme", 0, 'https://foo.com' );

new_ht();
test_ht( "Different host", 0, 'http://bar.com' );

new_ht();
test_ht( "Different port", 0, 'http://foo.com:8000' );

new_ht();
$h->timeout(30);
test_ht( "Different timeout", 0, 'http://foo.com' );

new_ht();
$h->timeout(60);
test_ht( "Same timeout", 1, 'http://foo.com' );

new_ht();
$h->default_headers({ 'X-Foo' => 'Bar' });
test_ht( "Default headers change", 1, 'http://foo.com' );

new_ht();
$h->{handle}->close;
test_ht( "Socket closed", 0, 'http://foo.com' );

for my $file ( dir_list( "corpus", qr/^keepalive/ ) ) {
    my $label = basename($file);
    my $data = do { local ( @ARGV, $/ ) = $file; <> };
    my ( $title, $ok, $response ) = map { trim($_) } split /--+/, $data;
    new_ht();
    clear_socket_source();
    set_socket_source( tmpfile(), tmpfile($response) );
    $h->request( 'POST', 'http://foo.com', { content => 'xx' } );
    is !!$h->{handle}, !!$ok, "$label - $title";
}

sub test_ht {
    my $title  = shift;
    my $result = !!shift();
    my $url    = shift;

    clear_socket_source();
    set_socket_source( tmpfile(), tmpfile($response) );
    $can_read = 0 if $result;
    my $old = $h->{handle} || 'old';
    $h->request( 'POST', $url, { content => 'xx' } );
    my $new = $h->{handle} || 'new';
    is $old eq $new, $result, $title;
}

sub new_ht {
    $h = HTTP::Tiny->new( keep_alive => 1, @_ );
    $can_read = 1;
    clear_socket_source();
    set_socket_source( tmpfile(), tmpfile($response) );
    $h->request( 'POST', 'http://foo.com' );
}

sub trim { $_[0] =~ s/^\s+//; $_[0] =~ s/\s+$//; return $_ }

done_testing;

