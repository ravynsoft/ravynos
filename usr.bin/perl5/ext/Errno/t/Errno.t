#!./perl -w

use Test::More tests => 12;

# Keep this before the use Errno.
my $has_einval = exists &Errno::EINVAL;

BEGIN {
    use_ok("Errno");
}

BAIL_OUT("No errno's are exported") unless @Errno::EXPORT_OK;

my $err = $Errno::EXPORT_OK[0];
my $num = &{"Errno::$err"};

is($num, &{"Errno::$err"},
    'element in @Errno::EXPORT_OK found via sub call');

$! = $num;
ok(exists $!{$err}, 'entry in %! reflects current value of $!');

$! = 0;
ok(! $!{$err}, 'entry in %! reflects the current value of $!');

ok(join(",",sort keys(%!)) eq join(",",sort @Errno::EXPORT_OK),
    'keys of %! match keys of @Errno::EXPORT_OK');

eval { exists $!{[]} };
ok(! $@, "no exception recorded in %! when element's key is '[]'");

eval {$!{$err} = "qunckkk" };
like($@, qr/^ERRNO hash is read only!/,
    "can't assign to ERRNO hash: 'ERRNO hash is read only!'");

eval {delete $!{$err}};
like($@, qr/^ERRNO hash is read only!/,
    "can't delete from ERRNO hash: 'ERRNO hash is read only!'");

# The following tests are in trouble if some OS picks errno values
# through Acme::MetaSyntactic::batman
is($!{EFLRBBB}, "", "non-existent entry in ERRNO hash");
ok(! exists($!{EFLRBBB}), "non-existent entry in ERRNO hash");

SKIP: {
    skip("Errno does not have EINVAL", 1)
	unless grep {$_ eq 'EINVAL'} @Errno::EXPORT_OK;
    is($has_einval, 1,
       'exists &Errno::EINVAL compiled before Errno is loaded works fine');
}

SKIP: {
    skip("Errno does not have EBADF", 1)
	unless grep {$_ eq 'EBADF'} @Errno::EXPORT_OK;
    is(exists &Errno::EBADF, 1,
       'exists &Errno::EBADF compiled after Errno is loaded works fine');
}
