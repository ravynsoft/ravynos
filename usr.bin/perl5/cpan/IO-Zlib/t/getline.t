use strict;
use warnings;

use IO::Zlib;

sub ok
{
    my ($no, $ok) = @_ ;
    print "ok $no\n" if $ok ;
    print "not ok $no\n" unless $ok ;
}

my $name = "test_getline_$$.gz";

print "1..23\n";

my @text = (<<EOM, <<EOM, <<EOM, <<EOM) ;
this is line 1
EOM
the second line
EOM
the line after the previous line
EOM
the final line
EOM

my $text = join("", @text) ;
my $file;

ok(1, $file = IO::Zlib->new($name, "wb"));
ok(2, $file->print($text));
ok(3, $file->close());

ok(4, $file = IO::Zlib->new($name, "rb"));
ok(5, !$file->eof());
ok(6, $file->getline() eq $text[0]);
ok(7, $file->getline() eq $text[1]);
ok(8, $file->getline() eq $text[2]);
ok(9, $file->getline() eq $text[3]);
ok(10, !defined($file->getline()));
ok(11, $file->eof());
ok(12, $file->close());

my @lines;

ok(13, $file = IO::Zlib->new($name, "rb"));
ok(14, !$file->eof());
eval '$file->getlines';
ok(15, $@ =~ /^IO::Zlib::getlines: must be called in list context /);
ok(16, @lines = $file->getlines());
ok(17, @lines == @text);
ok(18, $lines[0] eq $text[0]);
ok(19, $lines[1] eq $text[1]);
ok(20, $lines[2] eq $text[2]);
ok(21, $lines[3] eq $text[3]);
ok(22, $file->eof());
ok(23, $file->close());

unlink($name);
