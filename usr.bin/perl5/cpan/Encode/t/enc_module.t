# $Id: enc_module.t,v 2.6 2022/04/07 03:06:40 dankogai Exp $
# This file is in euc-jp
BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bEncode\b/) {
      print "1..0 # Skip: Encode was not built\n";
      exit 0;
    }
    unless (find PerlIO::Layer 'perlio') {
    print "1..0 # Skip: PerlIO was not built\n";
    exit 0;
    }
    if (defined ${^UNICODE} and ${^UNICODE} != 0){
    print "1..0 # Skip: \${^UNICODE} == ${^UNICODE}\n";
    exit 0;
    }
    if (ord("A") == 193) {
    print "1..0 # Skip: encoding pragma does not support EBCDIC platforms\n";
    exit(0);
    }
    if ($] >= 5.025 and !$Config{usecperl}) {
    print "1..0 # Skip: encoding pragma not supported in Perl 5.25 or later\n";
    exit(0);
    }
}
use lib qw(t ext/Encode/t ../ext/Encode/t); # latter 2 for perl core
use Mod_EUCJP;
no warnings "deprecated";
use encoding "euc-jp";
use Test::More tests => 3;
use File::Basename;
use File::Spec;
use File::Compare qw(compare_text);

my $DEBUG = shift || 0;
my $dir = dirname(__FILE__);
my $file0 = File::Spec->catfile($dir,"enc_module.enc");
my $file1 = File::Spec->catfile($dir,"$$.enc");

my $obj = Mod_EUCJP->new;
local $SIG{__WARN__} = sub{ $DEBUG and print STDERR @_ };
# to silence reopening STD(IN|OUT) w/o closing unless $DEBUG

open STDOUT, ">", $file1 or die "$file1:$!";
print $obj->str, "\n";
$obj->set("テスト文字列");
print $obj->str, "\n";

# Please do not move this to a point after the comparison -- Craig Berry
# and "unless $^O eq 'freebsd'" is needed for FreeBSD (toy-)?thread
# -- dankogai
close STDOUT unless $^O eq 'freebsd';

my $cmp = compare_text($file0, $file1);
is($cmp, 0, "encoding vs. STDOUT");

my @cmp = qw/初期文字列 テスト文字列/;
open STDIN, "<", $file0 or die "$file0:$!";
$obj = Mod_EUCJP->new;
my $i = 0;
while(<STDIN>){
    s/\r?\n\z//;
    is ($cmp[$i++], $_, "encoding vs. STDIN - $i");
}

unlink $file1 unless $cmp;
__END__

