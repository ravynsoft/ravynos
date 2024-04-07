
BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir('t') if -d 't';
        @INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

BEGIN {
    unless (5.006001 <= $]) {
	print "1..0 # skipped: Perl 5.6.1 or later".
		" needed for this test\n";
	exit;
    }
}

#########################

use strict;
use warnings;
BEGIN { $| = 1; print "1..34\n"; }
my $count = 0;
sub ok { Unicode::Normalize::ok(\$count, @_) }

use Unicode::Normalize qw(:all);

ok(1);

sub _pack_U   { Unicode::Normalize::dot_t_pack_U(@_) }
sub _unpack_U { Unicode::Normalize::dot_t_unpack_U(@_) }

#########################

our $proc;    # before the last starter
our $unproc;  # the last starter and after
# If string has no starter, entire string is set to $unproc.

($proc, $unproc) = splitOnLastStarter("");
ok($proc,   "");
ok($unproc, "");

($proc, $unproc) = splitOnLastStarter("A");
ok($proc,   "");
ok($unproc, "A");

($proc, $unproc) = splitOnLastStarter(_pack_U(0x41, 0x300, 0x327, 0x42));
ok($proc,   _pack_U(0x41, 0x300, 0x327));
ok($unproc, "B");

($proc, $unproc) = splitOnLastStarter(_pack_U(0x4E00, 0x41, 0x301));
ok($proc,   _pack_U(0x4E00));
ok($unproc, _pack_U(0x41, 0x301));

($proc, $unproc) = splitOnLastStarter(_pack_U(0x302, 0x301, 0x300));
ok($proc,   "");
ok($unproc, _pack_U(0x302, 0x301, 0x300));

our $ka_grave = _pack_U(0x41, 0, 0x42, 0x304B, 0x300);
our $dakuten  = _pack_U(0x3099);
our $ga_grave = _pack_U(0x41, 0, 0x42, 0x304C, 0x300);

our ($p, $u) = splitOnLastStarter($ka_grave);
our $concat = $p . NFC($u.$dakuten);

ok(NFC($ka_grave.$dakuten) eq $ga_grave);
ok(NFC($ka_grave).NFC($dakuten) ne $ga_grave);
ok($concat eq $ga_grave);

# 14

sub arraynorm {
    my $form   = shift;
    my @string = @_;
    my $result = "";
    my $unproc = "";
    foreach my $str (@string) {
        $unproc .= $str;
        my $n = normalize($form, $unproc);
        my($p, $u) = splitOnLastStarter($n);
        $result .= $p;
        $unproc  = $u;
    }
    $result .= $unproc;
    return $result;
}

my $strD = "\x{3C9}\x{301}\x{1100}\x{1161}\x{11A8}\x{1100}\x{1161}\x{11AA}";
my $strC = "\x{3CE}\x{AC01}\x{AC03}";
my @str1 = (substr($strD,0,3), substr($strD,3,4), substr($strD,7));
my @str2 = (substr($strD,0,1), substr($strD,1,3), substr($strD,4));
ok($strC eq NFC($strD));
ok($strD eq join('', @str1));
ok($strC eq arraynorm('NFC', @str1));
ok($strD eq join('', @str2));
ok($strC eq arraynorm('NFC', @str2));

my @strX = ("\x{300}\x{AC00}", "\x{11A8}");
my $strX =  "\x{300}\x{AC01}";
ok($strX eq NFC(join('', @strX)));
ok($strX eq arraynorm('NFC', @strX));
ok($strX eq NFKC(join('', @strX)));
ok($strX eq arraynorm('NFKC', @strX));

my @strY = ("\x{304B}\x{0308}", "\x{0323}\x{3099}");
my $strY = ("\x{304C}\x{0323}\x{0308}");
ok($strY eq NFC(join('', @strY)));
ok($strY eq arraynorm('NFC', @strY));
ok($strY eq NFKC(join('', @strY)));
ok($strY eq arraynorm('NFKC', @strY));

my @strZ = ("\x{304B}\x{0308}", "\x{0323}", "\x{3099}");
my $strZ = ("\x{304B}\x{3099}\x{0323}\x{0308}");
ok($strZ eq NFD(join('', @strZ)));
ok($strZ eq arraynorm('NFD', @strZ));
ok($strZ eq NFKD(join('', @strZ)));
ok($strZ eq arraynorm('NFKD', @strZ));

# 31

# don't modify the source

my $source = "ABC";
($proc, $unproc) = splitOnLastStarter($source);
ok($proc,   "AB");
ok($unproc, "C");
ok($source, "ABC");

# 34

