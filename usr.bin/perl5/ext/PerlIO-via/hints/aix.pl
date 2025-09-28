# compilation may hang at -O3 level
use Config;

my $optimize = $Config{optimize};
$optimize =~ s/(^| )-O[2-9]\b/$1-O/g
             and $self->{OPTIMIZE} = $optimize;
