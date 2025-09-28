#!./perl

BEGIN {
    unless ( PerlIO::Layer->find('perlio') ) {
	print "1..0 # Skip: not perlio\n";
	exit 0;
    }
    require($ENV{PERL_CORE} ? "../../t/test.pl" : "./t/test.pl");
}

use utf8;

skip_all("EBCDIC platform; testing not core")
                                           if $::IS_EBCDIC && ! $ENV{PERL_CORE};

plan(tests => 4);

my $bytes =
            "\xce\x9c\xe1\xbd\xb7\xce\xb1\x20\xcf\x80\xe1\xbd\xb1\xcf\x80\xce".
            "\xb9\xce\xb1\x2c\x20\xce\xbc\xe1\xbd\xb0\x20\xcf\x80\xce\xbf\xce".
            "\xb9\xe1\xbd\xb0\x20\xcf\x80\xe1\xbd\xb1\xcf\x80\xce\xb9\xce\xb1".
            "\xcd\xbe\x0a";

if ($::IS_EBCDIC) {
    require($ENV{PERL_CORE} ? "../../t/charset_tools.pl" : "./t/charset_tools.pl");
    $bytes = byte_utf8a_to_utf8n($bytes)
}

open my $fh, ">:raw", 'io_utf8argv';
print $fh $bytes;
close $fh or die "close: $!";


use IO::Handle;

SKIP: {
    skip("PERL_UNICODE set", 2)
        if exists $ENV{PERL_UNICODE};

    @ARGV = ('io_utf8argv') x 2;
    is *ARGV->getline, $bytes,
        'getline (no open pragma) when magically opening ARGV';

    is join('',*ARGV->getlines), $bytes,
        'getlines (no open pragma) when magically opening ARGV';
}

use open ":std", ":utf8";

@ARGV = ('io_utf8argv') x 2;
is *ARGV->getline, "Μία πάπια, μὰ ποιὰ πάπια;\n",
  'getline respects open pragma when magically opening ARGV';

is join('',*ARGV->getlines), "Μία πάπια, μὰ ποιὰ πάπια;\n",
  'getlines respects open pragma when magically opening ARGV';

END {
  1 while unlink "io_utf8argv";
}
