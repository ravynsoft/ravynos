#!./perl

BEGIN {
    unless (find PerlIO::Layer 'perlio') {
	print "1..0 # No perlio\n";
	exit 0;
    }
    if (ord("A") == 193) {
	print "1..0 # Skip: EBCDIC\n";
	exit 0;
    }
    unless ( eval { require Encode } ) {
	print "1..0 # Skip: No Encode\n";
	exit 0;
    }
    import Encode qw(:fallback_all);
}

use Test::More tests => 10;

# $PerlIO::encoding = 0; # WARN_ON_ERR|PERLQQ;

my $file = "fallback$$.txt";

{
    use warnings;
    my $message = '';
    local $SIG{__WARN__} = sub { $message = $_[0] };
    $PerlIO::encoding::fallback = Encode::PERLQQ;
    ok(open(my $fh,">encoding(iso-8859-1)",$file),"opened iso-8859-1 file");
    my $str = "\x{20AC}";
    print $fh $str,"0.02\n";
    close($fh);
    like($message, qr/does not map to iso-8859-1/o, "FB_WARN message");
}

open($fh,'<',$file) || die "File cannot be re-opened";
my $line = <$fh>;
is($line,"\\x{20ac}0.02\n","perlqq escapes");
close($fh);

$PerlIO::encoding::fallback = Encode::HTMLCREF;

ok(open(my $fh,">encoding(iso-8859-1)",$file),"opened iso-8859-1 file");
my $str = "\x{20AC}";
print $fh $str,"0.02\n";
close($fh);

open($fh,'<',$file) || die "File cannot be re-opened";
my $line = <$fh>;
is($line,"&#8364;0.02\n","HTML escapes");
close($fh);

{
    no utf8;
    open($fh,'>',$file) || die "File cannot be re-opened";
    binmode($fh);
    print $fh "\xA30.02\n";
    close($fh);
}

ok(open($fh,"<encoding(US-ASCII)",$file),"Opened as ASCII");
my $line = <$fh>;
printf "# %x\n",ord($line);
is($line,"\\xA30.02\n","Escaped non-mapped char");
close($fh);

{
    my $message = '';
    local $SIG{__WARN__} = sub { $message = $_[0] };

    $PerlIO::encoding::fallback = Encode::WARN_ON_ERR;

    ok(open($fh,"<encoding(US-ASCII)",$file),"Opened as ASCII");
    my $line = <$fh>;
    printf "# %x\n",ord($line);
    is($line,"\x{FFFD}0.02\n","Unicode replacement char");
    close($fh);

    like($message, qr/does not map to Unicode/o, "FB_WARN message");
}

END {
    1 while unlink($file);
}
