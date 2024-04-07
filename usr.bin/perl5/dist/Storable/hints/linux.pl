# gcc -O3 (and higher) can cause code produced from Storable.xs that
# dumps core immediately in recurse.t and retrieve.t, in is_storing()
# and last_op_in_netorder(), respectively.  In both cases the cxt is
# full of junk (and according to valgrind the cxt was never stack'd,
# malloc'd or free'd).  Observed in Debian 3.0 x86, with gccs 2.95.4
# 20011002 and 3.3, and in Redhat 7.1 with gcc 3.3.1. The failures
# happen only for unthreaded builds, threaded builds work okay.
use Config;
if ($Config{gccversion} and !$Config{usethreads}) {
    my $optimize = $Config{optimize};
    # works fine with gcc 4 or clang
    if ($optimize =~ s/(^| )-O[3-9]( |$)/$1-O2$2/ and $Config{gccversion} =~ /^[23]\./) {
	$self->{OPTIMIZE} = $optimize;
    }
}

