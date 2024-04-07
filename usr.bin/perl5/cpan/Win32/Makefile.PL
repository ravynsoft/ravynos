use 5.006;
use strict;
use warnings;
use ExtUtils::MakeMaker;

unless ($^O eq "MSWin32" || $^O eq "cygwin") {
    die "OS unsupported\n";
}

my %param = (
    NAME          => 'Win32',
    VERSION_FROM  => 'Win32.pm',
    INSTALLDIRS   => ($] >= 5.008004 && $] < 5.012 ? 'perl' : 'site'),
    PREREQ_PM     => {
        strict => 0,
        warnings => 0,
        vars => 0,
        Exporter => 0,
        DynaLoader => 0
    }
);
$param{NO_META} = 1 if eval "$ExtUtils::MakeMaker::VERSION" >= 6.10_03;

if ($^O eq 'cygwin') {
    $param{LIBS} = ['-L/lib/w32api -lole32 -lversion -luserenv -lnetapi32 -lwinhttp']
}
else {
    $param{LIBS} = ['-luserenv -lwinhttp']
}

my $test_requires = $ExtUtils::MakeMaker::VERSION >= 6.64
    ? 'TEST_REQUIRES'
    : 'PREREQ_PM';

$param{$test_requires}{'Test'} = 0;
$param{$test_requires}{'File::Temp'} = 0;

WriteMakefile(%param);
