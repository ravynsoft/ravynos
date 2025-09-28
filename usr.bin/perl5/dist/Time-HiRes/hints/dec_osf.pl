# needs to explicitly link against librt to pull in nanosleep
$self->{LIBS} = ['-lrt'];

