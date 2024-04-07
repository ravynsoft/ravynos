use strict;
use warnings;

BEGIN {
        if ($ENV{PERL_CORE}) {
                chdir 't' if -d 't';
                @INC = '../lib';
        }
}

use MIME::QuotedPrint;

my $x70 = "x" x 70;

my $IsASCII  = ord('A') == 65;
my $IsEBCDIC = ord('A') == 193;

my @tests;

if ($IsASCII) {

@tests =
  (
   # plain ascii should not be encoded
   ["", ""],
   ["quoted printable"  =>
    "quoted printable=\n"],

   # 8-bit chars should be encoded
   ["v\xe5re kj\xe6re norske tegn b\xf8r \xe6res" =>
    "v=E5re kj=E6re norske tegn b=F8r =E6res=\n"],

   # trailing space should be encoded
   ["  " => "=20=20=\n"],
   ["\tt\t" => "\tt=09=\n"],
   ["test  \ntest\n\t \t \n" => "test=20=20\ntest\n=09=20=09=20\n"],

   # "=" is special an should be decoded
   ["=30\n" => "=3D30\n"],
   ["\0\xff0" => "=00=FF0=\n"],

   # Very long lines should be broken (not more than 76 chars
   ["The Quoted-Printable encoding is intended to represent data that largly consists of octets that correspond to printable characters in the ASCII character set." =>
    "The Quoted-Printable encoding is intended to represent data that largly con=
sists of octets that correspond to printable characters in the ASCII charac=
ter set.=\n"
    ],

   # Long lines after short lines were broken through 2.01.
   ["short line
In America, any boy may become president and I suppose that's just one of the risks he takes. -- Adlai Stevenson" =>
    "short line
In America, any boy may become president and I suppose that's just one of t=
he risks he takes. -- Adlai Stevenson=\n"],

   # My (roderick@argon.org) first crack at fixing that bug failed for
   # multiple long lines.
   ["College football is a game which would be much more interesting if the faculty played instead of the students, and even more interesting if the
trustees played.  There would be a great increase in broken arms, legs, and necks, and simultaneously an appreciable diminution in the loss to humanity. -- H. L. Mencken" =>
    "College football is a game which would be much more interesting if the facu=
lty played instead of the students, and even more interesting if the
trustees played.  There would be a great increase in broken arms, legs, and=
 necks, and simultaneously an appreciable diminution in the loss to humanit=
y. -- H. L. Mencken=\n"],

   # Don't break a line that's near but not over 76 chars.
   ["$x70!23"		=> "$x70!23=\n"],
   ["$x70!234"		=> "$x70!234=\n"],
   ["$x70!2345"		=> "$x70!2345=\n"],
   ["$x70!23456"	=> "$x70!2345=\n6=\n"],
   ["$x70!234567"	=> "$x70!2345=\n67=\n"],
   ["$x70!23456="	=> "$x70!2345=\n6=3D=\n"],
   ["$x70!23\n"		=> "$x70!23\n"],
   ["$x70!234\n"	=> "$x70!234\n"],
   ["$x70!2345\n"	=> "$x70!2345\n"],
   ["$x70!23456\n"	=> "$x70!23456\n"],
   ["$x70!234567\n"	=> "$x70!2345=\n67\n"],
   ["$x70!23456=\n"	=> "$x70!2345=\n6=3D\n"],

   # Not allowed to break =XX escapes using soft line break
   ["$x70===xxxxx"  => "$x70=3D=\n=3D=3Dxxxxx=\n"],
   ["$x70!===xxxx"  => "$x70!=3D=\n=3D=3Dxxxx=\n"],
   ["$x70!2===xxx"  => "$x70!2=3D=\n=3D=3Dxxx=\n"],
   ["$x70!23===xx"  => "$x70!23=\n=3D=3D=3Dxx=\n"],
   ["$x70!234===x"  => "$x70!234=\n=3D=3D=3Dx=\n"],
   ["$x70!2="       => "$x70!2=3D=\n"],
   ["$x70!23="      => "$x70!23=\n=3D=\n"],
   ["$x70!234="     => "$x70!234=\n=3D=\n"],
   ["$x70!2345="    => "$x70!2345=\n=3D=\n"],
   ["$x70!23456="   => "$x70!2345=\n6=3D=\n"],
   ["$x70!2=\n"     => "$x70!2=3D\n"],
   ["$x70!23=\n"    => "$x70!23=3D\n"],
   ["$x70!234=\n"   => "$x70!234=\n=3D\n"],
   ["$x70!2345=\n"  => "$x70!2345=\n=3D\n"],
   ["$x70!23456=\n" => "$x70!2345=\n6=3D\n"],
   #                              ^
   #                      70123456|
   #                             max
   #                          line width

   # some extra special cases we have had problems with
   ["$x70!2=x=x" => "$x70!2=3D=\nx=3Dx=\n"],
   ["$x70!2345$x70!2345$x70!23456\n", "$x70!2345=\n$x70!2345=\n$x70!23456\n"],

   # trailing whitespace
   ["foo \t ", "foo=20=09=20=\n"],
   ["foo\t \n \t", "foo=09=20\n=20=09=\n"],
);

} elsif ($IsEBCDIC) {

@tests =
  (
   # plain ascii should not be encoded
   ["", ""],
   ["quoted printable"  =>
    "quoted printable=\n"],

   # 8-bit chars should be encoded
   ["v\x47re kj\x9cre norske tegn b\x70r \x47res" =>
    "v=47re kj=9Cre norske tegn b=70r =47res=\n"],

   # trailing space should be encoded
   ["  " => "=40=40=\n"],
   ["\tt\t" => "\tt=05=\n"],
   ["test  \ntest\n\t \t \n" => "test=40=40\ntest\n=05=40=05=40\n"],

   # "=" is special an should be decoded
   ["=30\n" => "=7E30\n"],
   ["\0\xff0" => "=00=FF0=\n"],

   # Very long lines should be broken (not more than 76 chars
   ["The Quoted-Printable encoding is intended to represent data that largly consists of octets that correspond to printable characters in the ASCII character set." =>
    "The Quoted-Printable encoding is intended to represent data that largly con=
sists of octets that correspond to printable characters in the ASCII charac=
ter set.=\n"
    ],

   # Long lines after short lines were broken through 2.01.
   ["short line
In America, any boy may become president and I suppose that's just one of the risks he takes. -- Adlai Stevenson" =>
    "short line
In America, any boy may become president and I suppose that's just one of t=
he risks he takes. -- Adlai Stevenson=\n"],

   # My (roderick@argon.org) first crack at fixing that bug failed for
   # multiple long lines.
   ["College football is a game which would be much more interesting if the faculty played instead of the students, and even more interesting if the
trustees played.  There would be a great increase in broken arms, legs, and necks, and simultaneously an appreciable diminution in the loss to humanity. -- H. L. Mencken" =>
    "College football is a game which would be much more interesting if the facu=
lty played instead of the students, and even more interesting if the
trustees played.  There would be a great increase in broken arms, legs, and=
 necks, and simultaneously an appreciable diminution in the loss to humanit=
y. -- H. L. Mencken=\n"],

   # Don't break a line that's near but not over 76 chars.
   ["$x70!23"		=> "$x70!23=\n"],
   ["$x70!234"		=> "$x70!234=\n"],
   ["$x70!2345"		=> "$x70!2345=\n"],
   ["$x70!23456"	=> "$x70!2345=\n6=\n"],
   ["$x70!234567"	=> "$x70!2345=\n67=\n"],
   ["$x70!23456="	=> "$x70!2345=\n6=7E=\n"],
   ["$x70!23\n"		=> "$x70!23\n"],
   ["$x70!234\n"	=> "$x70!234\n"],
   ["$x70!2345\n"	=> "$x70!2345\n"],
   ["$x70!23456\n"	=> "$x70!23456\n"],
   ["$x70!234567\n"	=> "$x70!2345=\n67\n"],
   ["$x70!23456=\n"	=> "$x70!2345=\n6=7E\n"],

   # Not allowed to break =XX escapes using soft line break
   ["$x70===xxxxx"  => "$x70=7E=\n=7E=7Exxxxx=\n"],
   ["$x70!===xxxx"  => "$x70!=7E=\n=7E=7Exxxx=\n"],
   ["$x70!2===xxx"  => "$x70!2=7E=\n=7E=7Exxx=\n"],
   ["$x70!23===xx"  => "$x70!23=\n=7E=7E=7Exx=\n"],
   ["$x70!234===x"  => "$x70!234=\n=7E=7E=7Ex=\n"],
   ["$x70!2=\n"     => "$x70!2=7E\n"],
   ["$x70!23=\n"    => "$x70!23=\n=7E\n"],
   ["$x70!234=\n"   => "$x70!234=\n=7E\n"],
   ["$x70!2345=\n"  => "$x70!2345=\n=7E\n"],
   ["$x70!23456=\n" => "$x70!2345=\n6=7E\n"],
   #                              ^
   #                      70123456|
   #                             max
   #                          line width

   # some extra special cases we have had problems with
   ["$x70!2=x=x" => "$x70!2=7E=\nx=7Ex=\n"],
   ["$x70!2345$x70!2345$x70!23456\n", "$x70!2345=\n$x70!2345=\n$x70!23456\n"],

   # trailing whitespace
   ["foo \t ", "foo=40=05=40=\n"],
   ["foo\t \n \t", "foo=05=40\n=40=05=\n"],
);

} else {
  die sprintf "Unknown character set: ord('A') == %d\n", ord('A');
}

my $notests = @tests + 16;
print "1..$notests\n";

my $testno = 0;
for (@tests) {
    $testno++;
    my ($plain, $encoded) = @$_;
    if (ord('A') == 193) {  # EBCDIC 8 bit chars are different
        if ($testno == 2) { $plain =~ s/\xe5/\x47/; $plain =~ s/\xe6/\x9c/g; $plain =~ s/\xf8/\x70/; }
        if ($testno == 7) { $plain =~ s/\xff/\xdf/; }
    }
    my $x = encode_qp($plain);
    if ($x ne $encoded) {
	print "Encode test failed\n";
	print "Got:      '$x'\n";
	print "Expected: '$encoded'\n";
	print "not ok $testno\n";
	next;
    }
    $x = decode_qp($encoded);
    if ($x ne $plain) {
	print "Decode test failed\n";
	print "Got:      '$x'\n";
	print "Expected: '$plain'\n";
	print "not ok $testno\n";
	next;
    }
    print "ok $testno\n";
}

if ($IsASCII) {

# Some extra testing for a case that was wrong until libwww-perl-5.09
print "not " unless decode_qp("foo  \n\nfoo =\n\nfoo=20\n\n") eq
                                "foo\n\nfoo \nfoo \n\n";
$testno++; print "ok $testno\n";

# Same test but with "\r\n" terminated lines
print "not " unless decode_qp("foo  \r\n\r\nfoo =\r\n\r\nfoo=20\r\n\r\n") eq
                                "foo\n\nfoo \nfoo \n\n";
$testno++; print "ok $testno\n";

# Trailing whitespace
print "not " unless decode_qp("foo  ") eq "foo  ";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo  \n") eq "foo\n";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x20\nbar\t\x20\n") eq "foo bar\n";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x20\r\nbar\t\x20\r\n") eq "foo bar\n";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x20\n") eq "foo ";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x20\r\n") eq "foo ";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x20y\r\n") eq "foo = \t\x20y\n";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo =xy\n") eq "foo =xy\n";
$testno++; print "ok $testno\n";

# Test with with alternative line break
print "not " unless encode_qp("$x70!2345$x70\n", "***") eq "$x70!2345=***$x70***";
$testno++; print "ok $testno\n";

# Test with no line breaks
print "not " unless encode_qp("$x70!2345$x70\n", "") eq "$x70!2345$x70=0A";
$testno++; print "ok $testno\n";

# Test binary encoding
print "not " unless encode_qp("foo", undef, 1) eq "foo=\n";
$testno++; print "ok $testno\n";

print "not " unless encode_qp("foo\nbar\r\n", undef, 1) eq "foo=0Abar=0D=0A=\n";
$testno++; print "ok $testno\n";

print "not " unless encode_qp(join("", map chr, 0..255), undef, 1) eq <<'EOT'; $testno++; print "ok $testno\n";
=00=01=02=03=04=05=06=07=08=09=0A=0B=0C=0D=0E=0F=10=11=12=13=14=15=16=17=18=
=19=1A=1B=1C=1D=1E=1F !"#$%&'()*+,-./0123456789:;<=3D>?@ABCDEFGHIJKLMNOPQRS=
TUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~=7F=80=81=82=83=84=85=86=87=88=
=89=8A=8B=8C=8D=8E=8F=90=91=92=93=94=95=96=97=98=99=9A=9B=9C=9D=9E=9F=A0=A1=
=A2=A3=A4=A5=A6=A7=A8=A9=AA=AB=AC=AD=AE=AF=B0=B1=B2=B3=B4=B5=B6=B7=B8=B9=BA=
=BB=BC=BD=BE=BF=C0=C1=C2=C3=C4=C5=C6=C7=C8=C9=CA=CB=CC=CD=CE=CF=D0=D1=D2=D3=
=D4=D5=D6=D7=D8=D9=DA=DB=DC=DD=DE=DF=E0=E1=E2=E3=E4=E5=E6=E7=E8=E9=EA=EB=EC=
=ED=EE=EF=F0=F1=F2=F3=F4=F5=F6=F7=F8=F9=FA=FB=FC=FD=FE=FF=
EOT

print "not " if $] >= 5.006 && (eval 'encode_qp("XXX \x{100}")' || !$@);
$testno++; print "ok $testno\n";

} elsif ($IsEBCDIC) {

# Some extra testing for a case that was wrong until libwww-perl-5.05
print "not " unless decode_qp("foo  \n\nfoo =\n\nfoo=40\n\n") eq
                                "foo\n\nfoo \nfoo \n\n";
$testno++; print "ok $testno\n";

# Same test but with "\r\n" terminated lines
print "not " unless decode_qp("foo  \r\n\r\nfoo =\r\n\r\nfoo=40\r\n\r\n") eq
                                "foo\n\nfoo \nfoo \n\n";
$testno++; print "ok $testno\n";

# Trailing whitespace
print "not " unless decode_qp("foo  ") eq "foo  ";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo  \n") eq "foo\n";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x40\nbar\t\x40\n") eq "foo bar\n";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x40\r\nbar\t\x40\r\n") eq "foo bar\n";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x40\n") eq "foo ";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x40\r\n") eq "foo ";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo = \t\x40y\r\n") eq "foo = \t\x40y\n";
$testno++; print "ok $testno\n";

print "not " unless decode_qp("foo =xy\n") eq "foo =xy\n";
$testno++; print "ok $testno\n";

# Test with with alternative line break
print "not " unless encode_qp("$x70!2345$x70\n", "***") eq "$x70!2345=***$x70***";
$testno++; print "ok $testno\n";

# Test with no line breaks
print "not " unless encode_qp("$x70!2345$x70\n", "") eq "$x70!2345$x70=15";
$testno++; print "ok $testno\n";

# Test binary encoding
print "not " unless encode_qp("foo", undef, 1) eq "foo=\n";
$testno++; print "ok $testno\n";

print "not " unless encode_qp("foo\nbar\r\n", undef, 1) eq "foo=15bar=0D=15=\n";
$testno++; print "ok $testno\n";

print "not " unless encode_qp(join("", map chr, 0..255), undef, 1) eq <<'EOT'; $testno++; print "ok $testno\n";
=00=01=02=03=04=05=06=07=08=09=0A=0B=0C=0D=0E=0F=10=11=12=13=14=15=16=17=18=
=19=1A=1B=1C=1D=1E=1F=20=21=22=23=24=25=26=27=28=29=2A=2B=2C=2D=2E=2F=30=31=
=32=33=34=35=36=37=38=39=3A=3B=3C=3D=3E=3F =41=42=43=44=45=46=47=48=49=4A.<=
(+|&=51=52=53=54=55=56=57=58=59!$*);^-/=62=63=64=65=66=67=68=69=6A,%_>?=70=
=71=72=73=74=75=76=77=78`:#@'=7E"=80abcdefghi=8A=8B=8C=8D=8E=8F=90jklmnopqr=
=9A=9B=9C=9D=9E=9F=A0~stuvwxyz=AA=AB=AC=AD=AE=AF=B0=B1=B2=B3=B4=B5=B6=B7=B8=
=B9=BA=BB=BC=BD=BE=BF{ABCDEFGHI=CA=CB=CC=CD=CE=CF}JKLMNOPQR=DA=DB=DC=DD=DE=
=DF\=E1STUVWXYZ=EA=EB=EC=ED=EE=EF0123456789=FA=FB=FC=FD=FE=FF=
EOT

print "not " if $] >= 5.006 && (eval 'encode_qp("XXX \x{100}")' || !$@);
$testno++; print "ok $testno\n";

}

