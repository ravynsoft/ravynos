BEGIN {
    if ($ENV{'PERL_CORE'}){
        chdir 't';
        unshift @INC, '../lib';
    }
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # Skip: EBCDIC\n";
    exit 0;
    }
    $| = 1;
}

use strict;
#use Test::More qw(no_plan);
use Test::More tests => 58;
use Encode q(:all);

my $uo = '';
my $nf  = '';
my ($af, $aq, $ap, $ah, $ax, $uf, $uq, $up, $uh, $ux, $ac, $uc);
for my $i (0x20..0x7e){
    $uo .= chr($i);
}
$af = $aq = $ap = $ah = $ax = $ac =
$uf = $uq = $up = $uh = $ux = $uc =
$nf = $uo;

my $residue = '';
for my $i (0x80..0xff){
    $uo   .= chr($i);
    $residue    .= chr($i);
    $af .= '?';
    $uf .= "\x{FFFD}";
    $ap .= sprintf("\\x{%04x}", $i);
    $up .= sprintf("\\x%02X", $i);
    $ah .= sprintf("&#%d;", $i);
    $uh .= sprintf("\\x%02X", $i);
    $ax .= sprintf("&#x%x;", $i);
    $ux .= sprintf("\\x%02X", $i);
    $ac .= sprintf("<U+%04X>", $i);
    $uc .= sprintf("[%02X]", $i);
}

my $ao = $uo;
utf8::upgrade($uo);

my $ascii  = find_encoding('ascii');
my $latin1 = find_encoding('latin1');
my $utf8   = find_encoding('utf8');

my $src = $uo;
my $dst = $ascii->encode($src, FB_DEFAULT);
is($dst, $af, "FB_DEFAULT ascii");
is($src, $uo, "FB_DEFAULT residue ascii");

$src = $ao;
$dst = $utf8->decode($src, FB_DEFAULT);
is($dst, $uf, "FB_DEFAULT utf8");
is($src, $ao, "FB_DEFAULT residue utf8");

$src = $uo;
eval{ $dst = $ascii->encode($src, FB_CROAK) };
like($@, qr/does not map to ascii/o, "FB_CROAK ascii");
is($src, $uo, "FB_CROAK residue ascii");

$src = $ao;
eval{ $dst = $utf8->decode($src, FB_CROAK) };
like($@, qr/does not map to Unicode/o, "FB_CROAK utf8");
is($src, $ao, "FB_CROAK residue utf8");

$src = $nf;
eval{ $dst = $ascii->encode($src, FB_CROAK) };
is($@, '', "FB_CROAK on success ascii");
is($src, '', "FB_CROAK on success residue ascii");

$src = $nf;
eval{ $dst = $utf8->decode($src, FB_CROAK) };
is($@, '', "FB_CROAK on success utf8");
is($src, '', "FB_CROAK on success residue utf8");

$src = $uo;
$dst = $ascii->encode($src, FB_QUIET);
is($dst, $aq,   "FB_QUIET ascii");
is($src, $residue, "FB_QUIET residue ascii");

$src = $ao;
$dst = $utf8->decode($src, FB_QUIET);
is($dst, $uq,   "FB_QUIET utf8");
is($src, $residue, "FB_QUIET residue utf8");

{
    my $message = '';
    local $SIG{__WARN__} = sub { $message = $_[0] };

    $src = $uo;
    $dst = $ascii->encode($src, FB_WARN);
    is($dst, $aq,   "FB_WARN ascii");
    is($src, $residue, "FB_WARN residue ascii");
    like($message, qr/does not map to ascii/o, "FB_WARN message ascii");

    $message = '';
    $src = $ao;
    $dst = $utf8->decode($src, FB_WARN);
    is($dst, $uq,   "FB_WARN utf8");
    is($src, $residue, "FB_WARN residue utf8");
    like($message, qr/does not map to Unicode/o, "FB_WARN message utf8");

    $message = '';
    $src = $uo;
    $dst = $ascii->encode($src, WARN_ON_ERR);
    is($dst, $af, "WARN_ON_ERR ascii");
    is($src, '',  "WARN_ON_ERR residue ascii");
    like($message, qr/does not map to ascii/o, "WARN_ON_ERR message ascii");

    $message = '';
    $src = $ao;
    $dst = $utf8->decode($src, WARN_ON_ERR);
    is($dst, $uf, "WARN_ON_ERR utf8");
    is($src, '',  "WARN_ON_ERR residue utf8");
    like($message, qr/does not map to Unicode/o, "WARN_ON_ERR message ascii");
}

$src = $uo;
$dst = $ascii->encode($src, FB_PERLQQ);
is($dst, $ap, "FB_PERLQQ encode");
is($src, $uo, "FB_PERLQQ residue encode");

$src = $ao;
$dst = $ascii->decode($src, FB_PERLQQ);
is($dst, $up, "FB_PERLQQ decode");
is($src, $ao, "FB_PERLQQ residue decode");

$src = $uo;
$dst = $ascii->encode($src, FB_HTMLCREF);
is($dst, $ah, "FB_HTMLCREF encode");
is($src, $uo, "FB_HTMLCREF residue encode");

$src = $ao;
$dst = $ascii->decode($src, FB_HTMLCREF);
is($dst, $uh, "FB_HTMLCREF decode");
is($src, $ao, "FB_HTMLCREF residue decode");

$src = $uo;
$dst = $ascii->encode($src, FB_XMLCREF);
is($dst, $ax, "FB_XMLCREF encode");
is($src, $uo, "FB_XMLCREF residue encode");

$src = $ao;
$dst = $ascii->decode($src, FB_XMLCREF);
is($dst, $ux, "FB_XMLCREF decode");
is($src, $ao, "FB_XMLCREF residue decode");

$src = $uo;
$dst = $ascii->encode($src, sub{ sprintf "<U+%04X>", shift });
is($dst, $ac, "coderef encode");
is($src, $uo, "coderef residue encode");

$src = $ao;
$dst = $ascii->decode($src, sub{ sprintf "[%02X]", shift });
is($dst, $uc, "coderef decode");
is($src, $ao, "coderef residue decode");

$src = "\x{3000}";
$dst = $ascii->encode($src, sub{ $_[0] });
is $dst, 0x3000."", q{$ascii->encode($src, sub{ $_[0] } )};
$dst = encode("ascii", "\x{3000}", sub{ $_[0] });
is $dst, 0x3000."", q{encode("ascii", "\x{3000}", sub{ $_[0] })};

$src = pack "C*", 0xFF;
$dst = $ascii->decode($src, sub{ $_[0] });
is $dst, 0xFF."", q{$ascii->encode($src, sub{ $_[0] } )};
$dst = decode("ascii", (pack "C*", 0xFF), sub{ $_[0] });
is $dst, 0xFF."", q{decode("ascii", (pack "C*", 0xFF), sub{ $_[0] })};


$src = pack "C*", 0x80;
$dst = $utf8->decode($src, sub{ $_[0] });
is $dst, 0x80."", q{$utf8->encode($src, sub{ $_[0] } )};
$dst = decode("utf8", $src, sub{ $_[0] });
is $dst, 0x80."", q{decode("utf8", (pack "C*", 0x80), sub{ $_[0] })};

$src = "\x{3000}";
$dst = $latin1->encode($src, sub { "\N{U+FF}" });
is $dst, "\x{ff}", q{$latin1->encode($src, sub { "\N{U+FF}" })};
$dst = encode("latin1", $src, sub { "\N{U+FF}" });
is $dst, "\x{ff}", q{encode("latin1", $src, sub { "\N{U+FF}" })};

$src = "\x{3000}";
$dst = $latin1->encode($src, sub { utf8::upgrade(my $r = "\x{ff}"); $r });
is $dst, "\x{ff}", q{$latin1->encode($src, sub { utf8::upgrade(my $r = "\x{ff}"); $r })};
$dst = encode("latin1", $src, sub { utf8::upgrade(my $r = "\x{ff}"); $r });
is $dst, "\x{ff}", q{encode("latin1", $src, sub { utf8::upgrade(my $r = "\x{ff}"); $r })};

$src = "\x{ff}";
$dst = $utf8->decode($src, sub { chr($_[0]) });
is $dst, "\x{ff}", q{$utf8->decode($src, sub { chr($_[0]) })};
$dst = decode("utf8", $src, sub { chr($_[0]) });
is $dst, "\x{ff}", q{decode("utf8", $src, sub { chr($_[0]) })};

{
    use charnames ':full';
    $src = "\x{ff}";
    $dst = $utf8->decode($src, sub { utf8::downgrade(my $r = "\N{LATIN SMALL LETTER Y WITH DIAERESIS}"); $r });
    is $dst, "\N{LATIN SMALL LETTER Y WITH DIAERESIS}", q{$utf8->decode($src, sub { utf8::downgrade(my $r = "\N{LATIN SMALL LETTER Y WITH DIAERESIS}"); $r })};
    $dst = decode("utf8", $src, sub { utf8::downgrade(my $r = "\N{LATIN SMALL LETTER Y WITH DIAERESIS}"); $r });
    is $dst, "\N{LATIN SMALL LETTER Y WITH DIAERESIS}", q{decode("utf8", $src, sub { utf8::downgrade(my $r = "\N{LATIN SMALL LETTER Y WITH DIAERESIS}"); $r })};
}
