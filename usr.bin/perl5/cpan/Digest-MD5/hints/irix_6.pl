# The Mongoose v7.1 compiler freezes up somewhere in the optimization of
# MD5Transform() in MD5.c with optimization -O3.  This is a workaround:

use strict;
no strict 'vars';
if ( $Config{cc} =~ /64|n32/ && `$Config{cc} -version 2>&1` =~ /\s7\.1/ ) {
    my $optimize = $Config{optimize};
    $optimize =~ s/(^| )-O[2-9]\b/$1-O1/g
      and $self->{OPTIMIZE} = $optimize;
}
