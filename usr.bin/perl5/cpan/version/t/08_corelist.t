#! /usr/local/perl -w
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More tests => 3;
use_ok("version", 0.9929);

# do strict lax tests in a sub to isolate a package to test importing
SKIP: {
    eval "use Module::CoreList 2.76";
    skip 'No tied hash in Modules::CoreList in Perl', 2
	if $@;

    my $foo = "version"->parse($Module::CoreList::version{5.008_000}{base});

    is $foo, 1.03, 'Correctly handle tied hash';

    $foo = "version"->qv($Module::CoreList::version{5.008_000}{Unicode});
    is $foo, '3.2.0', 'Correctly handle tied hash with dotted decimal';
}
