use strict;
use warnings;

our $TIME;
BEGIN {
    *CORE::GLOBAL::time = sub() {
        return CORE::time() unless defined $TIME;
        return $TIME;
    };
}

use Config qw/%Config/;

use Test2::Tools::Tiny;
use Test2::Util qw/
    try

    get_tid USE_THREADS

    pkg_to_file

    CAN_FORK
    CAN_THREAD
    CAN_REALLY_FORK

    ipc_separator
    gen_uid

    CAN_SIGSYS

    IS_WIN32

    clone_io
/;

BEGIN {
    if ($] lt "5.008") {
        require Test::Builder::IO::Scalar;
    }
}

{
    for my $try (\&try, Test2::Util->can('_manual_try'), Test2::Util->can('_local_try')) {
        my ($ok, $err) = $try->(sub { die "xxx" });
        ok(!$ok, "cought exception");
        like($err, qr/xxx/, "expected exception");

        ($ok, $err) = $try->(sub { 0 });
        ok($ok,   "Success");
        ok(!$err, "no error");
    }
}

is(pkg_to_file('A::Package::Name'), 'A/Package/Name.pm', "Converted package to file");

# Make sure running them does not die
# We cannot really do much to test these.
CAN_THREAD();
CAN_FORK();
CAN_REALLY_FORK();
IS_WIN32();

is(IS_WIN32(), ($^O eq 'MSWin32') ? 1 : 0, "IS_WIN32 is correct ($^O)");

my %sigs = map {$_ => 1} split /\s+/, $Config{sig_name};
if ($sigs{SYS}) {
    ok(CAN_SIGSYS, "System has SIGSYS");
}
else {
    ok(!CAN_SIGSYS, "System lacks SIGSYS");
}

my $check_for_sig_sys = Test2::Util->can('_check_for_sig_sys');
ok($check_for_sig_sys->("FOO SYS BAR"), "Found SIGSYS in the middle");
ok($check_for_sig_sys->("SYS FOO BAR"), "Found SIGSYS at start");
ok($check_for_sig_sys->("FOO BAR SYS"), "Found SIGSYS at end");
ok(!$check_for_sig_sys->("FOO SYSX BAR"), "SYSX is not SYS");
ok(!$check_for_sig_sys->("FOO XSYS BAR"), "XSYS is not SYS");

my $io = clone_io(\*STDOUT);
ok($io, "Cloned the filehandle");
close($io);

my $fh;
my $out = '';
if ($] ge "5.008") {
    open($fh, '>', \$out) or die "Could not open filehandle";
} else {
    $fh = Test::Builder::IO::Scalar->new(\$out) or die "Could not open filehandle";
}

$io = clone_io($fh);
is($io, $fh, "For a scalar handle we simply return the original handle, no other choice");
print $io "Test\n";

is($out, "Test\n", "wrote to the scalar handle");

is(ipc_separator(), '~', "Got ipc_separator");

{
    local $TIME = time;
    my $id1 = gen_uid();
    my $id2 = gen_uid();

    like($id1, qr/^\Q$$~0~$TIME~\E\d+$/, "Got a UID ($id1)");
    my ($inc) = ($id1 =~ m/(\d+)$/g);
    $inc++;
    is($id2, "$$~0~$TIME~$inc", "Next id is next in sequence ($id2)");
}

done_testing;
