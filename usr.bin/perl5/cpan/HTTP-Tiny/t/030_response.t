#!perl

use strict;
use warnings;

use Test::More qw[no_plan];
use lib 't';
use Util    qw[tmpfile rewind $CRLF $LF];
use HTTP::Tiny;

sub _header {
  return [ @{$_[0]}{qw/status reason headers protocol/} ]
}

{
    no warnings 'redefine';
    sub HTTP::Tiny::Handle::can_read  { 1 };
    sub HTTP::Tiny::Handle::can_write { 1 };
}

{
    my $response = join $CRLF, 'HTTP/1.1 200 OK', 'Foo: Foo', 'Bar: Bar', '', '';
    my $fh       = tmpfile($response);
    my $handle   = HTTP::Tiny::Handle->new(fh => $fh);
    my $exp      = [ 200, 'OK', { foo => 'Foo', bar => 'Bar' }, 'HTTP/1.1' ];
    is_deeply(_header($handle->read_response_header), $exp, "->read_response_header CRLF");
}

{
    my $response = join $LF, 'HTTP/1.1 200 OK', 'Foo: Foo', 'Bar: Bar', '', '';
    my $fh       = tmpfile($response);
    my $handle   = HTTP::Tiny::Handle->new(fh => $fh);
    my $exp      = [ 200, 'OK', { foo => 'Foo', bar => 'Bar' }, 'HTTP/1.1' ];
    is_deeply(_header($handle->read_response_header), $exp, "->read_response_header LF");
}

{
    # broken status-line
    my $response = join $LF, "HTTP/08.15 66x   Foo\nbar", 'Foo: Foo', 'Bar: Bar', '', '';
    my $fh       = tmpfile($response);
    my $handle   = HTTP::Tiny::Handle->new(fh => $fh);
    my $res      = eval{ $handle->read_response_header };
    my $err      = $@;
    like $err, qr/Malformed Status-Line: /, '->read_response_header diagnoses malformed status line';
}

{
    my $response = join $LF, "HTTP/2.0 200 Okish", 'Foo: Foo', 'Bar: Bar', '', '';
    my $fh       = tmpfile($response);
    my $handle   = HTTP::Tiny::Handle->new(fh => $fh);
    my $res      = eval{ $handle->read_response_header };
    my $err      = $@;
    like $err, qr/Unsupported HTTP protocol: /, '->read_response_header unsupported HTTP protocol';
}

{
    # strict RFC7230#3.1.2 compliance, require space after code
    my $response = join $LF, 'HTTP/1.1 200 ', 'Foo: Foo', 'Bar: Bar', '', '';
    my $fh       = tmpfile($response);
    my $handle   = HTTP::Tiny::Handle->new(fh => $fh);
    my $exp      = [ 200, '', { foo => 'Foo', bar => 'Bar' }, 'HTTP/1.1' ];
    is_deeply(_header($handle->read_response_header), $exp, "->read_response_header empty phrase preceded by SP");
}

{
    # practical RFC7230#3.1.2 interpretation, require space after code
    # only if there is a reason-phrase
    my $response = join $LF, 'HTTP/1.1 200', 'Foo: Foo', 'Bar: Bar', '', '';
    my $fh       = tmpfile($response);
    my $handle   = HTTP::Tiny::Handle->new(fh => $fh);
    my $exp      = [ 200, '', { foo => 'Foo', bar => 'Bar' }, 'HTTP/1.1' ];
    is_deeply(_header($handle->read_response_header), $exp, "->read_response_header empty phrase without preceding SP");
}
