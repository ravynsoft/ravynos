# gcc -O3 (and -O2) get overly excited over B.c in MacOS X 10.1.4.
use Config;

my $optimize = $Config{optimize};
$optimize =~ s/(^| )-O[2-9]\b/$1-O1/g
             and $self->{OPTIMIZE} = $optimize;
