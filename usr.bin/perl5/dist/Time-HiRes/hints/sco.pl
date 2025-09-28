# osr5 needs to explicitly link against libc to pull in usleep
# what's the reason for -lm?
$self->{LIBS} = ['-lm', '-lc'];

