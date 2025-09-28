# needs to explicitly link against librt to pull in clock_nanosleep
$self->{LIBS} = ['-lrt'];
