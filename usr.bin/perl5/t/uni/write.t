#!./perl -w
use strict;

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    skip_all_without_perlio();
}

plan tests => 8;

# Some tests for UTF8 and format/write

our ($bitem1, $uitem1) = ("\x{ff}", "\x{100}");
our ($bitem2, $uitem2) = ("\x{fe}", "\x{101}");
our ($blite1, $ulite1) = ("\x{fd}", "\x{102}");
our ($blite2, $ulite2) = ("\x{fc}", "\x{103}");
our ($bmulti, $umulti) = ("\x{fb}\n\x{fa}\n\x{f9}\n",
			  "\x{104}\n\x{105}\n\x{106}\n");

sub fmwrtest {
  no strict 'refs';
  my ($out, $format, $expect, $name) = @_;
  eval "format $out =\n$format.\n"; die $@ if $@;
  open $out, '>:utf8', 'Uni_write.tmp' or die "Can't create Uni_write.tmp";
  write $out;
  close $out or die "Could not close $out: $!";

  open UIN, '<:utf8', 'Uni_write.tmp' or die "Can't open Uni_write.tmp";;
  my $result = do { local $/; <UIN>; };
  close UIN;

  is($result, $expect, $name);
}

fmwrtest OUT1 => <<EOFORMAT, <<EOEXPECT, "non-UTF8 literal / UTF8 item (1)";
$blite1 @<<
\$uitem1
$blite2 @<<
\$bitem2
EOFORMAT
$blite1 $uitem1
$blite2 $bitem2
EOEXPECT

fmwrtest OUT2 => <<EOFORMAT, <<EOEXPECT, "non-UTF8 literal / UTF8 item (2)";
$blite1 @<<
\$bitem1
$blite2 @<<
\$uitem2
EOFORMAT
$blite1 $bitem1
$blite2 $uitem2
EOEXPECT

fmwrtest OUT3 => <<EOFORMAT, <<EOEXPECT, "UTF8 literal / non-UTF8 item (1)";
$ulite1 @<<
\$bitem1
$blite2 @<<
\$bitem2
EOFORMAT
$ulite1 $bitem1
$blite2 $bitem2
EOEXPECT

fmwrtest OUT4 => <<EOFORMAT, <<EOEXPECT, "UTF8 literal / non-UTF8 item (2)";
$blite1 @<<
\$bitem1
$ulite2 @<<
\$bitem2
EOFORMAT
$blite1 $bitem1
$ulite2 $bitem2
EOEXPECT

fmwrtest OUT5 => <<EOFORMAT, <<EOEXPECT, "non-UTF8 literal / UTF8 multiline";
$blite1
@*
\$umulti
$blite2
EOFORMAT
$blite1
$umulti$blite2
EOEXPECT

fmwrtest OUT6 => <<EOFORMAT, <<EOEXPECT, "UTF8 literal / non-UTF8 multiline";
$ulite1
@*
\$bmulti
$blite2
EOFORMAT
$ulite1
$bmulti$blite2
EOEXPECT

{
    use utf8;
    use open qw( :utf8 :std );

    local $~ = "놋웇ʱＦᚖṀŦ";
    eval { write };
    like $@, qr/Undefined format "놋웇ʱＦᚖṀŦ/u, 'no such format, with format name in UTF-8.';
}

{

format OUT =


.
    use utf8;
    use open qw( :utf8 :std );
    open OUT, '>', 'Uni_write2.tmp';

    my $oldfh = select OUT;
    local $^ = "უデﬁᕣネḓ_ＦᚖṀŦɐȾ";#"UNDEFINED_FORMAT";
    eval { write };
    like $@, qr/Undefined top format "უデﬁᕣネḓ_ＦᚖṀŦɐȾ/u, 'no such top format';
    select $oldfh;
    close OUT;
}

unlink_all qw( Uni_write.tmp Uni_write2.tmp );
