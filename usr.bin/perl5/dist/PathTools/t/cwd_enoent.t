use warnings;
use strict;

use Config;
use Errno qw();
use File::Temp qw(tempdir);
use Test::More;

if($^O eq "cygwin") {
    # This test skipping should be removed when the Cygwin bug is fixed.
    plan skip_all => "getcwd() fails to fail on Cygwin [perl #132733]";
}

my $tmp = tempdir(CLEANUP => 1);
unless(mkdir("$tmp/testdir") && chdir("$tmp/testdir") && rmdir("$tmp/testdir")){
    plan skip_all => "can't be in non-existent directory";
}

plan tests => 8;
require Cwd;

my @acceptable_errnos = (&Errno::ENOENT, (defined &Errno::ESTALE ? &Errno::ESTALE : ()));
foreach my $type (qw(regular perl)) {
    SKIP: {
	skip "_perl_abs_path() not expected to work", 4
	    if $type eq "perl" &&
		!(($Config{prefix} =~ m/\//) && $^O ne "cygwin");

        # https://github.com/Perl/perl5/issues/16525
        # https://bugs.dragonflybsd.org/issues/3250
        my @vlist = ($Config{osvers} =~ /(\d+)/g);
        my $osver = sprintf("%d%03d", map { defined() ? $_ : '0' } @vlist[0,1]);
	skip "getcwd() doesn't fail on non-existent directories on this platform", 4
	    if $type eq 'regular' && $^O eq 'dragonfly' && $osver < 6002;

	skip "getcwd() doesn't fail on non-existent directories on this platform", 4
	    if $type eq 'regular' && $^O eq 'haiku';

	no warnings "redefine";
	local *Cwd::abs_path = \&Cwd::_perl_abs_path if $type eq "perl";
	local *Cwd::getcwd = \&Cwd::_perl_getcwd if $type eq "perl";
	my($res, $eno);
	$! = 0;
	$res = Cwd::getcwd();
	$eno = 0+$!;
	is $res, undef, "$type getcwd result on non-existent directory";
	ok((grep { $eno == $_ } @acceptable_errnos), "$type getcwd errno on non-existent directory")
	    or diag "Got errno code $eno, expected " . join(", ", @acceptable_errnos);
	$! = 0;
	$res = Cwd::abs_path(".");
	$eno = 0+$!;
	is $res, undef, "$type abs_path result on non-existent directory";
	ok((grep { $eno == $_ } @acceptable_errnos), "$type abs_path errno on non-existent directory")
	    or diag "Got errno code $eno, expected " . join(", ", @acceptable_errnos);
    }
}

chdir $tmp or die "$tmp: $!";

END { chdir $tmp; }

1;
