#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "../t/test.pl";
    set_up_inc('../lib');
    skip_all_without_perlio();
}

plan (15);

use warnings 'layer';
my $warn;
my $file = "fail$$";
$SIG{__WARN__} = sub { $warn = shift };

END { 1 while unlink($file) }

ok(open(FH,">",$file),"Create works");
close(FH);
ok(open(FH,"<",$file),"Normal open works");

$warn = ''; $! = 0;
ok(!binmode(FH,":-)"),"All punctuation fails binmode");
print "# $!\n";
isnt($!,0,"Got errno");
like($warn,qr/in PerlIO layer/,"Got warning");

$warn = ''; $! = 0;
ok(!binmode(FH,":nonesuch"),"Bad package fails binmode");
print "# $!\n";
isnt($!,0,"Got errno");
like($warn,qr/nonesuch/,"Got warning");
close(FH);

$warn = ''; $! = 0;
ok(!open(FH,"<:-)",$file),"All punctuation fails open");
print "# $!\n";
isnt($!,"","Got errno");
like($warn,qr/in PerlIO layer/,"Got warning");

$warn = ''; $! = 0;
ok(!open(FH,"<:nonesuch",$file),"Bad package fails open");
print "# $!\n";
isnt($!,0,"Got errno");
like($warn,qr/nonesuch/,"Got warning");

ok(open(FH,"<",$file),"Normal open (still) works");
close(FH);
