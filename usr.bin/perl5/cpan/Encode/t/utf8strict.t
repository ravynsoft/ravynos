#!../perl
our $DEBUG = @ARGV;
our (%ORD, %SEQ, $NTESTS);
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
     if ($] <= 5.008 and !$Config{perl_patchlevel}){
     print "1..0 # Skip: Perl 5.8.1 or later required\n";
     exit 0;
     }
     # http://smontagu.damowmow.com/utf8test.html
     # The numbers below, like 2.1.2 are test numbers on this web page
     %ORD = (
         0x00000080 => 0, # 2.1.2
         0x00000800 => 0, # 2.1.3
         0x00010000 => 0, # 2.1.4
         0x00200000 => 1, # 2.1.5
         0x00400000 => 1, # 2.1.6
         0x0000007F => 0, # 2.2.1 -- unmapped okay
         0x000007FF => 0, # 2.2.2
         0x0000FFFF => 1, # 2.2.3
         0x001FFFFF => 1, # 2.2.4
         0x03FFFFFF => 1, # 2.2.5
         0x7FFFFFFF => 1, # 2.2.6
         0x0000D800 => 1, # 5.1.1
         0x0000DB7F => 1, # 5.1.2
         0x0000D880 => 1, # 5.1.3
         0x0000DBFF => 1, # 5.1.4
         0x0000DC00 => 1, # 5.1.5
         0x0000DF80 => 1, # 5.1.6
         0x0000DFFF => 1, # 5.1.7
         # 5.2 "Paird UTF-16 surrogates skipped
         # because utf-8-strict raises exception at the first one
         0x0000FFFF => 1, # 5.3.1
        );
     $NTESTS +=  scalar keys %ORD;
     if (ord('A') == 193) {
	 %SEQ = (
		 qq/dd 64 73 73/    => 0, # 2.3.1
		 qq/dd 67 41 41/    => 0, # 2.3.2
		 qq/ee 42 73 73 71/ => 0, # 2.3.3
		 qq/f4 90 80 80/ => 1, # 2.3.4 -- out of range so NG
		 # EBCDIC TODO: "3 Malformed sequences"
		 # EBCDIC TODO: "4 Overlong sequences"
		 );
     } else {
	 %SEQ = (
		 qq/ed 9f bf/    => 0, # 2.3.1
		 qq/ee 80 80/    => 0, # 2.3.2
		 qq/f4 8f bf bd/ => 0, # 2.3.3
		 qq/f4 90 80 80/ => 1, # 2.3.4 -- out of range so NG
		 qq/80/          => 1,             # 3.1.1
		 qq/bf/          => 1,             # 3.1.2
		 qq/80 bf/       => 1,             # 3.1.3
		 qq/80 bf 80/    => 1,             # 3.1.4
		 qq/80 bf 80 bf/ => 1,             # 3.1.5
		 qq/80 bf 80 bf 80/ => 1,          # 3.1.6
		 qq/80 bf 80 bf 80 bf/ => 1,       # 3.1.7
		 qq/80 bf 80 bf 80 bf 80/ => 1,    # 3.1.8
		 qq/80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf/ => 1, # 3.1.9
		 qq/c0 20 c1 20 c2 20 c3 20 c4 20 c5 20 c6 20 c7 20 c8 20 c9 20 ca 20 cb 20 cc 20 cd 20 ce 20 cf 20 d0 20 d1 20 d2 20 d3 20 d4 20 d5 20 d6 20 d7 20 d8 20 d9 20 da 20 db 20 dc 20 dd 20 de 20 df 20/ => 1, # 3.2.1
		 qq/e0 20 e1 20 e2 20 e3 20 e4 20 e5 20 e6 20 e7 20 e8 20 e9 20 ea 20 eb 20 ec 20 ed 20 ee 20 ef 20/ => 1, # 3.2.2
		 qq/f0 20 f1 20 f2 20 f3 20 f4 20 f5 20 f6 20 f7 20/ => 1, # 3.2.3
		 qq/f8 20 f9 20 fa 20 fb 20/ => 1, # 3.2.4
		 qq/fc 20 fd 20/ => 1,             # 3.2.5
		 qq/c0/ => 1,                      # 3.3.1
		 qq/e0 80/ => 1,                   # 3.3.2
		 qq/f0 80 80/ => 1,                # 3.3.3
		 qq/f8 80 80 80/ => 1,             # 3.3.4
		 qq/fc 80 80 80 80/ => 1,          # 3.3.5
		 qq/df/ => 1,                      # 3.3.6
		 qq/ef bf/ => 1,                   # 3.3.7
		 qq/f7 bf bf/ => 1,                # 3.3.8
		 qq/fb bf bf bf/ => 1,             # 3.3.9
		 qq/fd bf bf bf bf/ => 1,          # 3.3.10
		 qq/c0 e0 80 f0 80 80 f8 80 80 80 fc 80 80 80 80 df ef bf f7 bf bf fb bf bf bf fd bf bf bf bf/ => 1, # 3.4.1
		 qq/fe/ => 1,                      # 3.5.1
		 qq/ff/ => 1,                      # 3.5.2
		 qq/fe fe ff ff/ => 1,             # 3.5.3
		 qq/c0 af/ => 1,                   # 4.1.1
		 qq/e0 80 af/ => 1,                # 4.1.2
		 qq/f0 80 80 af/ => 1,             # 4.1.3
		 qq/f8 80 80 80 af/ => 1,          # 4.1.4
		 qq/fc 80 80 80 80 af/ => 1,       # 4.1.5
		 qq/c1 bf/ => 1,                   # 4.2.1
		 qq/e0 9f bf/ => 1,                # 4.2.2
		 qq/f0 8f bf bf/ => 1,             # 4.2.3
		 qq/f8 87 bf bf bf/ => 1,          # 4.2.4
		 qq/fc 83 bf bf bf bf/ => 1,       # 4.2.5
		 qq/c0 80/ => 1,                   # 4.3.1
		 qq/e0 80 80/ => 1,                # 4.3.2
		 qq/f0 80 80 80/ => 1,             # 4.3.3
		 qq/f8 80 80 80 80/ => 1,          # 4.3.4
		 qq/fc 80 80 80 80 80/ => 1,       # 4.3.5
		 );
     }
     $NTESTS +=  scalar keys %SEQ;
}
use strict;
use Encode;
use utf8;
use Test::More tests => $NTESTS;

local($SIG{__WARN__}) = sub { $DEBUG and $@ and print STDERR $@ };

my $d = find_encoding("utf-8-strict");
for my $u (sort keys %ORD){
    my $c = chr($u);
    eval { $d->encode($c,1) };
    $DEBUG and $@ and warn $@;
    my $t = $@ ? 1 : 0;
    is($t, $ORD{$u}, sprintf "U+%04X", $u);
}
for my $s (sort keys %SEQ){
    my $o = pack "C*" => map {hex} split /\s+/, $s;
    eval { $d->decode($o,1) };
    $DEBUG and $@ and warn $@;
    my $t = $@ ? 1 : 0;
    is($t, $SEQ{$s}, "sequence: $s");
}

__END__


