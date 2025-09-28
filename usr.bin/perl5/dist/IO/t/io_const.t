use Config;

BEGIN {
    if($ENV{PERL_CORE}) {
        if ($Config{'extensions'} !~ /\bIO\b/) {
            print "1..0 # Skip: IO extension not compiled\n";
            exit 0;
        }
    }
}

use IO::Handle;

print "1..6\n";
my $i = 1;
foreach (qw(SEEK_SET SEEK_CUR SEEK_END     _IOFBF    _IOLBF    _IONBF)) {
    no strict 'refs';
    my $d1 = defined(&{"IO::Handle::" . $_}) ? 1 : 0;
    my $v1 = $d1 ? &{"IO::Handle::" . $_}() : undef;
    my $v2 = IO::Handle::constant($_);
    my $d2 = defined($v2);

    print "not "
	if($d1 != $d2 || ($d1 && ($v1 != $v2)));
    print "ok ",$i++,"\n";
}
