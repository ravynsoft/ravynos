#
# $Id: Encoder.t,v 2.1 2013/09/14 07:51:59 dankogai Exp $
#

BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    $| = 1;
}

use strict;
#use Test::More 'no_plan';
use Test::More tests => 516;
use Encode::Encoder qw(encoder);
use MIME::Base64;
package Encode::Base64;
use parent 'Encode::Encoding';
__PACKAGE__->Define('base64');
use MIME::Base64;
sub encode{
    my ($obj, $data) = @_;
    return encode_base64($data);
}
sub decode{
    my ($obj, $data) = @_;
    return decode_base64($data);
}

package main;

my $e = encoder("foo", "ascii");
ok ($e->data("bar"));
is ($e->data, "bar");
ok ($e->encoding("latin1"));
is ($e->encoding, "iso-8859-1");

my $data = '';
for my $i (0..255){
    no warnings;
    $data .= chr($i);
    my $base64 = encode_base64($data);
    is(encoder($data)->base64, $base64, "encode");
    is(encoder($base64)->bytes('base64'), $data, "decode");
}

1;
__END__
