# 2.6 has nanosleep in -lposix4, after that it's in -lrt
my $r = `/usr/bin/uname -r`;
chomp($r);
if (substr($r, 2) <= 6) {
    $self->{LIBS} = ['-lposix4'];
} else {
    $self->{LIBS} = ['-lrt'];
}


