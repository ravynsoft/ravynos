#!./miniperl -w
use strict;
use Config;
use 5.012;
die "Can't fork" unless $Config{d_fork};

# Brute force testing for makedef.pl
#
# To use this...
#
# Before modifying makedef.pl, create your golden results:
#
# $ mkdir Gold
# $ ./perl -Ilib Porting/exercise_makedef.pl Gold/
# $ chmod -R -w Gold/
# $ mkdr Test
#
# then modify makedef.pl
#
# then test
#
# $ ./perl -Ilib Porting/exercise_makedef.pl Test
# $ diff -rpu Gold Test

my $prefix = shift;
die "$0 prefix" unless $prefix;
die "No such directory $prefix" unless -d $prefix;

my @unlink;
sub END {
    unlink @unlink;
}

$SIG{INT} = sub { die }; # Trigger END processing

{
    # needed for OS/2, so fake one up
    my $mpm = 'miniperl.map';

    die "$mpm exists" if -e $mpm;

    open my $in, '<', 'av.c' or die "Can't open av.c: $!";
    push @unlink, $mpm;
    open my $out, '>', $mpm or die "Can't open $mpm: $!";
    while (<$in>) {
	print $out "f $1\n" if /^(Perl_[A-Za-z_0-9]+)\(pTHX/;
    }
    close $out or die "Can't close $mpm: $!";
}

my @args = (platform => [map {"PLATFORM=$_"} qw(aix win32 os2 vms test)],
	    cflags => ['', 'CCFLAGS=-Dperl=rules -Dzzz'],
	    Deq => ['', '-Dbeer=foamy'],
	    D => ['', '-DPERL_IMPLICIT_SYS'],
	    cctype => ['', 'CCTYPE=GCC'],
	    filetype => ['', 'FILETYPE=def', 'FILETYPE=imp'],
	    targ_dir => ['', 'TARG_DIR=t/../'],
	   );

sub expand {
    my ($names, $args, $key, $vals, @rest) = @_;
    if (defined $key) {
	my $bad;
	while (my ($i, $v) = each @$vals) {
	    $bad += expand([@$names, "$key=$i"], [@$args, $v], @rest);
	}
	return $bad;
    }
    # time to process something:
    my $name = join ',', @$names;
    my @args = grep {length} @$args;

    $ENV{PERL5LIB} = join $Config{path_sep}, @INC;
    my $pid = fork;
    unless ($pid) {
	open STDOUT, '>', "$prefix/$name.out"
	    or die "Can't open $prefix/$name.out: $!";
	open STDERR, '>', "$prefix/$name.err"
	    or die "Can't open $prefix/$name.err: $!";
	exec $^X, 'makedef.pl', @args;
	die "Something went horribly wrong: $!";
    }
    die "Bad waitpid: $!" unless waitpid $pid, 0 == $pid;
    if ($?) {
	print STDERR "`$^X makedef.pl @args` failed with $?\n";
	print STDERR "See output in $prefix/$name.err\n";
	return 1;
    }
    return 0;
}

my $bad = expand([], [], @args);
exit($bad > 255 ? 255 : $bad);
