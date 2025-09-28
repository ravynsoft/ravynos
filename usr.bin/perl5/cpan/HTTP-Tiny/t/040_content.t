#!perl

use strict;
use warnings;

use Test::More qw[no_plan];
use lib 't';
use Util    qw[tmpfile rewind $CRLF $LF];
use HTTP::Tiny;

{
    no warnings 'redefine';
    sub HTTP::Tiny::Handle::can_read  { 1 };
    sub HTTP::Tiny::Handle::can_write { 1 };
}

{
    my $chunk    = join('', '0' .. '9', 'A' .. 'Z', 'a' .. 'z', '_', $LF) x 16; # 1024
    my $fh       = tmpfile();
    my $handle   = HTTP::Tiny::Handle->new(fh => $fh);
    my $nchunks  = 128;
    my $length   = $nchunks * length $chunk;

    {
        my $request = {
          cb => sub { $nchunks-- ? $chunk : undef },
          headers => { 'content-length' => $length }
        };
        my $got = $handle->write_content_body($request);
        is($got, $length, "written $length octets");
    }

    rewind($fh);

    {
        my $got = 0;
        $handle->read_content_body(sub { $got += length $_[0] }, {}, $length);
        is($got, $length, "read $length octets");
    }
}

