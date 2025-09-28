#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

use Test::More;

BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bSocket\b/ && 
        !(($^O eq 'VMS') && $Config{d_socket})) 
    {
	plan skip_all => "Test uses Socket, Socket not built";
    }
    if ($^O eq 'irix' && $Config{osvers} == 5) {
	plan skip_all => "Test relies on resolution of localhost, fails on $^O ($Config{osvers})";
    }
}

use Test::More;

BEGIN { use_ok 'Net::hostent' }

# Remind me to add this to Test::More.
sub DIE {
    print "# @_\n";
    exit 1;
}

# test basic resolution of localhost <-> 127.0.0.1
use Socket;

my $h = gethost('localhost');
SKIP: {
skip "Can't resolve localhost and you don't have /etc/hosts", 6
    if (!defined($h) && !-e '/etc/hosts');

ok(defined $h,  "gethost('localhost')") ||
  DIE("Can't continue without working gethost: $!");

is( inet_ntoa($h->addr), "127.0.0.1",   'addr from gethost' );

my $i = gethostbyaddr(inet_aton("127.0.0.1"));
ok(defined $i,  "gethostbyaddr('127.0.0.1')") || 
  DIE("Can't continue without working gethostbyaddr: $!");

is( inet_ntoa($i->addr), "127.0.0.1",   'addr from gethostbyaddr' );

$i = gethost("127.0.0.1");
ok(defined $i,  "gethost('127.0.0.1')");
is( inet_ntoa($i->addr), "127.0.0.1",   'addr from gethost' );

"127.0.0.1" =~ /(.*)/;
$i = gethost($1);
ok(defined $i, 'gethost on capture variable');

# need to skip the name comparisons on Win32 because windows will
# return the name of the machine instead of "localhost" when resolving
# 127.0.0.1 or even "localhost"

# - VMS returns "LOCALHOST" under tcp/ip services V4.1 ECO 2, possibly others
# - OS/390 returns localhost.YADDA.YADDA

SKIP: {
    skip "Windows will return the machine name instead of 'localhost'", 2
      if $^O eq 'MSWin32' or $^O eq 'cygwin';

    print "# name = " . $h->name . ", aliases = " . join (",", @{$h->aliases}) . "\n";

    my $in_alias;
    unless ($h->name =~ /^localhost(?:\..+)?$/i) {
        foreach (@{$h->aliases}) {
            if (/^localhost(?:\..+)?$/i) {
                $in_alias = 1;
                last;
            }
        }
	ok( $in_alias );
    } else {
	ok( 1 );
    }
    
    if ($in_alias) {
        # If we found it in the aliases before, expect to find it there again.
        foreach (@{$h->aliases}) {
            if (/^localhost(?:\..+)?$/i) {
                # This time, clear the flag if we see "localhost"
                undef $in_alias;
                last;
            }
        }
    } 

    if( $in_alias ) {
        like( $i->name, qr/^localhost(?:\..+)?$/i );
    }
    else {
        ok( !$in_alias );
        print "# " . $h->name . " " . join (",", @{$h->aliases}) . "\n";
    }
}
}

done_testing();
