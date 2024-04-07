# HP-UX 10.20 has different form for pthread_attr_getstacksize
my $ver = `uname -r`;
$ver =~ s/^\D*//;
if ($ver =~ /^10.20/) {
    if (exists($self->{'DEFINE'})) {
        $self->{'DEFINE'} .= " -DHPUX1020";
    } else {
        $self->{'DEFINE'} = "-DHPUX1020";
    }
}
