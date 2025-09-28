# gcc -O3 (and -O2) get overly excited over B.c in OpenBSD 3.3/sparc 64
use Config;

if ($Config{ARCH} eq 'sparc64') {
    my $optimize = $Config{optimize};
    $optimize =~ s/(^| )-O[2-9]\b/$1-O1/g
                 and $self->{OPTIMIZE} = $optimize;
}
