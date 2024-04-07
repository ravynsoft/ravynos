# uname -v
# V4.5.2
# needs to explicitly link against libc to pull in usleep
$self->{LIBS} = ['-lc'];

