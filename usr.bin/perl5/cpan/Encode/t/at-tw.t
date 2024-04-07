BEGIN {
    if (! -d 'blib' and -d 't'){ chdir 't' };
    unshift @INC,  '../lib';
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    unless (find PerlIO::Layer 'perlio') {
    print "1..0 # Skip: PerlIO was not built\n";
    exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # Skip: EBCDIC\n";
    exit 0;
    }
    $| = 1;
}

use strict;
use Test::More tests => 17;
use Encode;

no utf8; # we have raw Chinese encodings here

BEGIN {
    use_ok('Encode::TW');
}

# Since JP.t already tests basic file IO, we will just focus on
# internal encode / decode test here. Unfortunately, to test
# against all the UniHan characters will take a huge disk space,
# not to mention the time it will take, and the fact that Perl
# did not bundle UniHan.txt anyway.

# So, here we just test a typical snippet spanning multiple Unicode
# blocks, and hope it can point out obvious errors.

run_tests('Basic Big5 range', {
    'utf'	=> (
24093.39640.38525.20043.33495.35028.20846.65292.
26389.30343.32771.26352.20271.24248.65108.
25885.25552.35998.20110.23391.38508.20846.65292.
24799.24218.23493.21566.20197.38477.65108
    ),

    'big5'	=> (join('',
'«Ò°ª¶§¤§­]¸Ç¤¼¡A®Ó¬Ó¦Ò¤ê§B±e¡Q',
'Äá´£­s¤_©s³µ¤¼¡A±©©°±G§^¥H­°¡Q',
    )),

    'big5-hkscs'=> (join('',
'«Ò°ª¶§¤§­]¸Ç¤¼¡A®Ó¬Ó¦Ò¤ê§B±e¡Q',
'Äá´£­s¤_©s³µ¤¼¡A±©©°±G§^¥H­°¡Q',
    )),

    'cp950'	=> (join('',
'«Ò°ª¶§¤§­]¸Ç¤¼¡A®Ó¬Ó¦Ò¤ê§B±e¡Q',
'Äá´£­s¤_©s³µ¤¼¡A±©©°±G§^¥H­°¡Q',
    )),
});

run_tests('Hong Kong Extensions', {
    'utf'	=> (
24863.35613.25152.26377.20351.29992.32.80.101.114.108.32.
22021.26379.21451.65292.32102.25105.21707.22021.
25903.25345.12289.24847.35211.21644.40723.21237.
22914.26524.32232.30908.26377.20219.20309.37679.28431.
65292.35531.21578.35380.25105.21707.12290
    ),

    'big5-hkscs'	=> join('',
'·PÁÂ©Ò¦³¨Ï¥Î Perl ïªB¤Í¡Aµ¹§Ú’]ï¤ä«ù¡B·N¨£©M¹ªÀy',
'¦pªG½s½X¦³¥ô¦ó¿ùº|¡A½Ð§i¶D§Ú’]¡C'
    ),
});

sub run_tests {
    my ($title, $tests) = @_;
    my $utf = delete $tests->{'utf'};

    # $enc = encoding, $str = content
    foreach my $enc (sort keys %{$tests}) {
    my $str = $tests->{$enc};

    is(Encode::decode($enc, $str), $utf, "[$enc] decode - $title");
    is(Encode::encode($enc, $utf), $str, "[$enc] encode - $title");

    my $str2 = $str;
    my $utf8 = Encode::encode('utf-8', $utf);

    Encode::from_to($str2, $enc, 'utf-8');
    is($str2, $utf8, "[$enc] from_to => utf8 - $title");

    Encode::from_to($utf8, 'utf-8', $enc); # convert $utf8 as $enc
    is($utf8, $str,  "[$enc] utf8 => from_to - $title");
    }
}
