
use Test::More;

BEGIN {
    if( ord("A") == 193 ) {
	plan skip_all => 'No Encode::MIME::Header::ISO_2022_JP on EBCDIC Platforms';
    } else {
	plan tests => 14;
    }
}

use strict;
use Encode;

BEGIN{
    use_ok('Encode::MIME::Header::ISO_2022_JP');
}

require_ok('Encode::MIME::Header::ISO_2022_JP');

#  below codes are from mime.t in Jcode

my %mime = (
    "漢字、カタカナ、ひらがな"
     => "=?ISO-2022-JP?B?GyRCNEE7eiEiJSslPyUrJUohIiRSJGkkLCRKGyhC?=",
    "foo bar"
     => "foo bar",
    "漢字、カタカナ、ひらがなの混じったSubject Header."
     => "=?ISO-2022-JP?B?GyRCNEE7eiEiJSslPyUrJUohIiRSJGkkLCRKJE46LiQ4JEMkPxsoQlN1?=\n =?ISO-2022-JP?B?YmplY3Q=?= Header.",
);


for my $k (keys %mime){
    $mime{"$k\n"} = $mime{$k} . "\n";
}


for my $decoded (sort keys %mime){
    my $encoded = $mime{$decoded};

    my $header = Encode::encode('MIME-Header-ISO_2022_JP', decode('euc-jp', $decoded));
    my $utf8   = Encode::decode('MIME-Header', $header);

    is(encode('euc-jp', $utf8), $decoded);
    is($header, $encoded);
}

__END__
