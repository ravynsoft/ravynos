# tr.t
$|=1;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    if (is_miniperl()) {
	eval 'require utf8';
        if ($@) { skip_all("miniperl, no 'utf8'") }
    }
}

use utf8;
require Config;

plan tests => 315;

# Test this first before we extend the stack with other operations.
# This caused an asan failure due to a bad write past the end of the stack.
eval { my $x; die  1..127, $x =~ y/// };

$_ = "abcdefghijklmnopqrstuvwxyz";

tr/a-z/A-Z/;

is($_, "ABCDEFGHIJKLMNOPQRSTUVWXYZ",    'uc');

tr/A-Z/a-z/;

is($_, "abcdefghijklmnopqrstuvwxyz",    'lc');

tr/b-y/B-Y/;
is($_, "aBCDEFGHIJKLMNOPQRSTUVWXYz",    'partial uc');

tr/a-a/AB/;
is($_, "ABCDEFGHIJKLMNOPQRSTUVWXYz",    'single char range a-a');

eval 'tr/a/\N{KATAKANA LETTER AINU P}/;';
like $@,
     qr/\\N\{KATAKANA LETTER AINU P\} must not be a named sequence in transliteration operator/,
     "Illegal to tr/// named sequence";

eval 'tr/\x{101}-\x{100}//;';
like $@,
     qr/Invalid range "\\x\{0101\}-\\x\{0100\}" in transliteration operator/,
     "UTF-8 range with min > max";

$_ = "0123456789";
tr/10/01/;
is($_, "1023456789",    'swapping 0 and 1');
tr/01/10/;
is($_, "0123456789",    'swapping 0 and 1');

# Test /c and variants, with all the search and replace chars being
# non-utf8, but with both non-utf8 and utf8 strings.

SKIP: {
    my $all255            = join '', map chr, 0..0xff;
    my $all255_twice      = join '', map chr, map { ($_, $_) } 0..0xff;
    my $plus              = join '', map chr, 0x100..0x11f;
    my $plus_twice        = join '', map chr, map { ($_, $_) } 0x100..0x11f;
    my $all255_plus       = $all255 . $plus;
    my $all255_twice_plus = $all255_twice . $plus_twice;
    my ($c, $s);

    # length(replacement) == 0
    # non-utf8 string

    $s = $all255;
    $c = $s =~ tr/\x40-\xbf//c;
    is $s, $all255, "/c   ==0";
    is $c, 0x80, "/c   ==0  count";

    $s = $all255;
    $c = $s =~ tr/\x40-\xbf//cd;
    is $s, join('', map chr, 0x40.. 0xbf), "/cd  ==0";
    is $c, 0x80, "/cd  ==0  count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf//cs;
    is $s, join('', map chr,
                0x00..0x3f,
                (map  { ($_, $_) } 0x40..0xbf),
                0xc0..0xff,
            ),
        "/cs  ==0";
    is $c, 0x100, "/cs  ==0  count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf//csd;
    is $s, join('', map chr, (map  { ($_, $_) } 0x40..0xbf)), "/csd ==0";
    is $c, 0x100, "/csd ==0  count";


    # length(search) > length(replacement)
    # non-utf8 string

    $s = $all255;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x2f/c;
    is $s, join('', map chr,
                0x80..0xbf,
                0x40..0xbf,
                0x00..0x2f,
                ((0x2f) x 16),
            ),
        "/c   >";
    is $c, 0x80, "/c   >  count";

    $s = $all255;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x2f/cd;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x2f),
        "/cd  >";
    is $c, 0x80, "/cd  >  count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x2f/cs;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
            ),
        "/cs  >";
    is $c, 0x100, "/cs  >  count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x2f/csd;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
            ),
        "/csd >";
    is $c, 0x100, "/csd >  count";


    # length(search) == length(replacement)
    # non-utf8 string

    $s = $all255;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x3f/c;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x3f), "/c   ==";
    is $c, 0x80, "/c   == count";

    $s = $all255;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x3f/cd;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x3f), "/cd  ==";
    is $c, 0x80, "/cd  == count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x3f/cs;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x3f,
            ),
        "/cs  ==";
    is $c, 0x100, "/cs  == count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x3f/csd;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x3f,
            ),
        "/csd ==";
    is $c, 0x100, "/csd == count";

    # length(search) == length(replacement) - 1
    # non-utf8 string


    $s = $all255;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x30/c;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x2f, 0xf0..0xff),
        "/c   =-";
    is $c, 0x70, "/c   =-  count";

    $s = $all255;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x30/cd;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x2f, 0xf0..0xff),
        "/cd  =-";
    is $c, 0x70, "/cd  =-  count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x30/cs;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
                (map  { ($_, $_) } 0xf0..0xff),
            ),
        "/cs  =-";
    is $c, 0xe0, "/cs  =-  count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x30/csd;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
                (map  { ($_, $_) } 0xf0..0xff),
            ),
        "/csd =-";
    is $c, 0xe0, "/csd =-  count";

    # length(search) < length(replacement)
    # non-utf8 string

    $s = $all255;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x3f/c;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x2f, 0xf0..0xff),
        "/c   <";
    is $c, 0x70, "/c   <  count";

    $s = $all255;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x3f/cd;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x2f, 0xf0..0xff),
        "/cd  <";
    is $c, 0x70, "/cd  <  count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x3f/cs;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
                (map  { ($_, $_) } 0xf0..0xff),
            ),
        "/cs  <";
    is $c, 0xe0, "/cs  <  count";

    $s = $all255_twice;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x3f/csd;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
                (map  { ($_, $_) } 0xf0..0xff),
            ),
        "/csd <";
    is $c, 0xe0, "/csd <  count";


    # length(replacement) == 0
    # with some >= 0x100 utf8 chars in the string to be modified

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf//c;
    is $s, $all255_plus, "/c   ==0U";
    is $c, 0xa0, "/c   ==0U  count";

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf//cd;
    is $s, join('', map chr, 0x40..0xbf), "/cd  ==0U";
    is $c, 0xa0, "/cd  ==0U  count";

    $s = $all255_twice_plus;
    $c = $s =~ tr/\x40-\xbf//cs;
    is $s, join('', map chr,
                0x00..0x3f,
                (map  { ($_, $_) } 0x40..0xbf),
                0xc0..0x11f,
            ),
        "/cs  ==0U";
    is $c, 0x140, "/cs  ==0U  count";

    $s = $all255_twice_plus;
    $c = $s =~ tr/\x40-\xbf//csd;
    is $s, join('', map chr, (map  { ($_, $_) } 0x40..0xbf)), "/csd ==0U";
    is $c, 0x140, "/csd ==0U  count";

    # length(search) > length(replacement)
    # with some >= 0x100 utf8 chars in the string to be modified

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x2f/c;
    is $s, join('', map chr,
                0x80..0xbf,
                0x40..0xbf,
                0x00..0x2f,
                ((0x2f) x 48),
            ),
        "/c   >U";
    is $c, 0xa0, "/c   >U count";

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x2f/cd;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x2f),
        "/cd  >U";
    is $c, 0xa0, "/cd  >U count";

    $s = $all255_twice_plus . "\x3f\x3f\x{200}\x{300}";
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x2f/cs;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
                0xbf,
                0x2f,
            ),
        "/cs  >U";
    is $c, 0x144, "/cs  >U count";

    $s = $all255_twice_plus;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x2f/csd;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
            ),
        "/csd >U";
    is $c, 0x140, "/csd >U count";

    # length(search) == length(replacement)
    # with some >= 0x100 utf8 chars in the string to be modified

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x3f/c;
    is $s, join('', map chr,
                0x80..0xbf,
                0x40..0xbf,
                0x00..0x3f,
                ((0x3f) x 32),
            ),
        "/c   ==U";
    is $c, 0xa0, "/c   ==U count";

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x3f/cd;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x3f), "/cd ==U";
    is $c, 0xa0, "/cd  ==U count";

    $s = $all255_twice_plus . "\x3f\x3f\x{200}\x{300}";
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x3f/cs;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x3f,
                0xbf,
                0x3f,
            ),
        "/cs  ==U";
    is $c, 0x144, "/cs  ==U count";

    $s = $all255_twice_plus;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x3f/csd;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x3f,
            ),
        "/csd ==U";
    is $c, 0x140, "/csd ==U count";


    # length(search) == length(replacement) - 1
    # with some >= 0x100 utf8 chars in the string to be modified

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x40/c;
    is $s, join('', map chr,
                0x80..0xbf,
                0x40..0xbf,
                0x00..0x40,
                ((0x40) x 31),
            ),
        "/c   =-U";
    is $c, 0xa0, "/c   =-U count";

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x40/cd;
    is $s, join('', map chr, 0x80..0xbf, 0x40..0xbf, 0x00..0x40), "/cd =-U";
    is $c, 0xa0, "/cd  =-U count";

    $s = $all255_twice_plus . "\x3f\x3f\x{200}\x{300}";
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x40/cs;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x40,
                0xbf,
                0x40,
            ),
        "/cs  =-U";
    is $c, 0x144, "/cs  =-U count";

    $s = $all255_twice_plus;
    $c = $s =~ tr/\x40-\xbf/\x80-\xbf\x00-\x40/csd;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x40,
            ),
        "/csd =-U";
    is $c, 0x140, "/csd =-U count";



    # length(search) < length(replacement),
    # with some >= 0x100 utf8 chars in the string to be modified

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x3f/c;
    is $s, join('', map chr,
                    0x80..0xbf,
                    0x40..0xbf,
                    0x00..0x2f,
                    0xf0..0xff,
                    0x30..0x3f,
                    ((0x3f)x 16),
                ),
        "/c   <U";
    is $c, 0x90, "/c   <U count";

    $s = $all255_plus;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x3f/cd;
    is $s, join('', map chr,
                0x80..0xbf,
                0x40..0xbf,
                0x00..0x2f,
                0xf0..0xff,
                0x30..0x3f,
                ),
            "/cd  <U";
    is $c, 0x90, "/cd  <U count";

    $s = $all255_twice_plus . "\x3f\x3f\x{200}\x{300}";
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x3f/cs;
    is $s, join('', map chr,
                0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
                (map  { ($_, $_) } 0xf0..0xff),
                0x30..0x3f,
                0xbf,
                0x3f,
            ),
        "/cs  <U";
    is $c, 0x124, "/cs  <U count";

    $s = $all255_twice_plus;
    $c = $s =~ tr/\x40-\xbf\xf0-\xff/\x80-\xbf\x00-\x3f/csd;
    is $s, join('', map chr, 0x80..0xbf,
                (map  { ($_, $_) } 0x40..0xbf),
                0x00..0x2f,
                (map  { ($_, $_) } 0xf0..0xff),
                0x30..0x3f,
            ),
        "/csd <U";
    is $c, 0x120, "/csd <U count";

    if ($::IS_EBCDIC) {
        skip "Not valid only for EBCDIC", 4;
    }
    $s = $all255_twice;
    $c = $s =~ tr/[](){}<>\x00-\xff/[[(({{<</sd;
    is $s, "(<[{", 'tr/[](){}<>\x00-\xff/[[(({{<</sd';
    is $c, 512, "count of above";

    $s = $all255_plus;
    $c = $s =~ tr/[](){}<>\x00-\xff/[[(({{<</sd;
    is $s, "(<[{" . $plus, 'tr/[](){}<>\x00-\xff/[[(({{<</sd';
    is $c, 256, "count of above";
}

{
    # RT #132608
    # the 'extra length' for tr///c was stored as a short, so if the
    # replacement string had more than 0x7fff chars not paired with
    # search chars, bad things could happen

    my ($c, $e, $s);

    $s = "\x{9000}\x{9001}\x{9002}";
    $e =    "\$c = \$s =~ tr/\\x00-\\xff/"
          . ("ABCDEFGHIJKLMNO" x (0xa000 / 15))
          . "/c; 1; ";
    eval $e or die $@;
    is $s, "IJK", "RT #132608 len=0xa000";
    is $c, 3, "RT #132608 len=0xa000 count";

    $s = "\x{9003}\x{9004}\x{9005}";
    $e =    "\$c = \$s =~ tr/\\x00-\\xff/"
          . ("ABCDEFGHIJKLMNO" x (0x12000 / 15))
          . "/c; 1; ";
    eval $e or die $@;
    is $s, "LMN", "RT #132608 len=0x12000";
    is $c, 3, "RT #132608 len=0x12000 count";
}


SKIP: {   # Test literal range end point special handling
    unless ($::IS_EBCDIC) {
        skip "Valid only for EBCDIC", 24;
    }

    $_ = "\x89";    # is 'i'
    tr/i-j//d;
    is($_, "", '"\x89" should match [i-j]');
    $_ = "\x8A";
    tr/i-j//d;
    is($_, "\x8A", '"\x8A" shouldnt match [i-j]');
    $_ = "\x90";
    tr/i-j//d;
    is($_, "\x90", '"\x90" shouldnt match [i-j]');
    $_ = "\x91";    # is 'j'
    tr/i-j//d;
    is($_, "", '"\x91" should match [i-j]');

    $_ = "\x89";
    tr/i-\N{LATIN SMALL LETTER J}//d;
    is($_, "", '"\x89" should match [i-\N{LATIN SMALL LETTER J}]');
    $_ = "\x8A";
    tr/i-\N{LATIN SMALL LETTER J}//d;
    is($_, "\x8A", '"\x8A" shouldnt match [i-\N{LATIN SMALL LETTER J}]');
    $_ = "\x90";
    tr/i-\N{LATIN SMALL LETTER J}//d;
    is($_, "\x90", '"\x90" shouldnt match [i-\N{LATIN SMALL LETTER J}]');
    $_ = "\x91";
    tr/i-\N{LATIN SMALL LETTER J}//d;
    is($_, "", '"\x91" should match [i-\N{LATIN SMALL LETTER J}]');

    $_ = "\x89";
    tr/i-\N{U+6A}//d;
    is($_, "", '"\x89" should match [i-\N{U+6A}]');
    $_ = "\x8A";
    tr/i-\N{U+6A}//d;
    is($_, "\x8A", '"\x8A" shouldnt match [i-\N{U+6A}]');
    $_ = "\x90";
    tr/i-\N{U+6A}//d;
    is($_, "\x90", '"\x90" shouldnt match [i-\N{U+6A}]');
    $_ = "\x91";
    tr/i-\N{U+6A}//d;
    is($_, "", '"\x91" should match [i-\N{U+6A}]');

    $_ = "\x89";
    tr/\N{U+69}-\N{U+6A}//d;
    is($_, "", '"\x89" should match [\N{U+69}-\N{U+6A}]');
    $_ = "\x8A";
    tr/\N{U+69}-\N{U+6A}//d;
    is($_, "\x8A", '"\x8A" shouldnt match [\N{U+69}-\N{U+6A}]');
    $_ = "\x90";
    tr/\N{U+69}-\N{U+6A}//d;
    is($_, "\x90", '"\x90" shouldnt match [\N{U+69}-\N{U+6A}]');
    $_ = "\x91";
    tr/\N{U+69}-\N{U+6A}//d;
    is($_, "", '"\x91" should match [\N{U+69}-\N{U+6A}]');

    $_ = "\x89";
    tr/i-\x{91}//d;
    is($_, "", '"\x89" should match [i-\x{91}]');
    $_ = "\x8A";
    tr/i-\x{91}//d;
    is($_, "", '"\x8A" should match [i-\x{91}]');
    $_ = "\x90";
    tr/i-\x{91}//d;
    is($_, "", '"\x90" should match [i-\x{91}]');
    $_ = "\x91";
    tr/i-\x{91}//d;
    is($_, "", '"\x91" should match [i-\x{91}]');

    # Need to use eval, because tries to compile on ASCII platforms even
    # though the tests are skipped, and fails because 0x89-j is an illegal
    # range there.
    $_ = "\x89";
    eval 'tr/\x{89}-j//d';
    is($_, "", '"\x89" should match [\x{89}-j]');
    $_ = "\x8A";
    eval 'tr/\x{89}-j//d';
    is($_, "", '"\x8A" should match [\x{89}-j]');
    $_ = "\x90";
    eval 'tr/\x{89}-j//d';
    is($_, "", '"\x90" should match [\x{89}-j]');
    $_ = "\x91";
    eval 'tr/\x{89}-j//d';
    is($_, "", '"\x91" should match [\x{89}-j]');
}


# In EBCDIC 'I' is \xc9 and 'J' is \0xd1, 'i' is \x89 and 'j' is \x91.
# Yes, discontinuities.  Regardless, the \xca in the below should stay
# untouched (and not became \x8a).
{
    $_ = "I\xcaJ";

    tr/I-J/i-j/;

    is($_, "i\xcaj",    'EBCDIC discontinuity');
}
#

($x = 12) =~ tr/1/3/;
(my $y = 12) =~ tr/1/3/;
($f = 1.5) =~ tr/1/3/;
(my $g = 1.5) =~ tr/1/3/;
is($x + $y + $f + $g, 71,   'tr cancels IOK and NOK');

# /r
$_ = 'adam';
is y/dam/ve/rd, 'eve', '/r';
is $_, 'adam', '/r leaves param alone';
$g = 'ruby';
is $g =~ y/bury/repl/r, 'perl', '/r with explicit param';
is $g, 'ruby', '/r leaves explicit param alone';
is "aaa" =~ y\a\b\r, 'bbb', '/r with constant param';
ok !eval '$_ !~ y///r', "!~ y///r is forbidden";
like $@, qr\^Using !~ with tr///r doesn't make sense\,
  "!~ y///r error message";
{
  my $w;
  my $wc;
  local $SIG{__WARN__} = sub { $w = shift; ++$wc };
  local $^W = 1;
  eval 'y///r; 1';
  like $w, qr '^Useless use of non-destructive transliteration \(tr///r\)',
    '/r warns in void context';
  is $wc, 1, '/r warns just once';
}

# perlbug [ID 20000511.005 (#3237)]
$_ = 'fred';
/([a-z]{2})/;
$1 =~ tr/A-Z//;
s/^(\s*)f/$1F/;
is($_, 'Fred',  'harmless if explicitly not updating');


# A variant of the above, added in 5.7.2
$_ = 'fred';
/([a-z]{2})/;
eval '$1 =~ tr/A-Z/A-Z/;';
s/^(\s*)f/$1F/;
is($_, 'Fred',  'harmless if implicitly not updating');
is($@, '',      '    no error');


# check tr handles UTF8 correctly
($x = 256.65.258) =~ tr/a/b/;
is($x, 256.65.258,  'handles UTF8');
is(length $x, 3);

$x =~ tr/A/B/;
is(length $x, 3);
if ($::IS_ASCII) { # ASCII
    is($x, 256.66.258);
}
else {
    is($x, 256.65.258);
}

# EBCDIC variants of the above tests
($x = 256.193.258) =~ tr/a/b/;
is(length $x, 3);
is($x, 256.193.258);

$x =~ tr/A/B/;
is(length $x, 3);
if ($::IS_ASCII) { # ASCII
    is($x, 256.193.258);
}
else {
    is($x, 256.194.258);
}


start:
{
    my $l = chr(300); my $r = chr(400);
    $x = 200.300.400;
    $x =~ tr/\x{12c}/\x{190}/;
    is($x, 200.400.400,     
                        'changing UTF8 chars in a UTF8 string, same length');
    is(length $x, 3);

    $x = 200.300.400;
    $x =~ tr/\x{12c}/\x{be8}/;
    is($x, 200.3048.400,    '    more bytes');
    is(length $x, 3);

    $x = 100.125.60;
    $x =~ tr/\x{64}/\x{190}/;
    is($x, 400.125.60,      'Putting UT8 chars into a non-UTF8 string');
    is(length $x, 3);

    $x = 400.125.60;
    $x =~ tr/\x{190}/\x{64}/;
    is($x, 100.125.60,      'Removing UTF8 chars from UTF8 string');
    is(length $x, 3);

    $x = 400.125.60.400;
    $y = $x =~ tr/\x{190}/\x{190}/;
    is($y, 2,               'Counting UTF8 chars in UTF8 string');

    $x = 60.400.125.60.400;
    $y = $x =~ tr/\x{3c}/\x{3c}/;
    is($y, 2,               '         non-UTF8 chars in UTF8 string');

    # 17 - counting UTF8 chars in non-UTF8 string
    $x = 200.125.60;
    $y = $x =~ tr/\x{190}/\x{190}/;
    is($y, 0,               '         UTF8 chars in non-UTFs string');
}

$_ = "abcdefghijklmnopqrstuvwxyz";
eval 'tr/a-z-9/ /';
like($@, qr/^Ambiguous range in transliteration operator/,  'tr/a-z-9//');

# 19-21: Make sure leading and trailing hyphens still work
$_ = "car-rot9";
tr/-a-m/./;
is($_, '..r.rot9',  'hyphens, leading');

$_ = "car-rot9";
tr/a-m-/./;
is($_, '..r.rot9',  '   trailing');

$_ = "car-rot9";
tr/-a-m-/./;
is($_, '..r.rot9',  '   both');

$_ = "abcdefghijklmnop";
tr/ae-hn/./;
is($_, '.bcd....ijklm.op');

$_ = "abcdefghijklmnop";
tr/a-cf-kn-p/./;
is($_, '...de......lm...');

$_ = "abcdefghijklmnop";
tr/a-ceg-ikm-o/./;
is($_, '...d.f...j.l...p');


# 20000705 MJD
eval "tr/m-d/ /";
like($@, qr/^Invalid range "m-d" in transliteration operator/,
              'reversed range check');

'abcdef' =~ /(bcd)/;
is(eval '$1 =~ tr/abcd//', 3,  'explicit read-only count');
is($@, '',                      '    no error');

'abcdef' =~ /(bcd)/;
is(eval '$1 =~ tr/abcd/abcd/', 3,  'implicit read-only count');
is($@, '',                      '    no error');

is(eval '"123" =~ tr/12//', 2,     'LHS of non-updating tr');

eval '"123" =~ tr/1/2/';
like($@, qr|^Can't modify constant item in transliteration \(tr///\)|,
         'LHS bad on updating tr');


# v300 (0x12c) is UTF-8-encoded as 196 172 (0xc4 0xac)
# v400 (0x190) is UTF-8-encoded as 198 144 (0xc6 0x90)

# Transliterate a byte to a byte, all four ways.

($a = v300.196.172.300.196.172) =~ tr/\xc4/\xc5/;
is($a, v300.197.172.300.197.172,    'byte2byte transliteration');

($a = v300.196.172.300.196.172) =~ tr/\xc4/\x{c5}/;
is($a, v300.197.172.300.197.172);

($a = v300.196.172.300.196.172) =~ tr/\x{c4}/\xc5/;
is($a, v300.197.172.300.197.172);

($a = v300.196.172.300.196.172) =~ tr/\x{c4}/\x{c5}/;
is($a, v300.197.172.300.197.172);


($a = v300.196.172.300.196.172) =~ tr/\xc4/\x{12d}/;
is($a, v300.301.172.300.301.172,    'byte2wide transliteration');

($a = v300.196.172.300.196.172) =~ tr/\x{12c}/\xc3/;
is($a, v195.196.172.195.196.172,    '   wide2byte');

($a = v300.196.172.300.196.172) =~ tr/\x{12c}/\x{12d}/;
is($a, v301.196.172.301.196.172,    '   wide2wide');


($a = v300.196.172.300.196.172) =~ tr/\xc4\x{12c}/\x{12d}\xc3/;
is($a, v195.301.172.195.301.172,    'byte2wide & wide2byte');


($a = v300.196.172.300.196.172.400.198.144) =~
	tr/\xac\xc4\x{12c}\x{190}/\xad\x{12d}\xc5\x{191}/;
is($a, v197.301.173.197.301.173.401.198.144,    'all together now!');


is((($a = v300.196.172.300.196.172) =~ tr/\xc4/\xc5/), 2,
                                     'transliterate and count');

is((($a = v300.196.172.300.196.172) =~ tr/\x{12c}/\x{12d}/), 2);


($a = v300.196.172.300.196.172) =~ tr/\xc4/\x{12d}/c;
is($a, v301.196.301.301.196.301,    'translit w/complement');

($a = v300.196.172.300.196.172) =~ tr/\x{12c}/\xc5/c;
is($a, v300.197.197.300.197.197, 'more translit w/complement');


($a = v300.196.172.300.196.172) =~ tr/\xc4//d;
is($a, v300.172.300.172,            'translit w/deletion');

($a = v300.196.172.300.196.172) =~ tr/\x{12c}//d;
is($a, v196.172.196.172);


($a = v196.196.172.300.300.196.172) =~ tr/\xc4/\xc5/s;
is($a, v197.172.300.300.197.172,    'translit w/squeeze');

($a = v196.172.300.300.196.172.172) =~ tr/\x{12c}/\x{12d}/s;
is($a, v196.172.301.196.172.172);


# Tricky cases (When Simon Cozens Attacks)
($a = v196.172.200) =~ tr/\x{12c}/a/;
is(sprintf("%vd", $a), '196.172.200');

($a = v196.172.200) =~ tr/\x{12c}/\x{12c}/;
is(sprintf("%vd", $a), '196.172.200');

($a = v196.172.200) =~ tr/\x{12c}//d;
is(sprintf("%vd", $a), '196.172.200');


# UTF8 range tests from Inaba Hiroto

($a = v300.196.172.302.197.172) =~ tr/\x{12c}-\x{130}/\xc0-\xc4/;
is($a, v192.196.172.194.197.172,    'UTF range');

($a = v300.196.172.302.197.172) =~ tr/\xc4-\xc8/\x{12c}-\x{130}/;
is($a, v300.300.172.302.301.172);


# UTF8 range tests from Karsten Sperling (patch #9008 required)

($a = "\x{0100}") =~ tr/\x00-\x{100}/X/;
is($a, "X");

($a = "\x{0100}") =~ tr/\x{0000}-\x{00ff}/X/c;
is($a, "X");

($a = "\x{0100}") =~ tr/\x{0000}-\x{00ff}\x{0101}/X/c;
is($a, "X");
 
($a = v256) =~ tr/\x{0000}-\x{00ff}\x{0101}/X/c;
is($a, "X");


# UTF8 range tests from Inaba Hiroto

($a = "\x{200}") =~ tr/\x00-\x{100}/X/c;
is($a, "X");

($a = "\x{200}") =~ tr/\x00-\x{100}/X/cs;
is($a, "X");

# Tricky on EBCDIC: while [a-z] [A-Z] must not match the gap characters (as
# well as i-j, r-s, I-J, R-S), [\x89-\x91] [\xc9-\xd1] has to match them,
# from Karsten Sperling.

$c = ($a = "\x89\x8a\x8b\x8c\x8d\x8f\x90\x91") =~ tr/\x89-\x91/X/;
is($c, 8);
is($a, "XXXXXXXX");

$c = ($a = "\xc9\xca\xcb\xcc\xcd\xcf\xd0\xd1") =~ tr/\xc9-\xd1/X/;
is($c, 8);
is($a, "XXXXXXXX");

SKIP: {
    skip "EBCDIC-centric tests", 4 unless $::IS_EBCDIC;

    $c = ($a = "\x89\x8a\x8b\x8c\x8d\x8f\x90\x91") =~ tr/i-j/X/;
    is($c, 2);
    is($a, "X\x8a\x8b\x8c\x8d\x8f\x90X");
   
    $c = ($a = "\xc9\xca\xcb\xcc\xcd\xcf\xd0\xd1") =~ tr/I-J/X/;
    is($c, 2);
    is($a, "X\xca\xcb\xcc\xcd\xcf\xd0X");
}

($a = "\x{100}") =~ tr/\x00-\xff/X/c;
is(ord($a), ord("X"));

($a = "\x{100}") =~ tr/\x00-\xff/X/cs;
is(ord($a), ord("X"));

($a = "\x{100}\x{100}") =~ tr/\x{101}-\x{200}//c;
is($a, "\x{100}\x{100}");

($a = "\x{100}\x{100}") =~ tr/\x{101}-\x{200}//cs;
is($a, "\x{100}");

$a = "\xfe\xff"; $a =~ tr/\xfe\xff/\x{1ff}\x{1fe}/;
is($a, "\x{1ff}\x{1fe}");


# From David Dyck
($a = "R0_001") =~ tr/R_//d;
is(hex($a), 1);

# From Inaba Hiroto
@a = (1,2); map { y/1/./ for $_ } @a;
is("@a", ". 2");

@a = (1,2); map { y/1/./ for $_.'' } @a;
is("@a", "1 2");


# Additional test for Inaba Hiroto patch (robin@kitsite.com)
($a = "\x{100}\x{102}\x{101}") =~ tr/\x00-\377/XYZ/c;
is($a, "XZY");


# Used to fail with "Modification of a read-only value attempted"
%a = (N=>1);
foreach (keys %a) {
  eval 'tr/N/n/';
  is($_, 'n',   'pp_trans needs to unshare shared hash keys');
  is($@, '',    '   no error');
}


$x = eval '"1213" =~ tr/1/1/';
is($x, 2,   'implicit count on constant');
is($@, '',  '   no error');


my @foo = ();
eval '$foo[-1] =~ tr/N/N/';
is( $@, '',         'implicit count outside array bounds, index negative' );
is( scalar @foo, 0, "    doesn't extend the array");

eval '$foo[1] =~ tr/N/N/';
is( $@, '',         'implicit count outside array bounds, index positive' );
is( scalar @foo, 0, "    doesn't extend the array");


my %foo = ();
eval '$foo{bar} =~ tr/N/N/';
is( $@, '',         'implicit count outside hash bounds' );
is( scalar keys %foo, 0,   "    doesn't extend the hash");

$x = \"foo";
is( $x =~ tr/A/A/, 2, 'non-modifying tr/// on a scalar ref' );
is( ref $x, 'SCALAR', "    doesn't stringify its argument" );

# rt.perl.org 36622.  Perl didn't like a y/// at end of file.  No trailing
# newline allowed.
fresh_perl_is(q[$_ = "foo"; y/A-Z/a-z/], '', {}, 'RT #36622 y/// at end of file');


{ # [perl #38293] chr(65535) should be allowed in regexes
no warnings 'utf8'; # to allow non-characters

$s = "\x{d800}\x{ffff}";
$s =~ tr/\0/A/;
is($s, "\x{d800}\x{ffff}", "do_trans_simple");

$s = "\x{d800}\x{ffff}";
$i = $s =~ tr/\0//;
is($i, 0, "do_trans_count");

$s = "\x{d800}\x{ffff}";
$s =~ tr/\0/A/s;
is($s, "\x{d800}\x{ffff}", "do_trans_complex, SQUASH");

$s = "\x{d800}\x{ffff}";
$s =~ tr/\0/A/c;
is($s, "AA", "do_trans_complex, COMPLEMENT");

$s = "A\x{ffff}B";
$s =~ tr/\x{ffff}/\x{1ffff}/;
is($s, "A\x{1ffff}B", "utf8, SEARCHLIST");

$s = "\x{fffd}\x{fffe}\x{ffff}";
$s =~ tr/\x{fffd}-\x{ffff}/ABC/;
is($s, "ABC", "utf8, SEARCHLIST range");

$s = "ABC";
$s =~ tr/ABC/\x{ffff}/;
is($s, "\x{ffff}"x3, "utf8, REPLACEMENTLIST");

$s = "ABC";
$s =~ tr/ABC/\x{fffd}-\x{ffff}/;
is($s, "\x{fffd}\x{fffe}\x{ffff}", "utf8, REPLACEMENTLIST range");

$s = "A\x{ffff}B\x{100}\0\x{fffe}\x{ffff}"; $i = $s =~ tr/\x{ffff}//;
is($i, 2, "utf8, count");

$s = "A\x{ffff}\x{ffff}C";
$s =~ tr/\x{ffff}/\x{100}/s;
is($s, "A\x{100}C", "utf8, SQUASH");

$s = "A\x{ffff}\x{ffff}\x{fffe}\x{fffe}\x{fffe}C";
$s =~ tr/\x{fffe}\x{ffff}//s;
is($s, "A\x{ffff}\x{fffe}C", "utf8, SQUASH");

$s = "xAABBBy";
$s =~ tr/AB/\x{ffff}/s;
is($s, "x\x{ffff}y", "utf8, SQUASH");

$s = "xAABBBy";
$s =~ tr/AB/\x{fffe}\x{ffff}/s;
is($s, "x\x{fffe}\x{ffff}y", "utf8, SQUASH");

$s = "A\x{ffff}B\x{fffe}C";
$s =~ tr/\x{fffe}\x{ffff}/x/c;
is($s, "x\x{ffff}x\x{fffe}x", "utf8, COMPLEMENT");

$s = "A\x{10000}B\x{2abcd}C";
$s =~ tr/\0-\x{ffff}/x/c;
is($s, "AxBxC", "utf8, COMPLEMENT range");

$s = "A\x{fffe}B\x{ffff}C";
$s =~ tr/\x{fffe}\x{ffff}/x/d;
is($s, "AxBC", "utf8, DELETE");

} # non-characters end

{ # related to [perl #27940]
    my $c;

    ($c = "\x20\c@\x30\cA\x40\cZ\x50\c_\x60") =~ tr/\c@-\c_//d;
    is($c, "\x20\x30\x40\x50\x60", "tr/\\c\@-\\c_//d");

    ($c = "\x20\x00\x30\x01\x40\x1A\x50\x1F\x60") =~ tr/\x00-\x1f//d;
    is($c, "\x20\x30\x40\x50\x60", "tr/\\x00-\\x1f//d");
}

SKIP: {
    if (!eval { require XS::APItest }) { skip "no XS::APItest", 2 }
    skip "with NODEFAULT_SHAREKEYS there are few COWs", 2
        if $Config::Config{ccflags} =~ /-DNODEFAULT_SHAREKEYS\b/;

    ($s) = keys %{{pie => 3}};
    my $wasro = XS::APItest::SvIsCOW($s);
    ok $wasro, "have a COW";
    $s =~ tr/i//;
    ok( XS::APItest::SvIsCOW($s),
       "count-only tr doesn't deCOW COWs" );
}

# [ RT #61520 ]
#
# under threads, unicode tr within a cloned closure would SEGV or assert
# fail, since the pointer in the pad to the swash was getting zeroed out
# in the proto-CV

{
    my $x = "\x{142}";
    sub {
	$x =~ tr[\x{142}][\x{143}];
    }->();
    is($x,"\x{143}", "utf8 + closure");
}

# Freeing of trans ops prior to pmtrans() [perl #102858].
eval q{ $a ~= tr/a/b/; };
ok 1;
SKIP: {
    no warnings "deprecated";
    skip "no encoding", 1 unless eval { require encoding; 1 };
    eval q{ use encoding "utf8"; $a ~= tr/a/b/; };
    ok 1;
}

{ # [perl #113584]

    my $x = "Perlα";
    $x =~ tr/αα/βγ/;
    { no warnings 'utf8'; print "# $x\n"; } # No note() to avoid wide warning.
    is($x, "Perlβ", "Only first of multiple transliterations is used");
}

# tr/a/b/ should fail even on zero-length read-only strings
use constant nullrocow => (keys%{{""=>undef}})[0];
for ("", nullrocow) {
    eval { $_ =~ y/a/b/ };
    like $@, qr/^Modification of a read-only value attempted at /,
        'tr/a/b/ fails on zero-length ro string';
}

# Whether they're permitted or not, non-modifying tr/// should not write
# to read-only values, even with funky flags.
{ # [perl #123759]
	eval q{ ('a' =~ /./) =~ tr///d };
	ok(1, "tr///d on PL_Yes does not assert");
	eval q{ ('a' =~ /./) =~ tr/a-z/a-z/d };
	ok(1, "tr/a-z/a-z/d on PL_Yes does not assert");
	eval q{ ('a' =~ /./) =~ tr///s };
	ok(1, "tr///s on PL_Yes does not assert");
	eval q{ *x =~ tr///d };
	ok(1, "tr///d on glob does not assert");
}

{ # [perl #128734
    my $string = chr utf8::unicode_to_native(0x00e0);
    $string =~ tr/\N{U+00e0}/A/;
    is($string, "A", 'tr// of \N{U+...} works for upper-Latin1');
    $string = chr utf8::unicode_to_native(0x00e1);
    $string =~ tr/\N{LATIN SMALL LETTER A WITH ACUTE}/A/;
    is($string, "A", 'tr// of \N{name} works for upper-Latin1');
}

# RT #130198
# a tr/// that is cho(m)ped, possibly with an array as arg

{
    use warnings;

    my ($s, @a);

    my $warn;
    local $SIG{__WARN__ } = sub { $warn .= "@_" };

    for my $c (qw(chop chomp)) {
        for my $bind ('', '$s =~ ', '@a =~ ') {
            for my $arg2 (qw(a b)) {
                for my $r ('', 'r') {
                    $warn = '';
                    # tr/a/b/ modifies its LHS, so if the LHS is an
                    # array, this should die. The special cases of tr/a/a/
                    # and tr/a/b/r don't modify their LHS, so instead
                    # we croak because cho(m)p is trying to modify it.
                    #
                    my $exp =
                        ($r eq '' && $arg2 eq 'b' && $bind =~ /\@a/)
                            ? qr/Can't modify private array in transliteration/
                            : qr{Can't modify transliteration \(tr///\) in $c};

                    my $expr = "$c(${bind}tr/a/$arg2/$r);";
                    eval $expr;
                    like $@, $exp, "RT #130198 eval: $expr";

                    $exp =
                        $bind =~ /\@a/
                         ? qr{^Applying transliteration \(tr///\) to \@a will act on scalar\(\@a\)}
                         : qr/^$/;
                    like $warn, $exp, "RT #130198 warn: $expr";
                }
            }
        }
    }


}

{   # [perl #130656] This bug happens when the tr is split across lines, so
    # that the first line causes it to go into UTF-8, and the 2nd is only
    # things like \x
    my $x = "\x{E235}";
    $x =~ tr
    [\x{E234}-\x{E342}\x{E5B5}-\x{E5DF}]
    [\x{E5CD}-\x{E5DF}\x{EA80}-\x{EAFA}\x{EB0E}-\x{EB8E}\x{EAFB}-\x{EB0D}\x{E5B5}-\x{E5CC}];

    is $x, "\x{E5CE}", '[perl #130656]';

}

{
    fresh_perl_like('y/\x{a00}0-\N{}//', qr/Unknown charname/, { },
                    'RT #133880 illegal \N{}');
}

{
    my $c;
    my $x = "\1\0\0\0\0\0\0\0\0\0\0\0\0";
    $c = $x =~ tr/\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00/FEDCBA9876543210/;
    is $x, "1000000000000", "Decreasing ranges work with start at \\0";
    is $c, 13, "Count for above test";

    $x = "\1\0\0\0\0\0\0\0\0\0\0\0\0";
    $c = $x =~ tr/\x0f\x0e\x0d\x0c\x0b\x0a\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00/\x{FF26}\x{FF25}\x{FF24}\x{FF23}\x{FF22}\x{FF21}\x{FF19}\x{FF18}\x{FF17}\x{FF16}\x{FF15}\x{FF14}\x{FF13}\x{FF12}\x{FF11}\x{FF10}/;
    is $x, "\x{FF11}\x{FF10}\x{FF10}\x{FF10}\x{FF10}\x{FF10}\x{FF10}\x{FF10}\x{FF10}\x{FF10}\x{FF10}\x{FF10}\x{FF10}", "Decreasing Above ASCII ranges work with start at \\0";
    is $c, 13, "Count for above test";
}

{
    my $c = "\xff";
    my $d = "\x{104}";
    eval '$c =~ tr/\x{ff}-\x{104}/\x{100}-\x{105}/';
    is($@, "", 'tr/\x{ff}-\x{104}/\x{100}-\x{105}/ compiled');
    is($c, "\x{100}", 'ff -> 100');
    eval '$d =~ tr/\x{ff}-\x{104}/\x{100}-\x{105}/';
    is($d, "\x{105}", '104 -> 105');
}

{
    my $c = "cb";
    eval '$c =~ tr{aabc}{d\x{d0000}}';
    is($c, "\x{d0000}\x{d0000}", "Shouldn't generate valgrind errors");
}

1;
