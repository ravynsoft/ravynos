#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_if_miniperl("miniperl can't load PerlIO::scalar");
}

plan(tests => 2*11*29);

use Errno qw(EBADF ENOENT);

open(SCALARFILE, "<", \"wibble") or die $!; # needs PerlIO::scalar
open(CLOSEDFILE, "<", "./test.pl") or die $!;
close(CLOSEDFILE) or die $!;
opendir(CLOSEDDIR, "../lib") or die $!;
closedir(CLOSEDDIR) or die $!;

foreach my $op (
    qw(stat lstat),
    (map { "-$_" } qw(r w x o R W X O e z s f d l p S b c t u g k T B M A C)),
) {
    foreach my $arg (
	(map { ($_, "\\*$_") }
	    qw(NEVEROPENED SCALARFILE CLOSEDFILE CLOSEDDIR _)),
	"\"tmpnotexist\"",
    ) {
	my $argdesc = $arg;
	if ($arg eq "_") {
	    my @z = lstat "tmpnotexist";
	    $argdesc .= " with prior stat fail";
	}
	SKIP: {
	    if ($op eq "-l" && $arg =~ /\A\\/) {
		# The op weirdly stringifies the globref and uses it as
		# a filename, rather than treating it as a file handle.
		# That might be a bug, but while that behaviour exists it
		# needs to be exempted from these tests.
		skip "-l on globref", 2;
	    }
	    if ($op eq "-t" && $arg eq "\"tmpnotexist\"") {
		# The op doesn't operate on filenames.
		skip "-t on filename", 2;
	    }
	    $! = 0;
	    my $res = eval "$op $arg";
	    my $err = $!;
	    is $res, $op =~ /\A-/ ? undef : !!0, "result of $op $arg";
	    is 0+$err,
		$arg eq "\"tmpnotexist\"" ||
		    ($op =~ /\A-[TB]\z/ && $arg =~ /_\z/) ? ENOENT : EBADF,
		"error from $op $arg";
	}
    }
}

1;
