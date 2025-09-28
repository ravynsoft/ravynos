#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}

# Test that environment options are propagated to tainted tests

use strict;
use warnings;
use Test::More ( $^O eq 'VMS' ? ( skip_all => 'VMS' ) : ( tests => 2 ) );

use Config;
use TAP::Parser;

my $lib_path = join( ', ', map "'$_'", grep !ref, grep defined, @INC );

sub run_test_file {
    my ( $test_template, @args ) = @_;

    my $test_file = 'temp_test.tmp';

    open TEST, ">$test_file" or die $!;
    printf TEST $test_template, @args;
    close TEST;

    my $p = TAP::Parser->new(
        {   source => $test_file,

            # Test taint when there's spaces in a -I path
            switches => [q["-Ifoo bar"]],
        }
    );
    1 while $p->next;
    ok !$p->has_problems;

    unlink $test_file;
}

{
    local $ENV{PERL5OPT}
      = $ENV{PERL_CORE} ? '-I../../lib -Mstrict' : '-Mstrict';
    run_test_file(<<'END');
#!/usr/bin/perl -T

print "1..1\n";
print $INC{'strict.pm'} ? "ok 1\n" : "not ok 1\n";
END
}


# Check that PERL5LIB is propagated to -T.
{
    my $sentinel_dir = 'i/do/not/exist';
    local $ENV{PERL5LIB} = join $Config{path_sep}, $ENV{PERL5LIB} || '', $sentinel_dir;
    run_test_file(sprintf <<'END', $sentinel_dir);
#!/usr/bin/perl -T

print "1..1\n";
my $ok = grep { $_ eq '%s' } @INC;
print $ok ? "ok 1\n" : "not ok 1\n";
END
}

1;
