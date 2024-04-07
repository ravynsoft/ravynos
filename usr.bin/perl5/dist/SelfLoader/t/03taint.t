#!perl -T

# This runs 01SelfLoader.t under taint.

(my $file = __FILE__) =~ s/[\w.]+\z/01SelfLoader.t/;
unshift @INC, ".";
do $file or die "Cannot run $file: ", $@||$!;
