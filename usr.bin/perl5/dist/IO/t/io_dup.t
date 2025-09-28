#!./perl

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
use IO::File;

select(STDERR); $| = 1;
select(STDOUT); $| = 1;

print "1..6\n";

print "ok 1\n";

my $dupout = IO::Handle->new->fdopen( \*STDOUT ,"w");
my $duperr = IO::Handle->new->fdopen( \*STDERR ,"w");

my $stdout = \*STDOUT; bless $stdout, "IO::File"; # "IO::Handle";
my $stderr = \*STDERR; bless $stderr, "IO::Handle";

$stdout->open( "Io.dup","w") || die "Can't open stdout";
$stderr->fdopen($stdout,"w");

print $stdout "ok 2\n";
print $stderr "ok 3\n";

# Since some systems don't have echo, we use Perl.
my $echo = qq{$^X -le "print q(ok %d)"};

my $cmd = sprintf $echo, 4;
print `$cmd`;

$cmd = sprintf "$echo 1>&2", 5;
$cmd = sprintf $echo, 5 if $^O eq 'MacOS';
print `$cmd`;

$stderr->close;
$stdout->close;

$stdout->fdopen($dupout,"w");
$stderr->fdopen($duperr,"w");

if ($^O eq 'MSWin32' || $^O eq 'NetWare' || $^O eq 'VMS') { print `type Io.dup` }
elsif ($^O eq 'MacOS') { system 'Catenate Io.dup' }
else                   { system 'cat Io.dup' }
unlink 'Io.dup';

print STDOUT "ok 6\n";
